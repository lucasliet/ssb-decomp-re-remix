/**
 * grremixrainbow.c - Rainbow Road hazards: Chain Chomp schedule and stage sounds (Hazards.asm rainbow_road_setup,
 * chain_chomp_main_, sfx_main_). See grremixrainbow.h.
 */

#ifdef PORT

#include "grremixrainbow.h"
#include "grremixrainbowchomp.h"
#include "grremixhazards.h"
#include "grremixhazardtimerrules.h"
#include <gr/ground.h>
#include <gr/grdisplay.h>
#include <mp/map.h>
#include <sys/utils.h>

extern void *func_800269C0_275C0(u16 id);

#define GRREMIX_RAINBOW_STAGE_OFFSET_CHOMP_FILE 0x0080
#define GRREMIX_RAINBOW_STAGE_OFFSET_HITBOX_FILE 0x00AC
#define GRREMIX_RAINBOW_CHOMP_OFFSET_DOBJDESC 0x17F0
#define GRREMIX_RAINBOW_CHOMP_OFFSET_MODEL 0x19E8 /* asm: model and texture setup passed to 0x8000F8F4 (gcAddMObjAll) */
#define GRREMIX_RAINBOW_SCHEDULE_GATE 0x05DC
#define GRREMIX_RAINBOW_TIMER_RESET 0x35E8
#define GRREMIX_RAINBOW_SPAWN_CHANCE 810
#define GRREMIX_RAINBOW_SPAWN_ROLL 0x100
#define GRREMIX_RAINBOW_FGM_CHOMP_SPAWN 0x0401
#define GRREMIX_RAINBOW_CHOMP_DISPLAY_LINK 0x11 /* Hazards.asm 5707-5715: DISPLAY_INIT_ link 0x11 */

typedef struct GRRemixRainbowScene
{
    s32 first_timer;
    s32 last_timer;
    s32 clear_timer;
    intptr_t anim_offset_start;
    intptr_t anim_offset_end;
} GRRemixRainbowScene;

typedef struct GRRemixRainbowState
{
    GObj *chomp_gobj;
    void *chomp_file;
    void *hitbox_file;
    s32 timer;
    s32 chomp_flag;
    s32 sfx_timer;
} GRRemixRainbowState;

/* Windows of the chomp scenes; the scene index + 1 is the chomp_flag that blocks a second spawn in the same window. */
static const GRRemixRainbowScene sRainbowScenes[] =
{
    { 0x05DB, 0x07D0, 0x05DC, 0x9518, 0x9558 },
    { 0x10A4, 0x1298, 0x10A4, 0xE658, 0xE6A4 },
    { 0x1B6C, 0x1D60, 0x1B6C, 0x15AB8, 0x15B04 },
    { 0x30FC, 0x321C, 0x30FC, 0x1EDA4, 0x1EDF0 },
};

static GRRemixRainbowState sRainbow;

static void grRemixRainbowChompProcUpdate(GObj *chomp_gobj);
static void grRemixRainbowSfxProcUpdate(GObj *sfx_gobj);

/** Reads a stage file pointer from the stage data at byte offset; the pointer is a relocation token in the port. */
static void *grRemixRainbowReadStageFile(s32 offset)
{
    return (void *) PORT_RESOLVE(*(u32 *) ((uintptr_t) gMPCollisionGroundData + offset));
}

/**
 * Creates the Chain Chomp ground GObj with its model, drawn without the z-buffer on display link 0x11 and hidden until
 * its first spawn (Hazards.asm rainbow_road_setup sets GObj 0x7C to 1). gcPlayAnimAll runs as a per-frame process
 * (Hazards.asm 5737-5743), so the spawn animation moves the chomp along the track.
 */
static void grRemixRainbowChompCreate(void)
{
    GObj *chomp_gobj;

    chomp_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    sRainbow.chomp_gobj = chomp_gobj;
    chomp_gobj->flags = GOBJ_FLAG_HIDDEN;
    gcAddGObjDisplay(chomp_gobj, grDisplayLayer0PriProcDisplay, GRREMIX_RAINBOW_CHOMP_DISPLAY_LINK, GOBJ_PRIORITY_DEFAULT, ~0);

    gcSetupCustomDObjs(chomp_gobj, (DObjDesc *) ((uintptr_t) sRainbow.chomp_file + GRREMIX_RAINBOW_CHOMP_OFFSET_DOBJDESC), NULL, nGCMatrixKindTraRotRpyRSca, nGCMatrixKindNull, nGCMatrixKindNull);
    gcAddMObjAll(chomp_gobj, (MObjSub ***) ((uintptr_t) sRainbow.chomp_file + GRREMIX_RAINBOW_CHOMP_OFFSET_MODEL));
    gcAddGObjProcess(chomp_gobj, grRemixRainbowChompProcUpdate, nGCProcessKindFunc, 5);
    gcAddGObjProcess(chomp_gobj, gcPlayAnimAll, nGCProcessKindFunc, 5);
}

/** Creates the Rainbow Road sound GObj (Hazards.asm sfx_main_). */
static void grRemixRainbowSfxCreate(void)
{
    GObj *sfx_gobj;

    sfx_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(sfx_gobj, grRemixRainbowSfxProcUpdate, nGCProcessKindFunc, 5);
}

