/**
 * grremixrainbow.c - Rainbow Road hazards: Chain Chomp schedule and stage sounds (Hazards.asm rainbow_road_setup,
 * chain_chomp_main_, sfx_main_). See grremixrainbow.h.
 */

#ifdef PORT

#include "grremixrainbow.h"
#include "grremixrainbowchomp.h"
#include "grremixhazards.h"
#include <gr/ground.h>
#include <mp/map.h>
#include <sys/utils.h>

extern void *func_800269C0_275C0(u16 id);

#define GRREMIX_RAINBOW_STAGE_OFFSET_CHOMP_FILE 0x0080
#define GRREMIX_RAINBOW_STAGE_OFFSET_HITBOX_FILE 0x00AC
#define GRREMIX_RAINBOW_CHOMP_OFFSET_DOBJDESC 0x17F0
#define GRREMIX_RAINBOW_SCHEDULE_GATE 0x05DC
#define GRREMIX_RAINBOW_TIMER_RESET 0x35E8
#define GRREMIX_RAINBOW_SPAWN_CHANCE 810
#define GRREMIX_RAINBOW_SPAWN_ROLL 0x100
#define GRREMIX_RAINBOW_FGM_CHOMP_SPAWN 0x0401
#define GRREMIX_RAINBOW_SFX_FIRST_TIMER 0x0960
#define GRREMIX_RAINBOW_SFX_INTERVAL 0x003C
#define GRREMIX_RAINBOW_SFX_LOOP_COUNT 4
#define GRREMIX_RAINBOW_SFX_LAST_TIMER (GRREMIX_RAINBOW_SFX_FIRST_TIMER + GRREMIX_RAINBOW_SFX_INTERVAL * (GRREMIX_RAINBOW_SFX_LOOP_COUNT + 1))
#define GRREMIX_RAINBOW_FGM_SFX_FIRST 0x0402
#define GRREMIX_RAINBOW_FGM_SFX_LOOP 0x0403
#define GRREMIX_RAINBOW_FGM_SFX_LAST 0x0404

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
    sb32 chomp_visible;
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

/** Creates the Chain Chomp ground GObj with its model; it stays hidden until its first spawn. */
static void grRemixRainbowChompCreate(void)
{
    GObj *chomp_gobj;

    chomp_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    sRainbow.chomp_gobj = chomp_gobj;

    /* REMIX-TODO(stage object init): the model/texture setup at chomp file + 0x19E8 (vanilla 0x8000F8F4) is not applied yet. */
    gcSetupCustomDObjs(chomp_gobj, (DObjDesc *) ((uintptr_t) sRainbow.chomp_file + GRREMIX_RAINBOW_CHOMP_OFFSET_DOBJDESC), NULL, nGCMatrixKindTraRotRpyRSca, nGCMatrixKindNull, nGCMatrixKindNull);
    gcAddGObjProcess(chomp_gobj, grRemixRainbowChompProcUpdate, nGCProcessKindFunc, 5);
}

/** Creates the Rainbow Road sound GObj (Hazards.asm sfx_main_). */
static void grRemixRainbowSfxCreate(void)
{
    GObj *sfx_gobj;

    sfx_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(sfx_gobj, grRemixRainbowSfxProcUpdate, nGCProcessKindFunc, 5);
}

/** Plays the stage sound of one timer value: a first cue, four loop cues, then a last cue that restarts the sound timer. */
static void grRemixRainbowSfxPlayAt(s32 timer)
{
    s32 i;

    if (timer == GRREMIX_RAINBOW_SFX_FIRST_TIMER)
    {
        func_800269C0_275C0(GRREMIX_RAINBOW_FGM_SFX_FIRST);
        return;
    }
    for (i = 1; i <= GRREMIX_RAINBOW_SFX_LOOP_COUNT; i++)
    {
        if (timer == GRREMIX_RAINBOW_SFX_FIRST_TIMER + GRREMIX_RAINBOW_SFX_INTERVAL * i)
        {
            func_800269C0_275C0(GRREMIX_RAINBOW_FGM_SFX_LOOP);
            return;
        }
    }
    if (timer == GRREMIX_RAINBOW_SFX_LAST_TIMER)
    {
        sRainbow.sfx_timer = 0;
        func_800269C0_275C0(GRREMIX_RAINBOW_FGM_SFX_LAST);
    }
}

/** Per-frame sound routine of the stage. */
static void grRemixRainbowSfxProcUpdate(GObj *sfx_gobj)
{
    s32 timer = sRainbow.sfx_timer;

    sRainbow.sfx_timer = timer + 1;
    grRemixRainbowSfxPlayAt(timer);
}

/** Plays the chomp's animation from the stage file; only the first animation struct is applied (see the REMIX-TODO). */
static void grRemixRainbowChompAnimate(GObj *chomp_gobj, const GRRemixRainbowScene *scene)
{
    /* REMIX-TODO(vanilla 0x8000BED8, 0x8000DF34): the second animation struct (anim_offset_end) and the part-2 call have no decomp entry; the first struct plays alone. */
    gcAddAnimJointAll(chomp_gobj, (AObjEvent32 **) ((uintptr_t) sRainbow.chomp_file + scene->anim_offset_start), 0.0F);
    gcPlayAnimAll(chomp_gobj);
}

/** Shows the chomp at its first spawn; Hazards.asm creates it invisible (0x7C = 1) and clears that flag here. */
static void grRemixRainbowChompMakeVisible(GObj *chomp_gobj)
{
    if (sRainbow.chomp_visible)
    {
        return;
    }
    sRainbow.chomp_visible = TRUE;
    gcAddGObjDisplay(chomp_gobj, gcDrawDObjTreeForGObj, 6, GOBJ_PRIORITY_DEFAULT, ~0);
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
 * Entry point of the Rainbow Road setup: stage file pointers, the Chain Chomp (when hazards are on) and the stage sound
 * (when movement is on). State is cleared on every setup.
 */
void grRemixRainbowSetup(void)
{
    sRainbow = (GRRemixRainbowState) { 0 };
    sRainbow.chomp_file = grRemixRainbowReadStageFile(GRREMIX_RAINBOW_STAGE_OFFSET_CHOMP_FILE);
    sRainbow.hitbox_file = grRemixRainbowReadStageFile(GRREMIX_RAINBOW_STAGE_OFFSET_HITBOX_FILE);
    if (grRemixHazardsAreOn())
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
