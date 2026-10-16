/**
 * grremixbanzai.c - Banzai Bill factory of Peach's Castle II. Ported from Hazards.asm banzai_bill_factory_
 * and the object creation of peach_castle_II_setup_.
 */

#ifdef PORT

#include "grremixbanzai.h"
#include "grremixhazards.h"
#include "it/itremix/itremix.h"
#include "it/itremix/itremixbulletbill.h"
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
 * Spawns Banzai Bill at the origin, the coordinates of the asm banzai_bill_coordinates table, and starts its
 * animation tracks. The asm spawns through itManagerMakeItemSetupCommon as well.
 * REMIX-TODO(item id): the asm spawns Item.RobotBee.id (0x3B) with the Robot Bee item info; this port spawns
 * Bullet Bill (0x3D) as the task asked. Confirm the spawned kind in game, see docs/remix/notes/f7_hazard_banzai.md.
 */
static void grRemixBanzaiSpawn(void)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *bullet_gobj;

    bullet_gobj = itManagerMakeItemSetupCommon(NULL, ITREMIX_BULLETBILL_KIND, &pos, &vel, 0);
    if (bullet_gobj != NULL)
    {
        itRemixLogEvent("bulletbill-spawn", ITREMIX_BULLETBILL_KIND);
        itRemixBulletBillStartAnimation(bullet_gobj);
    }
}

#endif
