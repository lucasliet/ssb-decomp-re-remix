#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixcloakingdevice.h"
#include <it/item.h>
#include <it/itcommon/ittomato.h>
#include <sys/objman.h>
#include <sys/objdef.h>
#include <ft/fighter.h>
#include <if/ifcommon.h>
#include "ft/ftremix/ftremixenvcolor.h"
#include <math.h>

#define ITREMIX_CLOAKINGDEVICE_KIND 0x2D
#define ITREMIX_CLOAKINGDEVICE_FILE_OFFSET 0x0D40
#define ITREMIX_CLOAKINGDEVICE_PORTS 4
#define ITREMIX_CLOAKINGDEVICE_DECLOAK_FRAMES 600
#define ITREMIX_CLOAKINGDEVICE_TRANSITION_FRAMES 32
#define ITREMIX_CLOAKINGDEVICE_OPACITY_PER_FRAME 8
#define ITREMIX_CLOAKINGDEVICE_SPEED_ONLY_FRAMES 4 /* asm: the speed term is added from the 4th frame on */
#define ITREMIX_CLOAKINGDEVICE_CHARGE_LOOP_WORD 5 /* asm 0x8012CB34: first word of the loop body of the script at 0x8012CB20 */
#define ITREMIX_CLOAKINGDEVICE_CHARGE_LOOP_COUNT 2
#define ITREMIX_CLOAKINGDEVICE_FGM_CLOAK 0x0529
#define ITREMIX_CLOAKINGDEVICE_FGM_DECLOAK 0x052A

typedef struct ITRemixCloakingDeviceState
{
    sb32 is_active;
    s32 timer;
} ITRemixCloakingDeviceState;

extern void *func_800269C0_275C0(u16 id);
extern u32 dGMColScriptsFighterCommonSpecialNCharge[];

static ITRemixCloakingDeviceState sITRemixCloakingDeviceStates[ITREMIX_CLOAKINGDEVICE_PORTS];

static ITDesc sITRemixCloakingDeviceItemInfo =
{
    ITREMIX_CLOAKINGDEVICE_KIND,            /* Item.CloakingDevice.id */
    &gITManagerCommonData,                  /* item file 0xFB (asm dw 0x8018D040 = gITManagerCommonData) */
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

/* CloakingDevice: stage_setting_. Remix SPAWN_ITEM (a0 parent, a1 pos, a2 vel, a3 flags) is
 * Item.spawn_custom_item_based_on_tomato_ (Item.asm 374-388), a copy of itTomatoMakeItem: XObj 0x2E, the unused
 * item bool and the item arrow. */
GObj* itRemixCloakingDeviceMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixCloakingDeviceItemInfo, pos, vel, flags);
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

/* Opacity byte of the cloak at this frame (asm handle_active_cloaking_device_): it fades out over the transition,
 * grows back with the fighter's speed and flickers on a few of the final frames. */
static u32 itRemixCloakingDeviceGetOpacity(FTStruct *fp, s32 timer)
{
    s32 frame = (timer < ITREMIX_CLOAKINGDEVICE_TRANSITION_FRAMES) ? timer : ITREMIX_CLOAKINGDEVICE_TRANSITION_FRAMES;
    s32 opacity = -(frame * ITREMIX_CLOAKINGDEVICE_OPACITY_PER_FRAME);

    if (timer >= ITREMIX_CLOAKINGDEVICE_SPEED_ONLY_FRAMES)
    {
        /* CloakingDevice.asm 132-133 reads FTStruct 0x008C/0x0090: the distance moved this frame, knockback included. */
        f32 speed = sqrtf((fp->coll_data.pos_diff.x * fp->coll_data.pos_diff.x) + (fp->coll_data.pos_diff.y * fp->coll_data.pos_diff.y));

        opacity += (s32)speed;
        if (opacity >= 0)
        {
            opacity = -1;
        }
        switch (ITREMIX_CLOAKINGDEVICE_DECLOAK_FRAMES - timer)
        {
        case 30:
            opacity += 0x48;
            break;
        case 22:
            opacity += 0x18;
            break;
        case 18:
            opacity += 0x68;
            break;
        case 9:
            opacity += 0x78;
            break;
        case 5:
            opacity += 0x58;
            break;
        }
    }
    return (u32)opacity & 0xFF;
}

/* CloakingDevice: writes the env colour override of the port for this opacity (asm _set_env_color). The asm ORs the
 * state id 1 for Cloaked where it means 0xFFFFFF00 (docs/remix/deviations.md 5.5). */
static void itRemixCloakingDeviceSetEnvColor(FTStruct *fp, u32 opacity)
{
    FTRemixEnvColorState state = ftRemixEnvColorGetState(fp->player);
    u32 rgba = opacity;

    if ((state == nFTRemixEnvColorStateNormal) || (state == nFTRemixEnvColorStateCloaked))
    {
        rgba |= 0xFFFFFF00;
    }
    else if (rgba == 0)
    {
        rgba = 1;
    }
    ftRemixEnvColorSetOverride(fp->player, rgba);
}

/* CloakingDevice.asm 100-104: while the colanim loop slot (N64 0x0A30) holds the loop of the fully charged script, the loop
 * count (0x0A34) goes back to 2 every frame. The loop never ends, so the script never reaches its Goto and never spawns
 * the charge sparkle effect again. */
static void itRemixCloakingDeviceHoldChargeSparkle(FTStruct *fp)
{
    GMColScript *script = &fp->colanim.cs[0];

    if (script->p_subroutine[0] != (void*) &dGMColScriptsFighterCommonSpecialNCharge[ITREMIX_CLOAKINGDEVICE_CHARGE_LOOP_WORD])
    {
        return;
    }
    script->p_subroutine[1] = (void*) (uintptr_t) ITREMIX_CLOAKINGDEVICE_CHARGE_LOOP_COUNT;
}

/* CloakingDevice: handle_active_cloaking_device_. Runs every frame while a cloak is active. */
static void itRemixCloakingDeviceProcUpdate(GObj *routine_gobj)
{
    FTStruct *fp = (FTStruct*) routine_gobj->user_data.p;
    ITRemixCloakingDeviceState *state = itRemixCloakingDeviceGetState(fp);

    itRemixCloakingDeviceHoldChargeSparkle(fp);
    state->timer += 1;
    if ((state->timer >= ITREMIX_CLOAKINGDEVICE_DECLOAK_FRAMES) || (fp->status_id <= nFTCommonStatusDeadUpFall))
    {
        ftRemixEnvColorRestoreState(fp->player);
        state->is_active = FALSE;
        state->timer = 0;
        gcEjectGObj(routine_gobj);
        func_800269C0_275C0(ITREMIX_CLOAKINGDEVICE_FGM_DECLOAK);
        return;
    }
    itRemixCloakingDeviceSetEnvColor(fp, itRemixCloakingDeviceGetOpacity(fp, state->timer));
}

/* CloakingDevice: starts the cloak from a zero timer. A second pickup while cloaked only restarts the timer: the asm
 * registers a second handler, whose elder plays the decloak sound mid-cloak (docs/remix/deviations.md 5.5). */
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
    /* Not the effect link: ftParamRunProcEffect walks it and reads every user_data as an EFStruct (docs/bugs/remix_routine_gobjs_on_effect_link_2026-10-29.md) */
    routine_gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
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
        ftRemixEnvColorRestoreState(port);
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
