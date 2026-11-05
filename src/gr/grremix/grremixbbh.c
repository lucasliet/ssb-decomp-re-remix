/**
 * grremixbbh.c - Big Boo's Haunt Remix hazards (Hazards.asm bbh_setup_, boo_sound_). See grremixbbh.h.
 */

#ifdef PORT

#include "grremixbbh.h"
#include <sys/objdef.h>
#include <sys/objman.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/utils.h>
#include <port_log.h>

/* Hazards.asm bbh_setup_ creates object 0x03F2, which is nGCCommonKindGround (1010). */
#define GRREMIX_BBH_OBJECT_KIND nGCCommonKindGround
/* Hazards.asm bbh_setup_ uses group 1, which is nGCCommonLinkIDGround. */
#define GRREMIX_BBH_LINK_ID nGCCommonLinkIDGround
/* Hazards.asm bbh_setup_ registers boo_sound_ with priority 4. */
#define GRREMIX_BBH_PROC_PRIORITY 4
/* Hazards.asm boo_sound_: FGM 0x0527 is the Boo laugh. */
#define GRREMIX_BBH_FGM_BOO_LAUGH 0x0527
/* efGroundMakeAppearActor / efGroundMakeEffectID: countdown (frames) is rand(10000) + 6000 between Boo flights. */
#define GRREMIX_BBH_COUNTDOWN_RANGE 10000
#define GRREMIX_BBH_COUNTDOWN_BASE 6000
/* efGroundMakeEffectID: frames between the Boos of one flight (make_queue != 0). */
#define GRREMIX_BBH_FLIGHT_GAP 30
/* stages/bbh.asm add_sprite_info_array(0x0000, 0x0001, 0x0, 0x01): spawn_multiple is the EFGroundParam make_queue. */
#define GRREMIX_BBH_PARAM_MAKE_QUEUE 1
/* EFGroundActorProcUpdate: make_queue += syUtilsRandIntRange(2) when the param queue is not zero. */
#define GRREMIX_BBH_QUEUE_RANDOM_RANGE 2

/*
 * State of the vanilla ground actor (sEFGroundActor, RAM 0x80131AD8) that Hazards.asm boo_sound_ reads. Remix runs the
 * vanilla actor on Big Boo's Haunt with the bg_info of stages/bbh.asm (Stages.asm bg_animation_); the port does not run
 * the actor for Remix stages (efGroundMakeAppearActor only accepts vanilla stages), so only its countdown is kept here.
 */
typedef struct GRRemixBbhBooActor
{
    s32 make_wait;
    s32 make_queue;

} GRRemixBbhBooActor;

static GRRemixBbhBooActor sGRRemixBbhBooActor;

/* Same vanilla FGM player declaration as grpupupu.c and gryamabuki.c. */
extern void *func_800269C0_275C0(u16 id);

/**
 * Returns TRUE when the current scene is the training mode, where Hazards.asm bbh_setup_ creates nothing.
 *
 * @return TRUE in training mode.
 */
static sb32 grRemixBbhIsTrainingScene(void)
{
    return gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode;
}

/**
 * Draws the countdown that follows a Boo flight (efGroundMakeEffectID end): a long wait after the last Boo of the
 * flight, the flight gap while more Boos are queued.
 */
static void grRemixBbhBooActorSetNextWait(void)
{
    if (sGRRemixBbhBooActor.make_queue == 0)
    {
        sGRRemixBbhBooActor.make_wait = syUtilsRandIntRange(GRREMIX_BBH_COUNTDOWN_RANGE) + GRREMIX_BBH_COUNTDOWN_BASE;
        return;
    }
    sGRRemixBbhBooActor.make_wait = GRREMIX_BBH_FLIGHT_GAP;
}

/**
 * Queue step of EFGroundActorProcUpdate when the countdown reached zero: a new flight takes the queue of the only
 * bg_info param plus a random extra Boo, a running flight takes one Boo off the queue.
 */
static void grRemixBbhBooActorStepQueue(void)
{
    if (sGRRemixBbhBooActor.make_queue != 0)
    {
        sGRRemixBbhBooActor.make_queue--;
        return;
    }
    sGRRemixBbhBooActor.make_queue = GRREMIX_BBH_PARAM_MAKE_QUEUE + syUtilsRandIntRange(GRREMIX_BBH_QUEUE_RANDOM_RANGE);
}

/**
 * Runs the ground actor countdown for one frame (EFGroundActorProcUpdate).
 *
 * @return TRUE on the frame the countdown is zero, the frame the actor spawns a Boo and boo_sound_ laughs.
 */
static sb32 grRemixBbhBooActorUpdate(void)
{
    sb32 is_countdown_zero = (sGRRemixBbhBooActor.make_wait == 0);

    if (is_countdown_zero)
    {
        grRemixBbhBooActorStepQueue();
        grRemixBbhBooActorSetNextWait();
    }
    sGRRemixBbhBooActor.make_wait--;

    return is_countdown_zero;
}

/**
 * Per-frame routine of the Boo laugh object (Hazards.asm boo_sound_).
 *
 * @param gobj Boo laugh object created by grRemixBbhSetup.
 */
static void grRemixBbhBooSoundProcUpdate(GObj *gobj)
{
    if (grRemixBbhBooActorUpdate())
    {
        func_800269C0_275C0(GRREMIX_BBH_FGM_BOO_LAUGH);
    }
}

/**
 * Creates the Boo laugh object of Big Boo's Haunt (Hazards.asm bbh_setup_), skipped in training mode.
 */
void grRemixBbhSetup(void)
{
    GObj *gobj;

    if (grRemixBbhIsTrainingScene())
    {
        return;
    }
    gobj = gcMakeGObjSPAfter(GRREMIX_BBH_OBJECT_KIND, NULL, GRREMIX_BBH_LINK_ID, GOBJ_PRIORITY_DEFAULT);
    if (gobj == NULL)
    {
        port_log("[remix-hazard] bbh: Boo laugh object not created, the GObj pool is full");
        return;
    }
    sGRRemixBbhBooActor.make_queue = 0;
    sGRRemixBbhBooActor.make_wait = syUtilsRandIntRange(GRREMIX_BBH_COUNTDOWN_RANGE) + GRREMIX_BBH_COUNTDOWN_BASE;
    gcAddGObjProcess(gobj, grRemixBbhBooSoundProcUpdate, nGCProcessKindFunc, GRREMIX_BBH_PROC_PRIORITY);
}

#endif
