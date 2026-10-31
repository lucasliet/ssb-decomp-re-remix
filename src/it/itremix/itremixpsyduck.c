/**
 * itremixpsyduck.c - C port of the Smash Remix Psyduck item
 * (.toolchain/pkgs/smashremix-src/src/items/Pokemon/Psyduck.asm).
 *
 * Psyduck spawns like the vanilla Sawamura Pokemon (itSawamuraMakeItem) with its own
 * item footer and animation bank offsets. Its states and procs are the vanilla Sawamura
 * ones, so the status table is dITSawamuraStatusDescs rather than a copy.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixpsyduck.h"
#include <it/item.h>
#include <it/itmanager.h>
#include <it/itvars.h>
#include <it/itmonster/itsawamura.h>
#include <gm/gmdef.h>
#include <sys/objanim.h>
#include <sys/objdef.h>
#include <sys/objman.h>

#define ITREMIX_PSYDUCK_KIND 0x41
#define ITREMIX_PSYDUCK_FILE_OFFSET 0x0BB0 /* asm FILE_OFFSET: item footer in file 0xFB */
#define ITREMIX_PSYDUCK_ANIM_BASE_OFFSET 0x11F40 /* asm: attr->data - 0x11F40 + 0x13624 */
#define ITREMIX_PSYDUCK_ANIM_TARGET_OFFSET 0x13624
#define ITREMIX_PSYDUCK_APPEAR_FGM 0x013E /* asm FGM literal at stage_setting_ */
#define ITREMIX_PSYDUCK_JOINT_KIND 0x48 /* gcAddXObjForDObjFixed kind, as vanilla Sawamura */
#define ITREMIX_PSYDUCK_DL_LINK 18 /* gcMoveGObjDLHead link, as vanilla Sawamura */

extern void *func_800269C0_275C0(u16 id);

static void itRemixPsyduckSetup(GObj *item_gobj, Vec3f *pos);

/* asm item_info_array. Attack state is 0 (Off); vanilla Sawamura uses nGMAttackStateNew. */
static ITDesc sITRemixPsyduckItemInfo =
{
    ITREMIX_PSYDUCK_KIND,
    &gITManagerCommonData,                  /* item file 0xFB: asm dw 0x8018D040 */
    ITREMIX_PSYDUCK_FILE_OFFSET,

    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itSawamuraCommonProcUpdate,             /* 0x80182AE0 */
    itSawamuraCommonProcMap,                /* 0x80182B34 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* Psyduck.stage_setting_. Remix SPAWN_ITEM (a0 parent, a1 pos, a2 vel, a3 flags) */
GObj* itRemixPsyduckMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixPsyduckItemInfo, pos, vel, flags);

    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixPsyduckSetup(item_gobj, pos);

    return item_gobj;
}

/* Psyduck.stage_setting_ after itManagerMakeItem: rise velocity, timer, model position, animation, sound */
static void itRemixPsyduckSetup(GObj *item_gobj, Vec3f *pos)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    ITStruct *ip = itGetStruct(item_gobj);

    ip->multi = ITMONSTER_RISE_STOP_WAIT;
    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = ITMONSTER_RISE_VEL_Y;
    ip->physics.vel_air.z = 0.0F;

    gcAddXObjForDObjFixed(dobj, ITREMIX_PSYDUCK_JOINT_KIND, 0);

    dobj->translate.vec.f = *pos;
    dobj->translate.vec.f.y -= ip->attr->map_coll_bottom;

    gcAddDObjAnimJoint(dobj, itGetPData(ip, ITREMIX_PSYDUCK_ANIM_BASE_OFFSET, ITREMIX_PSYDUCK_ANIM_TARGET_OFFSET), 0.0F);
    func_800269C0_275C0(ITREMIX_PSYDUCK_APPEAR_FGM);
    gcMoveGObjDLHead(item_gobj, ITREMIX_PSYDUCK_DL_LINK, item_gobj->dl_link_priority);
}

/* Descriptor of Psyduck (item id 0x41). Pickup, drop, throw and touch are 0 in the asm. */
const ITRemixItemDesc dITRemixPsyduckItemDesc =
{
    ITREMIX_PSYDUCK_KIND,               /* Item.Psyduck.id */
    itRemixPsyduckMakeItem,             /* SPAWN_ITEM */
    FALSE,                              /* SHOW_GFX_WHEN_SPAWNED(OS.FALSE) */
    NULL,                               /* PICKUP_ITEM_INIT(0) */
    NULL,                               /* PICKUP_ITEM_MAIN(0) */
    NULL,                               /* DROP_ITEM(0) */
    NULL,                               /* THROW_ITEM(0) */
    NULL                                /* PLAYER_COLLISION(0) */
};

#endif
