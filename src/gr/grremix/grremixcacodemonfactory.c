/**
 * grremixcacodemonfactory.c - C port of the Cacodemon spawner of Spawned Fear
 * (.toolchain/pkgs/smashremix-src/src/Hazards.asm cacodemon_factory_).
 *
 * After the first 1500 frames of a match, each frame rolls a 1-in-1500 chance to spawn a
 * Cacodemon. At frame 3000 a Cacodemon always spawns. Only one Cacodemon is alive at a time.
 */

#ifdef PORT

#include <ssb_types.h>
#include <it/itmanager.h>
#include <sc/scene.h>
#include <sys/obj.h>
#include <sys/utils.h>
#include "gr/grremix/grremixcacodemonfactory.h"
#include "it/itremix/itremixcacodemon.h"

#define GRREMIX_CACODEMON_START_FRAME 1500 /* asm 0x05DC: no spawn roll before this match frame */
#define GRREMIX_CACODEMON_ROLL_RANGE 1500 /* asm: random integer in [0, 1500) per frame */
#define GRREMIX_CACODEMON_ROLL_HIT 80 /* asm 0x50: the roll value that spawns a Cacodemon */
#define GRREMIX_CACODEMON_FORCE_FRAME 3000 /* asm 0x0BB8: the Cacodemon always spawns on this frame */

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

/** Roll for one frame: a hit at the roll value, or the forced frame. */
static sb32 grRemixCacodemonFactoryShouldSpawn(u32 frame)
{
    s32 roll = syUtilsRandIntRange(GRREMIX_CACODEMON_ROLL_RANGE);

    if (roll == GRREMIX_CACODEMON_ROLL_HIT)
    {
        return TRUE;
    }
    return frame == GRREMIX_CACODEMON_FORCE_FRAME;
}

/** Spawns the item at the origin (asm stage item spawn at cacodemon_coordinates) and resets the timer to a random direction. */
static void grRemixCacodemonFactorySpawn(void)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };

    if (sGRRemixCacodemonFactory.is_spawn_prevented)
    {
        return;
    }
    sGRRemixCacodemonFactory.timer = 0;
    sGRRemixCacodemonFactory.is_spawn_prevented = TRUE;
    itManagerMakeItemSetupCommon(NULL, ITREMIX_CACODEMON_KIND, &pos, &vel, ITEM_FLAG_PARENT_GROUND);
    /* REMIX-TODO(anim): asm sets up the two Cacodemon animation tracks for the direction (0x8000BD1C, 0x8000BD54). */
    sGRRemixCacodemonFactory.timer = syUtilsRandIntRange(2);
}

/** cacodemon_factory_ (process): counts the match frames and spawns a Cacodemon on a roll or the forced frame. */
static void grRemixCacodemonFactoryProcUpdate(GObj *factory_gobj)
{
    u32 frame;

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
    if (grRemixCacodemonFactoryShouldSpawn(frame))
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
