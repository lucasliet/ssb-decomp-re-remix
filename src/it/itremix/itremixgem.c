/**
 * itremixgem.c - C port of the Smash Remix Gem item
 * (.toolchain/pkgs/smashremix-src/src/items/Gem.asm).
 *
 * Gem reuses the Bob-omb throw and explode routines, the Maxim Tomato fall
 * routines and the red and green shell hit and bounce routines. Each routine
 * names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixgem.h"
#include "it/itmain.h"
#include "it/itmap.h"
#include "it/itmanager.h"
#include "it/itcommon/itbombhei.h"
#include "it/itcommon/itgshell.h"
#include "it/itcommon/itrshell.h"
#include "it/itcommon/ittomato.h"
#include "if/ifcommon.h"
#include "ef/efmanager.h"
#include "sys/objman.h"
#include "sys/utils.h"

#define ITREMIX_GEM_ID 0x42
#define ITREMIX_GEM_FILE_OFFSET 0x40 /* asm FILE_OFFSET: item footer in the owner's item file */
#define ITREMIX_GEM_HIT_FGM 0x105 /* asm hitbox FGM set on spawn */
#define ITREMIX_GEM_SHATTER_FGM 0x33 /* asm SHATTER_FGM (fan smack) */
#define ITREMIX_GEM_CRACKED_IMAGE 1 /* MObj texture index of the cracked gem */
#define ITREMIX_GEM_HEALTH_MAX 4 /* asm passes a0 = 4 to syUtilsRandIntRange */
#define ITREMIX_GEM_BOUNCE_VEL_Y 38.0F /* 0x42180000 */
#define ITREMIX_GEM_BOUNCE_MUL_X 0.125F /* 0x3E000000 */
#define ITREMIX_GEM_LANDING_VEL_MUL -0.25F /* 0xBE800000, applied when the hurtbox is hit uncracked */
#define ITREMIX_GEM_HIT_VEL_MUL_X 0.7F /* RAM 0x8018CDA8; same literal as ITRSHELL_RECOIL_MUL_X, not verified against the ROM */
#define ITREMIX_GEM_PICKUP_WAIT_MSB 0x800 /* asm clears the top bit of the byte at 0x2CE, which is pickup_wait bit 11 */

enum itRemixGemStatus
{
    nITRemixGemStatusWait,
    nITRemixGemStatusFall,
    nITRemixGemStatusPickup,
    nITRemixGemStatusThrown,
    nITRemixGemStatusCollideClipping,
    nITRemixGemStatusEnumCount
};

typedef struct ITRemixGemVariant
{
    f32 palette_index;
    s32 damage;
    s32 knockback_base;
    s32 knockback_scale;
} ITRemixGemVariant;

extern void* func_800269C0_275C0(u16 id);

static void itRemixGemApplyMultiplier(GObj *item_gobj, f32 multiplier);
static void itRemixGemDisableBoxes(GObj *item_gobj);
static void itRemixGemShatterGfx(GObj *item_gobj);
static void itRemixGemDestroy(GObj *item_gobj);
static void itRemixGemCollideTransition(GObj *item_gobj);
static sb32 itRemixGemCollideWithClipping(GObj *item_gobj);
static sb32 itRemixGemThrownProcMap(GObj *item_gobj);
static sb32 itRemixGemHurtboxCollision(GObj *item_gobj);
static sb32 itRemixGemCollideWithPlayer(GObj *item_gobj);
static void itRemixGemBounceBack(GObj *item_gobj);
static void itRemixGemSetFallStatus(GObj *item_gobj);
static void itRemixGemPrepickup(GObj *item_gobj);
static void itRemixGemThrowInitial(GObj *item_gobj);
static void itRemixGemApplyVariant(GObj *item_gobj, const ITRemixGemVariant *variant);

/* asm green_gem: the variant used when an owner spawns the gem. */
static const ITRemixGemVariant sITRemixGemOwnedVariant =
{
    1.0F,   /* palette 0x3F80 as float 1.0F */
    4,      /* base damage */
    40,     /* base knockback */
    90      /* knockback growth */
};

