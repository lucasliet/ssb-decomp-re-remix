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
#include "wp/weapon.h"
#include "ft/fighter.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define ITREMIX_GOLDENGUN_FILE_OFFSET 0xFC0 /* asm FILE_OFFSET: item footer in the common item file */
#define ITREMIX_GOLDENGUN_BULLET_KIND 0x26 /* Projectile.id.GOLDENGUN_BULLET (0x20 + 6) */
#define ITREMIX_GOLDENGUN_BULLET_DAMAGE 99 /* PROJECTILE_DAMAGE */
#define ITREMIX_GOLDENGUN_BULLET_KNOCKBACK_SCALE 45 /* KNOCKBACK_1 (0x130) */
#define ITREMIX_GOLDENGUN_BULLET_KNOCKBACK_WEIGHT 0 /* KNOCKBACK_2 (0x134), fixed */
#define ITREMIX_GOLDENGUN_BULLET_KNOCKBACK_BASE 130 /* KNOCKBACK_3 (0x138) */
#define ITREMIX_GOLDENGUN_BULLET_ANGLE 7 /* KNOCKBACK_ANGLE */
#define ITREMIX_GOLDENGUN_BULLET_FGM 52 /* ON_HIT_FGM = FGM.item.BAT */
#define ITREMIX_GOLDENGUN_AMMO_COUNT 1 /* asm AMMO_COUNT, stored in ITStruct.multi (asm offset 0x033E) */

static void itRemixGoldenGunLoadBulletFile(void);

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
    itRemixGoldenGunLoadBulletFile();
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

sb32 itRemixGoldenGunIsHeldBy(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    return (fp->item_gobj != NULL) && (itGetStruct(fp->item_gobj)->kind == ITREMIX_GOLDENGUN_ID);
}

/* projectile_info, its display list, image and vertices (the Remix RAM image from projectile_info up to
 * projectile_info_pointer), copied per spawn into the scene arena as a small file the desc reads at offset 0. */
#define ITREMIX_GOLDENGUN_BULLET_FILE_START REMIX_SYM_Item__GoldenGun__projectile_info
#define ITREMIX_GOLDENGUN_BULLET_FILE_SIZE (REMIX_SYM_Item__GoldenGun__projectile_info_pointer - REMIX_SYM_Item__GoldenGun__projectile_info)

static void *sITRemixGoldenGunBulletFile;

static WPDesc sITRemixGoldenGunBulletDesc =
{
    0x00,                                   /* golden_bullet_projectile_struct 0x00 */
    ITREMIX_GOLDENGUN_BULLET_KIND,
    &sITRemixGoldenGunBulletFile,
    0,
    {
        nGCMatrixKindTraRotRpyRSca,         /* 0x1C000000 */
        nGCMatrixKindNull,
        0
    },
    itLGunWeaponAmmoProcUpdate,             /* 0x801758BC */
    itLGunWeaponAmmoProcMap,                /* 0x80175914 */
    NULL,                                   /* hurtbox hit: 0, the bullet goes through */
    NULL,                                   /* shield: 0 */
    itLGunWeaponAmmoProcHop,                /* 0x80175988 */
    NULL,                                   /* clang: 0 */
    itLGunWeaponAmmoProcReflector,          /* 0x80175A00 */
    NULL                                    /* absorb: 0 */
};

/* The bullet file lives in the scene arena: a gun always spawns in the scene before its first shot. */
static void itRemixGoldenGunLoadBulletFile(void)
{
    sITRemixGoldenGunBulletFile = remix_ram_image_make_file(ITREMIX_GOLDENGUN_BULLET_FILE_START, ITREMIX_GOLDENGUN_BULLET_FILE_SIZE,
                                                            syTaskmanMalloc(ITREMIX_GOLDENGUN_BULLET_FILE_SIZE, 0x10));
}

WPDesc* itRemixGoldenGunGetBulletDesc(void)
{
    return &sITRemixGoldenGunBulletDesc;
}

void itRemixGoldenGunSetupBullet(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);

    itRemixLogEvent("bullet", ITREMIX_GOLDENGUN_ID);
    wp->attack_coll.damage = ITREMIX_GOLDENGUN_BULLET_DAMAGE;
    wp->attack_coll.element = nGMHitElementNormal;
    wp->attack_coll.knockback_scale = ITREMIX_GOLDENGUN_BULLET_KNOCKBACK_SCALE;
    wp->attack_coll.knockback_weight = ITREMIX_GOLDENGUN_BULLET_KNOCKBACK_WEIGHT;
    wp->attack_coll.knockback_base = ITREMIX_GOLDENGUN_BULLET_KNOCKBACK_BASE;
    wp->attack_coll.angle = ITREMIX_GOLDENGUN_BULLET_ANGLE;
    wp->attack_coll.fgm_id = ITREMIX_GOLDENGUN_BULLET_FGM;
}

#endif
