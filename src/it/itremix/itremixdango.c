/*
 * itremixdango.c - C port of the Smash Remix Dango
 * (.toolchain/pkgs/smashremix-src/src/items/Dango.asm).
 *
 * Dango is a Maxim Tomato clone: the spawn is the tomato spawn with the Dango
 * item info, the fall and drop routines are the vanilla tomato ones, and the
 * pickup restores 10 damage before the shared consume tail.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixdango.h"
#include "it/item.h"
#include "it/itmanager.h"
#include "it/itcommon/ittomato.h"
#include "if/ifcommon.h"
#include "ft/fighter.h"
#include "ft/ftparam.h"
#include "ft/ftremix/crash/ftremixcrashshared.h"
#include "sys/obj.h"

#define ITREMIX_DANGO_KIND 0x36
#define ITREMIX_DANGO_FILE_OFFSET 0x1010 /* asm FILE_OFFSET: item footer in file 0xFB */
#define ITREMIX_DANGO_DAMAGE_RESTORE 10 /* asm DAMAGE_RESTORE, percent of damage restored */

/* asm item_info_array. Standard item: the file is the common item file 0xFB. */
static ITDesc sITRemixDangoDesc =
{
    ITREMIX_DANGO_KIND,
    &gITManagerCommonData,
    ITREMIX_DANGO_FILE_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itTomatoFallProcUpdate,             /* 0x801744C0 */
    itTomatoFallProcMap,                /* 0x80174524 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/** Dango.spawn_custom_item_based_on_tomato_ (SPAWN_ITEM): Maxim Tomato spawn with the Dango item info. */
GObj* itRemixDangoMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixDangoDesc, pos, vel, flags);
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

/** Dango.pickup_main_ (PICKUP_ITEM_MAIN): restores damage, then jumps to the tomato consume tail. */
static void itRemixDangoConsume(FTStruct *fp, GObj *item_gobj)
{
    ftRemixCrashEatSfx(fp);
    ftParamSetHealDamage(fp, ITREMIX_DANGO_DAMAGE_RESTORE);
    itRemixConsumeTomatoTail(fp, item_gobj);
}

/** Descriptor of Dango (item id 0x36) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixDangoItemDesc =
{
    ITREMIX_DANGO_KIND,                 /* Item.Dango.id */
    itRemixDangoMakeItem,               /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                               /* PICKUP_ITEM_INIT */
    itRemixDangoConsume,                /* PICKUP_ITEM_MAIN */
    itTomatoDroppedSetStatus,           /* DROP_ITEM (asm 0x801745FC, same as Maxim Tomato) */
    NULL,                               /* THROW_ITEM */
    NULL                                /* PLAYER_COLLISION */
};

#endif