/* asm item_info_array. p_file is bound to the owner's file per spawn. */
static ITDesc sITRemixGemDesc =
{
    ITREMIX_GEM_ID,
    NULL,
    ITREMIX_GEM_FILE_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itTomatoFallProcUpdate,
    itTomatoFallProcMap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* asm item_state_table. The asm labels the last block "STATE 6", but its
 * offset 0x80 is state 4, which is the one collide_transition sets. */
static ITStatusDesc sITRemixGemStatusDescs[nITRemixGemStatusEnumCount] =
{
    {
        NULL,
        itTomatoWaitProcMap,
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
    },
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },
    {
        itBombHeiThrownProcUpdate,
        itRemixGemThrownProcMap,
        itRemixGemCollideWithPlayer,
        itRemixGemCollideWithPlayer,
        itMainCommonProcHop,
        itRemixGemCollideWithPlayer,
        itMainCommonProcReflector,
        itRemixGemHurtboxCollision
    },

    {
        itRemixGemCollideWithClipping,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/** Gem.apply_multiplier_: scales the item's horizontal and vertical velocity. */
static void itRemixGemApplyMultiplier(GObj *item_gobj, f32 multiplier)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x *= multiplier;
    ip->physics.vel_air.y *= multiplier;
}

/** Gem.falling_initial_: turns off the hurtbox and the hitbox while the gem falls. */
static void itRemixGemDisableBoxes(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = nGMHitStatusNone;
    ip->attack_coll.attack_state = nGMAttackStateOff;
}

/** Gem.shatter_gfx_: spawns the shatter effect at the gem's position, offset to its base. */
static void itRemixGemShatterGfx(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    Vec3f pos = DObjGetStruct(item_gobj)->translate.vec.f;

    pos.y += ip->attr->map_coll_bottom;
    efManagerShieldBreakMakeEffect(&pos);
    ip->item_vars.shell.interact = 8;
}

/** Gem.destroy_gem_: shatter effect and sound. */
static void itRemixGemDestroy(GObj *item_gobj)
{
    itRemixGemShatterGfx(item_gobj);
    func_800269C0_275C0(ITREMIX_GEM_SHATTER_FGM);
}

/** Gem.collide_transition: marks the gem cracked and moves it to the clipping collision status. */
static void itRemixGemCollideTransition(GObj *item_gobj)
{
    MObj *mobj = DObjGetStruct(item_gobj)->mobj;

    mobj->texture_id_curr = ITREMIX_GEM_CRACKED_IMAGE;
    itMainSetStatus(item_gobj, sITRemixGemStatusDescs, nITRemixGemStatusCollideClipping);
}

/** Gem.collide_with_clipping_: shatters the gem when it hits a wall or the ground. */
static sb32 itRemixGemCollideWithClipping(GObj *item_gobj)
{
    itBombHeiCommonCheckMakeDustEffect(item_gobj, FALSE);
    itRemixGemDestroy(item_gobj);

    return TRUE;
}

/** Gem.throw_duration_ (thrown proc_map): runs the map check, transitioning to clipping on contact. */
static sb32 itRemixGemThrownProcMap(GObj *item_gobj)
{
    return itMapCheckMapProcAll(item_gobj, itRemixGemCollideTransition);
}

/** Gem.hurtbox_collision_ (thrown proc_damage): cracks the gem, or shatters it when already cracked. */
static sb32 itRemixGemHurtboxCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    MObj *mobj = DObjGetStruct(item_gobj)->mobj;
    s32 image_index_prev = mobj->texture_id_curr;

    ip->attack_coll.fgm_id = ITREMIX_GEM_SHATTER_FGM;
    mobj->texture_id_curr = ITREMIX_GEM_CRACKED_IMAGE;
    if (image_index_prev == ITREMIX_GEM_CRACKED_IMAGE)
    {
        itRemixGemDestroy(item_gobj);
        return TRUE;
    }
    itRemixGemApplyMultiplier(item_gobj, ITREMIX_GEM_LANDING_VEL_MUL);
    itRemixGemDisableBoxes(item_gobj);

    return FALSE;
}

/** Gem.collide_with_player_ (thrown hit, shield and setoff procs): cracks the gem, or shatters it, then bounces it back. */
static sb32 itRemixGemCollideWithPlayer(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    MObj *mobj = DObjGetStruct(item_gobj)->mobj;
    s32 image_index_prev = mobj->texture_id_curr;

    ip->attack_coll.fgm_id = ITREMIX_GEM_SHATTER_FGM;
    mobj->texture_id_curr = ITREMIX_GEM_CRACKED_IMAGE;
    if (image_index_prev == ITREMIX_GEM_CRACKED_IMAGE)
    {
        itRemixGemDestroy(item_gobj);
        return TRUE;
    }
    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    ip->item_vars.shell.health = (u8) image_index_prev; /* asm stores the old image index here (sb v0) */
    ip->physics.vel_air.x = ((ip->physics.vel_air.x * 1.0F) + (ITRSHELL_RECOIL_VEL_X * ip->hit_lr)) * ITREMIX_GEM_HIT_VEL_MUL_X;
    itRShellCommonClearAnim(item_gobj);
    itRemixGemBounceBack(item_gobj);

    return FALSE;
}

