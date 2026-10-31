/**
 * grremixbanzai.c - Banzai Bill factory of Peach's Castle II. Ported from Hazards.asm banzai_bill_factory_
 * and the object creation of peach_castle_II_setup_. Unreachable in Remix 2.0.1, as in the asm: Stages.asm 3256 gives
 * Peach's Castle II `function.CLONE`, so the stage runs the original Peach's Castle routine and nothing references
 * peach_castle_II_setup_. The port of the dead code stays faithful and never runs.
 */

#ifdef PORT

#include "grremixbanzai.h"
#include "grremixhazards.h"
#include "it/itremix/itremix.h"
#include "it/itremix/itremixbulletbill.h"
#include "it/itremix/itremixrobotbee.h"
#include "gr/grremix/grremixmetallic.h"
#include "it/itmanager.h"
#include "sc/sctypes.h"
#include "sc/scmanager.h"
#include "sys/objman.h"
#include "sys/objanim.h"
#include "sys/utils.h"

#define GRREMIX_BANZAI_SPAWN_WAIT_TIME 0x5DC    /* asm banzai_bill_factory_: wait 1500 tics before the first roll */
#define GRREMIX_BANZAI_SPAWN_ROLL_RANGE 0x5DC   /* asm: Global.get_random_int_(1500) */
#define GRREMIX_BANZAI_SPAWN_ROLL_HIT 0x50      /* asm: roll result that spawns Banzai Bill (80) */
#define GRREMIX_BANZAI_SPAWN_TIME_FORCED 0xBB8  /* asm: Banzai Bill spawns at timer 3000 when no roll hit */

static u32 sGRRemixBanzaiTimer;

static void grRemixBanzaiMakeProcessGObj(void (*proc)(GObj*));
static void grRemixBanzaiFactoryProcUpdate(GObj *factory_gobj);
static sb32 grRemixBanzaiFactoryIsActive(void);
static sb32 grRemixBanzaiRollSpawn(u32 timer);
static void grRemixBanzaiSpawn(void);

/**
 * Stores the stage header, clears the factory timer and creates the factory process. The asm also stores the
 * Banzai Bill file top at 0x801313F0 here, which itRemixBulletBillSetStageFile keeps as the stage header instead.
 */
void grRemixBanzaiFactoryMake(void *stage_header)
{
    sGRRemixBanzaiTimer = 0;
    itRemixBulletBillSetStageFile(stage_header);
    /* Hazards.asm stores the Robot Bee file in the hazard words (0x801313F0) shared with Metallic Madness: the Robot Bee item reads it. */
    *grRemixMetallicStageHeaderRef() = stage_header;
    *grRemixMetallicLaserFileRef() = NULL;
    grRemixBanzaiMakeProcessGObj(grRemixBanzaiFactoryProcUpdate);
}

/** Creates a ground GObj that runs `proc` every frame, the same way the Metallic factory does. */
static void grRemixBanzaiMakeProcessGObj(void (*proc)(GObj*))
{
    GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjProcess(gobj, proc, nGCProcessKindFunc, 4);
}

/**
 * banzai_bill_factory_ (asm). Counts tics once the factory is active, rolls for a spawn after 1500 tics and
 * spawns Banzai Bill when the roll or the forced timer says so.
 */
static void grRemixBanzaiFactoryProcUpdate(GObj *factory_gobj)
{
    u32 timer;

    if (!grRemixBanzaiFactoryIsActive())
    {
        return;
    }
    timer = sGRRemixBanzaiTimer;
    sGRRemixBanzaiTimer = timer + 1;
    if (timer < GRREMIX_BANZAI_SPAWN_WAIT_TIME)
    {
        return;
    }
    if (!grRemixBanzaiRollSpawn(timer))
    {
        return;
    }
    sGRRemixBanzaiTimer = 0;
    grRemixBanzaiSpawn();
}

/**
 * The asm returns for every screen but training mode after checking the match clock, and then returns
 * unconditionally ("this is added to prevent spawn crash until the item object works"). So the factory only
 * runs in training mode. The port keeps that gate.
 */
static sb32 grRemixBanzaiFactoryIsActive(void)
{
    return gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode;
}

/**
 * Banzai Bill spawns when the random roll hits 80, or when the timer reaches 3000 without a roll hit (asm
 * banzai_bill_factory_). Returns TRUE when it spawns.
 */
static sb32 grRemixBanzaiRollSpawn(u32 timer)
{
    s32 roll = syUtilsRandIntRange(GRREMIX_BANZAI_SPAWN_ROLL_RANGE);

    if (roll == GRREMIX_BANZAI_SPAWN_ROLL_HIT)
    {
        return TRUE;
    }
    return timer == GRREMIX_BANZAI_SPAWN_TIME_FORCED;
}

/**
 * Spawns Banzai Bill at the origin, the coordinates of the asm banzai_bill_coordinates table. As in the asm the item is a Robot Bee
 * (Item.RobotBee.id 0x3B) that flies on the Banzai Bill animation tracks of the stage file (offsets 0x908 and 0x9B4). The
 * banzai_bill_stage_setting_, main_ and destroy_ routines of Hazards.asm are not referenced anywhere in Remix, so the Robot Bee
 * routines are the ones that run. The bee fires no laser here: the asm stores an uninitialised register as the laser file.
 */
static void grRemixBanzaiSpawn(void)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *bee_gobj;

    bee_gobj = itManagerMakeItemSetupCommon(NULL, ITREMIX_ROBOTBEE_KIND, &pos, &vel, 0);
    if (bee_gobj != NULL)
    {
        itRemixLogEvent("banzai-robotbee-spawn", ITREMIX_ROBOTBEE_KIND);
        itRemixBulletBillStartAnimation(bee_gobj);
    }
}

#endif
