/**
 * itremixgoldengun.c - C port of the Smash Remix Golden Gun item
 * (.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm).
 *
 * The item reuses the vanilla Ray Gun hold, drop, throw and fall routines. The Remix
 * code is the spawn routine (stage_setting_) and the ammo count. Each routine names
 * its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixgoldengun.h"
#include "it/item.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "it/itcommon/itlgun.h"
#include "it/itcommon/ittomato.h"
#include "if/ifcommon.h"
#include "sys/utils.h"

#define ITREMIX_GOLDENGUN_ID 0x35
#define ITREMIX_GOLDENGUN_FILE_OFFSET 0xFC0 /* asm FILE_OFFSET: item footer in the common item file */
#define ITREMIX_GOLDENGUN_AMMO_COUNT 1 /* asm AMMO_COUNT, stored in ITStruct.multi (asm offset 0x033E) */

enum itRemixGoldenGunStatus
{
    nITRemixGoldenGunStatusGrounded,
    nITRemixGoldenGunStatusAerial,
    nITRemixGoldenGunStatusEnumCount
};

/* asm item_state_table. No routine in this port selects it: the hold, drop and throw
 * routines use the vanilla Ray Gun status table, so this table is kept for parity only. */
static const ITStatusDesc sITRemixGoldenGunStatusDescs[nITRemixGoldenGunStatusEnumCount] =
{
    {
        NULL,
        itLGunWaitProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },
    {
        itTomatoFallProcUpdate,
        itTomatoFallProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/* asm item_info_array. p_file points at the common item file (0xFB), as in ittomato.c. */
static ITDesc sITRemixGoldenGunDesc =
{
    ITREMIX_GOLDENGUN_ID,
    &gITManagerCommonData,
    ITREMIX_GOLDENGUN_FILE_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itLGunFallProcUpdate,
    itLGunFallProcMap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/**
 * GoldenGun.stage_setting_ tilt roll. The asm advances the RNG once, then sets the DObj
 * rotate.vec.f.y (offset 0x34) to float 0x8018CD00 or 0x8018CD04 by the low bit of the
 * result. The two floats are not in the port yet, so only the RNG advance is kept, which
 * keeps the spawn random sequence unchanged.
 */
static void itRemixGoldenGunRollTilt(void)
{
    /* REMIX-TODO(tilt): set rotate.vec.f.y from float 0x8018CD00 or 0x8018CD04, chosen by the low bit of the RNG value below. */
    syUtilsRandUShort();
}

/**
 * GoldenGun.stage_setting_ (SPAWN_ITEM). The vanilla ABI is (parent, pos, vel, flags).
 * Vanilla itManagerMakeItem is the asm's jal 0x8016E174.
 */
GObj* itRemixGoldenGunMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixGoldenGunDesc, pos, vel, flags);
    ITStruct *ip;

    if (item_gobj == NULL)
    {
        return NULL;
    }
    ip = itGetStruct(item_gobj);
    ip->multi = ITREMIX_GOLDENGUN_AMMO_COUNT;
    itRemixGoldenGunRollTilt();
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/** Descriptor of GoldenGun (item id 0x35) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixGoldenGunItemDesc =
{
    ITREMIX_GOLDENGUN_ID,               /* Item.GoldenGun.id */
    itRemixGoldenGunMakeItem,           /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    itLGunHoldSetStatus,                /* PICKUP_ITEM_INIT (asm 0x801755FC, copied Ray Gun) */
    itRemixConsumeTomatoTail,           /* PICKUP_ITEM_MAIN: pickup_item jumps to 0x80145C4C */
    itLGunDroppedSetStatus,             /* DROP_ITEM (asm 0x80175780, copied Ray Gun) */
    itLGunThrownSetStatus,              /* THROW_ITEM (asm 0x801756AC, copied Ray Gun) */
    NULL                                /* PLAYER_COLLISION */
};

#endif
