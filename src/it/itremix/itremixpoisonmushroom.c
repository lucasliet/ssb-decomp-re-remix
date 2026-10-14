/*
 * itremixpoisonmushroom.c - C port of the Smash Remix Poison Mushroom
 * (.toolchain/pkgs/smashremix-src/src/items/PoisonMushroom.asm).
 *
 * The asm item holds only its own item_info_array (FILE_OFFSET 0xDE0) and points every routine
 * at Item.SuperMushroom: the spawn (SPAWN_ITEM), the movement, the ground collision and the
 * player collision that registers the shared effect routine. The port follows that split: this
 * file owns the descriptor and the spawn entry point, while the shared effect state (asm
 * player_shrooms and spawning_shrooms) and all effect routines live in itremixsupermushroom.c
 * and are declared in itremixsupermushroom.h.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixpoisonmushroom.h"
#include "it/itremix/itremixsupermushroom.h"
#include <it/item.h>
#include <it/itmain.h>
#include <it/itvars.h>
#include <it/itcommon/itstar.h>

#define ITREMIX_POISON_MUSHROOM_FILE_OFFSET 0xDE0 /* asm FILE_OFFSET: item footer in file 0xFB */

/* asm PoisonMushroom item_info_array. Words 0x14, 0x18 and PLAYER_COLLISION are
 * Item.SuperMushroom's routines, shared with the Super Mushroom. */
static ITDesc sITRemixPoisonMushroomDesc =
{
    ITREMIX_SUPERMUSHROOM_POISON_KIND,              /* Item.PoisonMushroom.id */
    &gITManagerCommonData,                          /* item file 0xFB (asm dw 0x8018D040) */
    ITREMIX_POISON_MUSHROOM_FILE_OFFSET,

    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itRemixSuperMushroomMovementProcUpdate,         /* asm dw Item.SuperMushroom.movement_routine_no_bounce_ */
    itRemixSuperMushroomGroundProcMap,              /* asm dw Item.SuperMushroom.ground_collision_no_bounce_ */
    itStarCommonProcHit,                            /* 0x80174A0C, same as Star */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/** PoisonMushroom.spawn_custom_item_based_on_star_ (SPAWN_ITEM): the asm routine is
 * SuperMushroom's and resolves the item info array by id, so the spawn velocity (asm
 * spawning_shrooms) and the post-spawn setup are the shared ones. */
GObj* itRemixPoisonMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    Vec3f spawn_vel;
    GObj *item_gobj;

    itRemixSuperMushroomGetSpawnVel(pos, &spawn_vel);
    item_gobj = itManagerMakeItem(parent_gobj, &sITRemixPoisonMushroomDesc, pos, &spawn_vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixSuperMushroomSetupItem(item_gobj);

    return item_gobj;
}

/** Poison-side clear for the scene-end aggregator (see the header). */
void itRemixPoisonMushroomClearActive(void)
{
    itRemixSuperMushroomClearStates();
}

/** Descriptor of Poison Mushroom (item id 0x2F) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixPoisonMushroomItemDesc =
{
    ITREMIX_SUPERMUSHROOM_POISON_KIND,      /* Item.PoisonMushroom.id */
    itRemixPoisonMushroomMakeItem,          /* SPAWN_ITEM */
    TRUE,                                   /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                                   /* PICKUP_ITEM_INIT */
    NULL,                                   /* PICKUP_ITEM_MAIN */
    NULL,                                   /* DROP_ITEM */
    NULL,                                   /* THROW_ITEM */
    itRemixSuperMushroomProcTouch           /* PLAYER_COLLISION (Item.SuperMushroom.collide_mushroom_) */
};

#endif
