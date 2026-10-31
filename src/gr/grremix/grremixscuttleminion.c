/**
 * grremixscuttleminion.c - Scuttle Town Scuttle Minion factory.
 * Ported from Hazards.asm minion_factory_.
 */

#ifdef PORT

#include "grremixscuttleminion.h"
#include "grremixhazards.h"
#include "it/itremix/itremix.h"
#include "it/itremix/itremixscuttleminion.h"
#include "it/itmanager.h"
#include "sys/objman.h"
#include "sys/utils.h"

#define GRREMIX_SCUTTLEMINION_SPAWN_WAIT_TIME 0x5DC    /* asm minion_factory_: 1500 tics before the first roll */
#define GRREMIX_SCUTTLEMINION_SPAWN_ROLL_RANGE 0x5DC   /* asm minion_factory_: Global.get_random_int_(1500) */
#define GRREMIX_SCUTTLEMINION_SPAWN_ROLL_HIT 0x50      /* asm minion_factory_: roll result that spawns the minion (80) */
#define GRREMIX_SCUTTLEMINION_SPAWN_TIME_FORCED 0xBB8  /* asm minion_factory_: the minion spawns at timer 3000 when no roll hit */

static void *sGRRemixScuttleMinionStageHeader;
static u32 sGRRemixScuttleMinionTimer;

static void grRemixScuttleMinionClearState(void);
static void grRemixScuttleMinionMakeProcessGObj(void (*proc)(GObj*));
static void grRemixScuttleMinionFactoryProcUpdate(GObj *factory_gobj);
static sb32 grRemixScuttleMinionRollSpawn(u32 timer);
static void grRemixScuttleMinionSpawn(void);

/**
 * Creates the minion factory process and clears its state. The stage header is kept so the minion item can read its
 * file from it (asm minion_factory_ reads the item file from 0x801313F0).
 */
void grRemixScuttleMinionFactoryMake(void *stage_header)
{
    grRemixScuttleMinionClearState();
    sGRRemixScuttleMinionStageHeader = stage_header;
    grRemixScuttleMinionMakeProcessGObj(grRemixScuttleMinionFactoryProcUpdate);
}

/** Returns the address of the stage header pointer for item descriptors. */
void **grRemixScuttleMinionStageHeaderRef(void)
{
    return &sGRRemixScuttleMinionStageHeader;
}

/** Clears the stage header and the spawn timer. */
static void grRemixScuttleMinionClearState(void)
{
    sGRRemixScuttleMinionStageHeader = NULL;
    sGRRemixScuttleMinionTimer = 0;
}

/** Creates a ground GObj that runs `proc` every frame with the default process priority of the stage routines. */
static void grRemixScuttleMinionMakeProcessGObj(void (*proc)(GObj*))
{
    GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjProcess(gobj, proc, nGCProcessKindFunc, 4);
}

/**
 * minion_factory_ (asm). Counts the tics while the hazard is active and rolls for a minion once 1500 tics have passed.
 * The timer only counts while the match has started (training mode always counts), as the asm checks it first.
 */
static void grRemixScuttleMinionFactoryProcUpdate(GObj *factory_gobj)
{
    u32 timer;

    if (!grRemixHazardsMatchHasStarted())
    {
        return;
    }
    timer = sGRRemixScuttleMinionTimer;
    sGRRemixScuttleMinionTimer = timer + 1;
    if (timer < GRREMIX_SCUTTLEMINION_SPAWN_WAIT_TIME)
    {
        return;
    }
    if (!grRemixScuttleMinionRollSpawn(timer))
    {
        return;
    }
    sGRRemixScuttleMinionTimer = 0;
    grRemixScuttleMinionSpawn();
}

/**
 * The minion spawns when the random roll hits 80, or when the timer reaches 3000 without a roll hit.
 * Returns TRUE when the minion spawns.
 */
static sb32 grRemixScuttleMinionRollSpawn(u32 timer)
{
    s32 roll = syUtilsRandIntRange(GRREMIX_SCUTTLEMINION_SPAWN_ROLL_RANGE);

    if (roll == GRREMIX_SCUTTLEMINION_SPAWN_ROLL_HIT)
    {
        return TRUE;
    }
    return timer == GRREMIX_SCUTTLEMINION_SPAWN_TIME_FORCED;
}

/**
 * Spawns the minion at the stage origin (asm minion_coordinates, the spawn of 0x8016EA78 with item id 0x3E).
 * The asm does not start any animation track for the minion: its movement block is commented out.
 * DEVIATION(unreachable in Remix 2.0.1): the asm passes object id 0x03F5 as the first argument of 0x8016EA78
 * (parent_gobj in decomp), and the port passes NULL. Hazards.asm 7338 (`b _end`) skips the whole minion setup, so the
 * factory is never created, see GRREMIX_SCUTTLE_MINIONS_ENABLED in grremixscuttle.c.
 */
static void grRemixScuttleMinionSpawn(void)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *minion_gobj;

    minion_gobj = itManagerMakeItemSetupCommon(NULL, ITREMIX_SCUTTLEMINION_KIND, &pos, &vel, 0);
    if (minion_gobj != NULL)
    {
        itRemixLogEvent("scuttleminion-spawn", ITREMIX_SCUTTLEMINION_KIND);
    }
}

#endif
