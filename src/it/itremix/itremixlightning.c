#ifdef PORT

#include "it/itremix/itremixlightning.h"
#include "it/itremix/itremixsupermushroom.h"
#include "ft/ftremix/ftremixhooks.h"
#include <it/item.h>
#include <it/itcommon/ittomato.h>
#include <ef/efmanager.h>
#include <ft/fighter.h>
#include <if/ifcommon.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>

#define ITREMIX_LIGHTNING_KIND 0x31
#define ITREMIX_LIGHTNING_FILE_OFFSET 0xE80
#define ITREMIX_LIGHTNING_TIME 25
#define ITREMIX_LIGHTNING_SFX_TIME 20 /* ITREMIX_LIGHTNING_TIME - zap_sfx_delay (5) */
#define ITREMIX_LIGHTNING_SFX_FGM 0x00E8
#define ITREMIX_LIGHTNING_SHRINK_RESET_TIME 0x01E0 /* written to every port slot before a zap */
#define ITREMIX_LIGHTNING_SHRUNK_SIZE 0.5F /* lui t4, 0x3F00 in _zap_player */
#define ITREMIX_LIGHTNING_SHRINK_TIME 600 /* lightning_shrink_timer, 10 seconds at 60 tics per second */
#define ITREMIX_LIGHTNING_ROUTINE_MAX 8
#define ITREMIX_LIGHTNING_ACTION_REVIVE1 0x007 /* Remix Action.Revive1 */
#define ITREMIX_LIGHTNING_ACTION_REVIVE2 0x008 /* Remix Action.Revive2 */
#define ITREMIX_LIGHTNING_ACTION_REVIVE_WAIT 0x009 /* Remix Action.ReviveWait */
#define ITREMIX_LIGHTNING_PROCESS_PRIORITY 3

typedef struct ITRemixLightningRoutine
{
    GObj *routine_gobj;
    FTStruct *owner_fp;
    u8 timer;
} ITRemixLightningRoutine;

extern void *func_800269C0_275C0(u16 id);

static void itRemixLightningRoutineProcUpdate(GObj *routine_gobj);
static void itRemixLightningShrinkProcUpdate(GObj *shrink_gobj);

static ITRemixLightningRoutine sITRemixLightningRoutines[ITREMIX_LIGHTNING_ROUTINE_MAX];
static GObj *sITRemixLightningShrinkGObj;
static s32 sITRemixLightningShrinkTimers[MAXCONTROLLERS];

static ITDesc sITRemixLightningItemInfo =
{
    ITREMIX_LIGHTNING_KIND,                 /* Item.Lightning.id */
    &gITManagerCommonData,                  /* item file 0xFB, see notes REMIX-TODO(file) */
    ITREMIX_LIGHTNING_FILE_OFFSET,          /* item footer offset */

    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itTomatoFallProcUpdate,                 /* 0x801744C0 */
    itTomatoFallProcMap,                    /* 0x80174524 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* Lightning: spawn_custom_item_based_on_tomato_. Same as itTomatoMakeItem with the Lightning ITDesc */
GObj* itRemixLightningMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixLightningItemInfo, pos, vel, flags);
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    if (item_gobj == NULL)
    {
        return NULL;
    }
    joint = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);
    translate = joint->translate.vec.f;

    gcAddXObjForDObjFixed(joint, 0x2E, 0);

    joint->translate.vec.f = translate;
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/* Lightning: pickup_lightning_ register_routine aerial_main_ */
static ITRemixLightningRoutine* itRemixLightningAllocRoutine(void)
{
    s32 i;

    for (i = 0; i < ITREMIX_LIGHTNING_ROUTINE_MAX; i++)
    {
        if (sITRemixLightningRoutines[i].routine_gobj == NULL)
        {
            return &sITRemixLightningRoutines[i];
        }
    }
    return NULL;
}

