/**
 * grremixcacodemonfactory.c - C port of the Cacodemon spawner of Spawned Fear
 * (.toolchain/pkgs/smashremix-src/src/Hazards.asm cacodemon_factory_).
 *
 * After the first 1500 frames of a match, each frame rolls a 1-in-1500 chance to spawn a
 * Cacodemon; at frame 3000 the spawn always triggers. Only one Cacodemon is alive at a time: a trigger while one is
 * alive only restarts the timer.
 */

#ifdef PORT

#include <ssb_types.h>
#include <it/itmanager.h>
#include <sc/scene.h>
#include <sys/obj.h>
#include <sys/utils.h>
#include "gr/grremix/grremixcacodemonfactory.h"
#include "gr/grremix/grremixhazardtimerrules.h"
#include "it/itremix/itremix.h"
#include "it/itremix/itremixcacodemon.h"

#define GRREMIX_CACODEMON_DIRECTION_RANGE 2 /* asm draws the direction twice and keeps the second draw */

typedef struct GRRemixCacodemonFactoryVars
{
    u32 timer;                      /* asm 0x801313F0 + 0x20; after a spawn it holds the random direction */
    sb32 is_spawn_prevented;        /* asm 0x801313F0 + 0x1C: a Cacodemon is alive */
} GRRemixCacodemonFactoryVars;

static GRRemixCacodemonFactoryVars sGRRemixCacodemonFactory;

/** TRUE on the Training Mode screen or once the match clock has started (asm Global.current_screen / match_info). */
static sb32 grRemixCacodemonFactoryIsActive(void)
{
    if (gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode)
    {
        return TRUE;
    }
    return gSCManagerBattleState->time_passed != 0;
}

/** Spawns the item at the origin (asm stage item spawn at cacodemon_coordinates) and sets the timer to a random direction. */
static void grRemixCacodemonFactorySpawn(void)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *cacodemon_gobj;

    sGRRemixCacodemonFactory.is_spawn_prevented = TRUE;
    cacodemon_gobj = itManagerMakeItemSetupCommon(NULL, ITREMIX_CACODEMON_KIND, &pos, &vel, ITEM_FLAG_PARENT_GROUND);
    syUtilsRandIntRange(GRREMIX_CACODEMON_DIRECTION_RANGE);
    sGRRemixCacodemonFactory.timer = syUtilsRandIntRange(GRREMIX_CACODEMON_DIRECTION_RANGE);
    if (cacodemon_gobj != NULL)
    {
        itRemixLogEvent("cacodemon-spawn", ITREMIX_CACODEMON_KIND);
        itRemixCacodemonStartAnimation(cacodemon_gobj);
    }
}

/**
 * cacodemon_factory_ (process): counts the match frames; a roll hit or the forced frame restarts the timer and spawns a
 * Cacodemon when none is alive.
 */
static void grRemixCacodemonFactoryProcUpdate(GObj *factory_gobj)
{
    u32 frame;
    GRRemixCacodemonFactoryAction action;

    if (!grRemixCacodemonFactoryIsActive())
    {
        return;
    }
    frame = sGRRemixCacodemonFactory.timer;
    sGRRemixCacodemonFactory.timer = frame + 1;
    if (frame < GRREMIX_CACODEMON_START_FRAME)
    {
        return;
    }
    action = grRemixCacodemonRulesFactoryAction(frame, syUtilsRandIntRange(GRREMIX_CACODEMON_ROLL_RANGE),
                                                sGRRemixCacodemonFactory.is_spawn_prevented);
    if (action == nGRRemixCacodemonFactoryActionNone)
    {
        return;
    }
    /* Hazards.asm 8641-8643: the restart sits in the delay slot of the prevent check, so it runs with one alive too */
    sGRRemixCacodemonFactory.timer = 0;
    if (action == nGRRemixCacodemonFactoryActionSpawn)
    {
        grRemixCacodemonFactorySpawn();
    }
}

void grRemixCacodemonFactoryMake(void)
{
    GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    sGRRemixCacodemonFactory.timer = 0;
    sGRRemixCacodemonFactory.is_spawn_prevented = FALSE;
    gcAddGObjProcess(gobj, grRemixCacodemonFactoryProcUpdate, nGCProcessKindFunc, 4);
}

void grRemixCacodemonFactoryAllowSpawn(void)
{
    sGRRemixCacodemonFactory.is_spawn_prevented = FALSE;
}

#endif