/** Plays the stage sound of one timer value: a first cue, five loop cues, then a last cue that restarts the sound timer. */
static void grRemixRainbowSfxPlayAt(s32 timer)
{
    s32 fgm_id = grRemixRainbowRulesSfxCue(timer);

    if (fgm_id == GRREMIX_RAINBOW_FGM_SFX_NONE)
    {
        return;
    }
    if (fgm_id == GRREMIX_RAINBOW_FGM_SFX_LAST)
    {
        sRainbow.sfx_timer = 0;
    }
    func_800269C0_275C0(fgm_id);
}

/** Per-frame sound routine of the stage. */
static void grRemixRainbowSfxProcUpdate(GObj *sfx_gobj)
{
    s32 timer = sRainbow.sfx_timer;

    sRainbow.sfx_timer = timer + 1;
    grRemixRainbowSfxPlayAt(timer);
}

/** Plays the chomp's joint and material animation from the stage file (asm 0x8000BED8 then 0x8000DF34). */
static void grRemixRainbowChompAnimate(GObj *chomp_gobj, const GRRemixRainbowScene *scene)
{
    gcAddAnimAll(chomp_gobj,
                 (AObjEvent32 **) ((uintptr_t) sRainbow.chomp_file + scene->anim_offset_start),
                 (AObjEvent32 ***) ((uintptr_t) sRainbow.chomp_file + scene->anim_offset_end),
                 0.0F);
    gcPlayAnimAll(chomp_gobj);
}

/** Shows the chomp on every spawn; Hazards.asm _animation clears GObj 0x7C (the hidden flag). */
static void grRemixRainbowChompMakeVisible(GObj *chomp_gobj)
{
    chomp_gobj->flags = 0;
}

/** Runs one chomp spawn: animation, visibility, the hitbox weapon and its sound. */
static void grRemixRainbowChompPerform(const GRRemixRainbowScene *scene)
{
    GObj *chomp_gobj = sRainbow.chomp_gobj;

    grRemixRainbowChompAnimate(chomp_gobj, scene);
    grRemixRainbowChompMakeVisible(chomp_gobj);
    grRemixRainbowChompSpawn(chomp_gobj, &sRainbow.hitbox_file);
    func_800269C0_275C0(GRREMIX_RAINBOW_FGM_CHOMP_SPAWN);
}

/** Rolls the spawn of one window: at most one spawn per window, with a 1 in 810 roll on each frame until it happens. */
static void grRemixRainbowChompTrySpawn(const GRRemixRainbowScene *scene, s32 flag)
{
    if (sRainbow.chomp_flag != 0)
    {
        return;
    }
    if (syUtilsRandIntRange(GRREMIX_RAINBOW_SPAWN_CHANCE) != GRREMIX_RAINBOW_SPAWN_ROLL)
    {
        return;
    }
    sRainbow.chomp_flag = flag;
    grRemixRainbowChompPerform(scene);
}

/** TRUE when the timer lies inside the scene window, both ends included. */
static sb32 grRemixRainbowSceneContains(const GRRemixRainbowScene *scene, s32 timer)
{
    return (timer >= scene->first_timer) && (timer <= scene->last_timer);
}

/** Walks the scene windows for one timer value (the timer before this frame's increment, as in the asm). */
static void grRemixRainbowChompUpdateScenes(s32 timer)
{
    s32 i;
    const GRRemixRainbowScene *scene;

    for (i = 0; i < ARRAY_COUNT(sRainbowScenes); i++)
    {
        scene = &sRainbowScenes[i];
        if (timer == scene->clear_timer)
        {
            sRainbow.chomp_flag = 0;
        }
        if (grRemixRainbowSceneContains(scene, timer))
        {
            grRemixRainbowChompTrySpawn(scene, i + 1);
            return;
        }
    }
}

/**
 * Per-frame Chain Chomp routine. The scene windows are checked against the timer before this frame's increment, and the
 * timer restarts after GRREMIX_RAINBOW_TIMER_RESET frames.
 */
static void grRemixRainbowChompProcUpdate(GObj *chomp_gobj)
{
    s32 timer = sRainbow.timer;

    sRainbow.timer = timer + 1;
    if (timer + 1 >= GRREMIX_RAINBOW_SCHEDULE_GATE)
    {
        grRemixRainbowChompUpdateScenes(timer);
    }
    if (sRainbow.timer == GRREMIX_RAINBOW_TIMER_RESET)
    {
        sRainbow.timer = 0;
    }
}

/**
 * Entry point of the Rainbow Road setup: stage file pointers, the Chain Chomp (when both hazards and movement are on) and
 * the stage sound (when movement is on). State is cleared on every setup.
 */
void grRemixRainbowSetup(void)
{
    sRainbow = (GRRemixRainbowState) { 0 };
    sRainbow.chomp_file = grRemixRainbowReadStageFile(GRREMIX_RAINBOW_STAGE_OFFSET_CHOMP_FILE);
    sRainbow.hitbox_file = grRemixRainbowReadStageFile(GRREMIX_RAINBOW_STAGE_OFFSET_HITBOX_FILE);
    /* Hazards.asm:5661 rainbow_road_setup tests the whole hazard_mode with bnez, so MOVEMENT OFF also drops the chomp */
    if ((grRemixHazardsAreOn() != FALSE) && (grRemixMovementIsOn() != FALSE))
    {
        grRemixRainbowChompCreate();
    }
    if (grRemixMovementIsOn())
    {
        grRemixRainbowSfxCreate();
    }
}

GObj *grRemixRainbowGetChompGObj(void)
{
    return sRainbow.chomp_gobj;
}

#endif