/* Lightning: starts one routine per pickup. The pool is the port's limit; a full pool consumes the item without a zap */
static void itRemixLightningStartRoutine(FTStruct *fp)
{
    ITRemixLightningRoutine *routine = itRemixLightningAllocRoutine();
    GObj *routine_gobj;

    if (routine == NULL)
    {
        return;
    }
    routine_gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    routine_gobj->user_data.p = routine;
    routine->routine_gobj = routine_gobj;
    routine->owner_fp = fp;
    routine->timer = ITREMIX_LIGHTNING_TIME;
    gcAddGObjProcess(routine_gobj, itRemixLightningRoutineProcUpdate, nGCProcessKindFunc, ITREMIX_LIGHTNING_PROCESS_PRIORITY);
}

/* Lightning: aerial_main_ _end_lightning */
static void itRemixLightningEndRoutine(ITRemixLightningRoutine *routine)
{
    /* REMIX-TODO(GFX): the asm also sets the camera draw mode back to 4 here. */
    gcEjectGObj(routine->routine_gobj);
    routine->routine_gobj = NULL;
    routine->owner_fp = NULL;
}

/* Lightning: lightning_zap_players_ team check (team battle on, team attack off, same team) */
static sb32 itRemixLightningIsProtectedTeammate(FTStruct *owner_fp, FTStruct *fp)
{
    if (gSCManagerVSBattleState.is_team_battle == FALSE)
    {
        return FALSE;
    }
    if (gSCManagerVSBattleState.is_team_attack != FALSE)
    {
        return FALSE;
    }
    return owner_fp->team == fp->team;
}

/* Lightning: lightning_zap_players_ action check (Revive1, Revive2, ReviveWait) */
static sb32 itRemixLightningIsReviving(FTStruct *fp)
{
    return (fp->status_id == ITREMIX_LIGHTNING_ACTION_REVIVE1) ||
           (fp->status_id == ITREMIX_LIGHTNING_ACTION_REVIVE2) ||
           (fp->status_id == ITREMIX_LIGHTNING_ACTION_REVIVE_WAIT);
}

/* Lightning: lightning_zap_players_ checks before a zap, in asm order */
static sb32 itRemixLightningCanZap(FTStruct *owner_fp, FTStruct *fp)
{
    if (fp == owner_fp)
    {
        return FALSE;
    }
    if (itRemixLightningIsProtectedTeammate(owner_fp, fp))
    {
        return FALSE;
    }
    if (itRemixLightningIsReviving(fp))
    {
        return FALSE;
    }
    if (fp->invincible_tics != 0)
    {
        return FALSE;
    }
    return fp->star_invincible_tics == 0;
}

/* Lightning: lightning_zap_players_ _zap_player */
static void itRemixLightningZapPlayer(FTStruct *fp)
{
    efManagerThunderAmpMakeEffect(fp->coll_data.p_translate);
    sITRemixLightningShrinkTimers[fp->player] = ITREMIX_LIGHTNING_SHRINK_TIME;
    ftRemixSizeSetMultiplier(fp->player, ITREMIX_LIGHTNING_SHRUNK_SIZE);
    itRemixSuperMushroomDestroyForPlayer(fp->player);
    /* REMIX-TODO(FranklinBadge): a Franklin Badge does not reflect the zap; every eligible player is zapped. */
}

/* Lightning: aerial_main_ zap frame, then lightning_zap_players_ */
static void itRemixLightningZapPlayers(FTStruct *owner_fp)
{
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];
    FTStruct *fp;
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        sITRemixLightningShrinkTimers[port] = ITREMIX_LIGHTNING_SHRINK_RESET_TIME;
    }
    while (fighter_gobj != NULL)
    {
        fp = ftGetStruct(fighter_gobj);
        if (itRemixLightningCanZap(owner_fp, fp))
        {
            itRemixLightningZapPlayer(fp);
        }
        fighter_gobj = fighter_gobj->link_next;
    }
}

