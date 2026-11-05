/**
 * grremixrollingbombs.c - rolling bombs of the Remix bonus stages, see grremixrollingbombs.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixrollingbombs.h"
#include "gr/grremix/grremixbonusrules.h"
#include <gr/ground.h>
#include <it/item.h>
#include <sc/scene.h>

typedef struct GRRemixRollingBombState
{
    void *item_head;
    Vec3f spawn_pos;
    s32 spawn_wait;

} GRRemixRollingBombState;

static GRRemixRollingBombState sGRRemixRollingBombs;

/** Spawn point and first wait of the current stage; FALSE when it has no spawner. */
static sb32 grRemixRollingBombsGetSpawn(f32 *x, f32 *y, s32 *first_wait)
{
    return grRemixBonusRulesGetRollingBomb(gSCManagerBattleState->gkind, x, y, first_wait) != 0;
}

/** Copy of grBonus3TaruBombProcUpdate reading the spawn point and timer of this module (rolling_bomb_position). */
static void grRemixRollingBombsProcUpdate(GObj *actor_gobj)
{
    Vec3f vel;

    if (sGRRemixRollingBombs.spawn_wait == 0)
    {
        vel.x = vel.y = vel.z = 0.0F;

        itManagerMakeItemSetupCommon(NULL, nITKindTaruBomb, &sGRRemixRollingBombs.spawn_pos, &vel, ITEM_FLAG_PARENT_GROUND);

        sGRRemixRollingBombs.spawn_wait = GRREMIX_BONUS_BOMB_WAIT_REPEAT;
    }
    sGRRemixRollingBombs.spawn_wait--;
}

void grRemixRollingBombsMakeGround(void)
{
    f32 x;
    f32 y;
    s32 first_wait;

    if (grRemixRollingBombsGetSpawn(&x, &y, &first_wait) == FALSE)
    {
        return;
    }
    sGRRemixRollingBombs.item_head = (void*) gMPCollisionGroundData; /* llGRBonus3MapItemHead is 0 */
    sGRRemixRollingBombs.spawn_pos.x = x;
    sGRRemixRollingBombs.spawn_pos.y = y;
    sGRRemixRollingBombs.spawn_pos.z = 0.0F;
    sGRRemixRollingBombs.spawn_wait = first_wait;

    gcAddGObjProcess(gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT), grRemixRollingBombsProcUpdate, nGCProcessKindFunc, 4);
}

sb32 grRemixRollingBombsIsStage(void)
{
    f32 x;
    f32 y;
    s32 first_wait;

    return grRemixRollingBombsGetSpawn(&x, &y, &first_wait);
}

void** grRemixRollingBombsGetItemHeadSlot(void)
{
    return &sGRRemixRollingBombs.item_head;
}

#endif
