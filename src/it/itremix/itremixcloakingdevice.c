#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixcloakingdevice.h"
#include <it/item.h>
#include <it/itcommon/ittomato.h>
#include <sys/objman.h>
#include <sys/objdef.h>
#include <ft/fighter.h>

#define ITREMIX_CLOAKINGDEVICE_KIND 0x2D
#define ITREMIX_CLOAKINGDEVICE_FILE_OFFSET 0x0D40
#define ITREMIX_CLOAKINGDEVICE_PORTS 4
#define ITREMIX_CLOAKINGDEVICE_DECLOAK_FRAMES 600
#define ITREMIX_CLOAKINGDEVICE_FGM_CLOAK 0x0529
#define ITREMIX_CLOAKINGDEVICE_FGM_DECLOAK 0x052A

typedef struct ITRemixCloakingDeviceState
{
    sb32 is_active;
    s32 timer;
} ITRemixCloakingDeviceState;

extern void *func_800269C0_275C0(u16 id);

static ITRemixCloakingDeviceState sITRemixCloakingDeviceStates[ITREMIX_CLOAKINGDEVICE_PORTS];

static ITDesc sITRemixCloakingDeviceItemInfo =
{
    ITREMIX_CLOAKINGDEVICE_KIND,            /* Item.CloakingDevice.id */
    &gITManagerCommonData,                  /* item file 0xFB, see notes REMIX-TODO(file) */
    ITREMIX_CLOAKINGDEVICE_FILE_OFFSET,     /* item footer offset */

    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itTomatoFallProcUpdate,                 /* 0x801744C0, same as Maxim Tomato (no own status table) */
    itTomatoFallProcMap,                    /* 0x80174524 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* CloakingDevice: returns the cloak state of the player's port */
static ITRemixCloakingDeviceState* itRemixCloakingDeviceGetState(FTStruct *fp)
{
    return &sITRemixCloakingDeviceStates[fp->player];
}

/* CloakingDevice: stage_setting_. Remix SPAWN_ITEM (a0 parent, a1 pos, a2 vel, a3 flags) */
GObj* itRemixCloakingDeviceMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    return itManagerMakeItem(parent_gobj, &sITRemixCloakingDeviceItemInfo, pos, vel, flags);
}

/* CloakingDevice: handle_active_cloaking_device_. Runs every frame while a cloak is active.
 * REMIX-TODO(CharEnvColor): the asm fades the env color override over 32 frames and flickers near the end.
 * The port has only a binary is_invisible, so the cloaked fighter is fully hidden. */
static void itRemixCloakingDeviceProcUpdate(GObj *routine_gobj)
{
    FTStruct *fp = (FTStruct*) routine_gobj->user_data.p;
    ITRemixCloakingDeviceState *state = itRemixCloakingDeviceGetState(fp);

    state->timer += 1;
    if ((state->timer >= ITREMIX_CLOAKINGDEVICE_DECLOAK_FRAMES) || (fp->status_id <= nFTCommonStatusDeadUpFall))
    {
        fp->is_invisible = FALSE;
        state->is_active = FALSE;
        state->timer = 0;
        gcEjectGObj(routine_gobj);
        func_800269C0_275C0(ITREMIX_CLOAKINGDEVICE_FGM_DECLOAK);
        return;
    }
    fp->is_invisible = TRUE;
}

/* CloakingDevice: starts the cloak from a zero timer. A second pickup while cloaked only restarts the timer. */
static void itRemixCloakingDeviceStart(FTStruct *fp)
{
    ITRemixCloakingDeviceState *state = itRemixCloakingDeviceGetState(fp);
    GObj *routine_gobj;

    state->timer = 0;
    if (state->is_active)
    {
        return;
    }
    state->is_active = TRUE;
    routine_gobj = gcMakeGObjSPAfter(nGCCommonKindEffect, NULL, nGCCommonLinkIDEffect, GOBJ_PRIORITY_DEFAULT);
    routine_gobj->user_data.p = fp;
    gcAddGObjProcess(routine_gobj, itRemixCloakingDeviceProcUpdate, nGCProcessKindFunc, 3);
}

/* CloakingDevice: pickup_cloaking_device_ (PICKUP_ITEM_MAIN). Starts the cloak, plays the sfx, then jumps to 0x80145C4C. */
static void itRemixCloakingDevicePickupMain(FTStruct *fp, GObj *item_gobj)
{
    itRemixCloakingDeviceStart(fp);
    func_800269C0_275C0(ITREMIX_CLOAKINGDEVICE_FGM_CLOAK);
    itRemixConsumeTomatoTail(fp, item_gobj);
}

/* CloakingDevice: the asm's clear_active_cloaking_devices_ state reset. The routine GObjs are freed with the scene. */
void itRemixCloakingDeviceClearStates(void)
{
    s32 port;

    for (port = 0; port < ITREMIX_CLOAKINGDEVICE_PORTS; port++)
    {
        sITRemixCloakingDeviceStates[port].is_active = FALSE;
        sITRemixCloakingDeviceStates[port].timer = 0;
    }
}

/* CloakingDevice: prevent_damage_when_cloaked_ reads this (asm cloaked_players[port]). */
sb32 itRemixCloakingDeviceIsCloaked(s32 player)
{
    if ((player < 0) || (player >= ITREMIX_CLOAKINGDEVICE_PORTS))
    {
        return FALSE;
    }
    return sITRemixCloakingDeviceStates[player].is_active;
}

const ITRemixItemDesc dITRemixCloakingDeviceItemDesc =
{
    ITREMIX_CLOAKINGDEVICE_KIND,
    itRemixCloakingDeviceMakeItem,
    TRUE,
    NULL,                                   /* PICKUP_ITEM_INIT: consume type, no hold routine */
    itRemixCloakingDevicePickupMain,        /* PICKUP_ITEM_MAIN */
    itTomatoDroppedSetStatus,               /* DROP_ITEM (0x801745FC, same as Maxim Tomato) */
    NULL,                                   /* THROW_ITEM: 0 in the asm */
    NULL                                    /* PLAYER_COLLISION: 0 in the asm */
};

#endif