/* Lightning: aerial_main_ shrink routine registration, done once until ClearActive */
static void itRemixLightningEnsureShrinkRoutine(void)
{
    if (sITRemixLightningShrinkGObj != NULL)
    {
        return;
    }
    sITRemixLightningShrinkGObj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(sITRemixLightningShrinkGObj, itRemixLightningShrinkProcUpdate, nGCProcessKindFunc, ITREMIX_LIGHTNING_PROCESS_PRIORITY);
}

/* Lightning: aerial_main_ per-frame flow. The asm's 0xFF timer branch is unreachable, since the end check runs first */
static void itRemixLightningRoutineProcUpdate(GObj *routine_gobj)
{
    ITRemixLightningRoutine *routine = routine_gobj->user_data.p;

    if (routine->timer == ITREMIX_LIGHTNING_TIME)
    {
        itRemixLightningEnsureShrinkRoutine();
        itRemixLightningZapPlayers(routine->owner_fp);
    }
    else if (routine->timer == ITREMIX_LIGHTNING_SFX_TIME)
    {
        func_800269C0_275C0(ITREMIX_LIGHTNING_SFX_FGM);
    }
    else if (routine->timer == 0)
    {
        itRemixLightningEndRoutine(routine);
        return;
    }
    /* REMIX-TODO(GFX): the asm sets the global camera draw colour (dark blue or white, bit 4 of the timer) and mode 2 here. */
    routine->timer -= 1;
}

/* Lightning: shrink_timer_ per port */
static void itRemixLightningShrinkTick(s32 port)
{
    if (sITRemixLightningShrinkTimers[port] == 0)
    {
        return;
    }
    if (itRemixSuperMushroomIsActive(port) != FALSE)
    {
        sITRemixLightningShrinkTimers[port] = 0;
        return;
    }
    if (sITRemixLightningShrinkTimers[port] == 1)
    {
        /* REMIX-TODO(Size): the asm restores the size of Size.match_state_table (GIANT/TINY match modes, ROADMAP T2.4); every match is NORMAL today. */
        ftRemixSizeSetMultiplier(port, 1.0F);
        sITRemixLightningShrinkTimers[port] = 0;
        return;
    }
    sITRemixLightningShrinkTimers[port] -= 1;
}

/* Lightning: shrink_timer_ */
static void itRemixLightningShrinkProcUpdate(GObj *shrink_gobj)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        itRemixLightningShrinkTick(port);
    }
}

/* Lightning: pickup_lightning_. The camera flash is not ported (REMIX-TODO(GFX)) */
static void itRemixLightningConsume(FTStruct *fp, GObj *item_gobj)
{
    itRemixLightningStartRoutine(fp);
    itRemixConsumeTomatoTail(fp, item_gobj);
}

/* Lightning: clear_active_lightning_routine_ and the shrink timers, plus the routine pool (its GObjs are freed with the scene) */
void itRemixLightningClearActive(void)
{
    s32 i;

    for (i = 0; i < ITREMIX_LIGHTNING_ROUTINE_MAX; i++)
    {
        sITRemixLightningRoutines[i].routine_gobj = NULL;
        sITRemixLightningRoutines[i].owner_fp = NULL;
        sITRemixLightningRoutines[i].timer = 0;
    }
    sITRemixLightningShrinkGObj = NULL;
    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        sITRemixLightningShrinkTimers[i] = 0;
    }
}

const ITRemixItemDesc dITRemixLightningItemDesc =
{
    ITREMIX_LIGHTNING_KIND,
    itRemixLightningMakeItem,
    FALSE,                                  /* SHOW_GFX_WHEN_SPAWNED = OS.FALSE */
    NULL,                                   /* PICKUP_ITEM_INIT = 0 */
    itRemixLightningConsume,                /* PICKUP_ITEM_MAIN: pickup_lightning_ */
    itTomatoDroppedSetStatus,               /* DROP_ITEM: 0x801745FC, same as Maxim Tomato */
    NULL,                                   /* THROW_ITEM = 0 */
    NULL                                    /* PLAYER_COLLISION = 0 */
};

#endif