/** Gem.set_idle_aerial_: bounces the gem back after a hit, with a random horizontal speed. */
static void itRemixGemBounceBack(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    ip->item_vars.shell.health = (u8) syUtilsRandIntRange(ITREMIX_GEM_HEALTH_MAX);
    ip->physics.vel_air.y = ITREMIX_GEM_BOUNCE_VEL_Y;
    ip->physics.vel_air.x = syUtilsRandFloat() * (-ip->physics.vel_air.x * ITREMIX_GEM_BOUNCE_MUL_X);
    itMainClearOwnerStats(item_gobj);
    itGShellCommonClearAnim(item_gobj);
    itRemixGemSetFallStatus(item_gobj);
}

/** Gem.set_idle_aerial_2: disables the boxes, cracks the gem and switches to the fall status. */
static void itRemixGemSetFallStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    MObj *mobj = DObjGetStruct(item_gobj)->mobj;

    ip->damage_coll.hitstatus = nGMHitStatusNone;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    ip->pickup_wait &= ~ITREMIX_GEM_PICKUP_WAIT_MSB;
    itMapSetAir(ip);
    mobj->texture_id_curr = ITREMIX_GEM_CRACKED_IMAGE;
    itMainSetStatus(item_gobj, sITRemixGemStatusDescs, nITRemixGemStatusFall);
}

/** Gem.prepickup_ (PICKUP_ITEM_INIT): disables the hurtbox and switches to the picked-up status. */
static void itRemixGemPrepickup(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNone(item_gobj);
    itMainSetStatus(item_gobj, sITRemixGemStatusDescs, nITRemixGemStatusPickup);
}

/** Gem.throw_initial_ (THROW_ITEM): enables the hurtbox and switches to the thrown status. */
static void itRemixGemThrowInitial(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixGemStatusDescs, nITRemixGemStatusThrown);
}

/** Gem._get_item_info (owned-gem branch): sets damage, knockback, hit FGM and palette. */
static void itRemixGemApplyVariant(GObj *item_gobj, const ITRemixGemVariant *variant)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->attack_coll.damage = variant->damage;
    ip->attack_coll.knockback_base = variant->knockback_base;
    ip->attack_coll.knockback_scale = variant->knockback_scale;
    ip->attack_coll.fgm_id = ITREMIX_GEM_HIT_FGM;
    DObjGetStruct(item_gobj)->mobj->palette_id = variant->palette_index;
}

/**
 * Gem.gem_stage_setting_ (SPAWN_ITEM). The vanilla ABI is (parent, pos, vel, flags).
 * The asm's no-owner branch (blue gem) is not ported: the gem's item file is the
 * owner's special3 file, so without an owner there is nothing to bind.
 */
GObj* itRemixGemMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    ITDesc item_desc;
    FTStruct *fp;
    GObj *item_gobj;
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    if (parent_gobj == NULL)
    {
        return NULL;
    }
    fp = ftGetStruct(parent_gobj);
    item_desc = sITRemixGemDesc;
    item_desc.p_file = fp->data->p_file_special3;

    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    joint = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);
    translate = joint->translate.vec.f;

    ip->multi = 0;
    itMainClearOwnerStats(item_gobj);
    gcAddXObjForDObjFixed(joint, 0x2E, 0);
    joint->translate.vec.f = translate;
    joint->rotate.vec.f.z = 0.0F;

    itRemixGemApplyVariant(item_gobj, &sITRemixGemOwnedVariant);
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/** Descriptor of Gem (item id 0x42) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixGemItemDesc =
{
    ITREMIX_GEM_ID,                     /* Item.Gem.id */
    itRemixGemMakeItem,                 /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    itRemixGemPrepickup,                /* PICKUP_ITEM_INIT */
    itRemixConsumeTomatoTail,           /* PICKUP_ITEM_MAIN: pickup_gem jumps to 0x80145C4C */
    itTomatoDroppedSetStatus,           /* DROP_ITEM (asm 0x801745FC, same as Maxim Tomato) */
    itRemixGemThrowInitial,             /* THROW_ITEM */
    NULL                                /* PLAYER_COLLISION */
};

#endif
