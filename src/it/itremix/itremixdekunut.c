/**
 * itremixdekunut.c - C port of the Smash Remix Deku Nut item
 * (.toolchain/pkgs/smashremix-src/src/items/DekuNut.asm).
 *
 * The nut uses the Bob-omb throw and map callbacks and the Maxim Tomato fall
 * routines. Its explosion is four hurtbox steps (hurtbox_struct_) driven by
 * the item's multi timer, then a six-frame flash that destroys the item.
 * Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixdekunut.h"
#include "it/itremix/itremixscreenflash.h"
#include "it/itmain.h"
#include "it/itmap.h"
#include "it/itmanager.h"
#include "it/itvisuals.h"
#include "it/itcommon/itbombhei.h"
#include "it/itcommon/ittomato.h"
#include "if/ifcommon.h"
#include "ef/efmanager.h"
#include "sys/objman.h"

#define ITREMIX_DEKUNUT_ID 0x32
#define ITREMIX_DEKUNUT_FILE_OFFSET 0x0ED0 /* asm FILE_OFFSET: item footer in the common item file 0xFB */
#define ITREMIX_DEKUNUT_HURTBOX_FGM 0x0408 /* asm hurtbox FGM, set on spawn and on explosion */
#define ITREMIX_DEKUNUT_EXPLODE_FGM 0x0407 /* asm explode_transition_ FGM */
#define ITREMIX_DEKUNUT_KNOCKBACK_SCALE 0x0100 /* asm writes 0x100 as a halfword at attack_coll.knockback_scale (0x140) */
#define ITREMIX_DEKUNUT_ELEMENT_DEKU_STUN 0x0A /* Damage.asm index 0xA, DEKU_STUN */
#define ITREMIX_DEKUNUT_ANGLE 269 /* decoded from hurtbox_struct_ word 0x00434040, bits 31..22 (signed) */
#define ITREMIX_DEKUNUT_DAMAGE 1 /* decoded from hurtbox_struct_ word 0x00434040, bits 13..6 */
#define ITREMIX_DEKUNUT_FLASH_STEPS 4 /* hurtbox_struct_ entries; event_id stays on the last one */
#define ITREMIX_DEKUNUT_FLASH_LIFETIME 6 /* multi value at which the flash status destroys the item */
#define ITREMIX_DEKUNUT_THROW_GRAVITY 1.7546875F /* 0x3FE0999A */
#define ITREMIX_DEKUNUT_THROW_TERMINAL_VEL 100.0F /* 0x42C80000 */
#define ITREMIX_DEKUNUT_SHAKE_SEVERITY 1 /* asm screen shake severity = moderate */

enum itRemixDekuNutStatus
{
    nITRemixDekuNutStatusWait,
    nITRemixDekuNutStatusFall,
    nITRemixDekuNutStatusPickup,
    nITRemixDekuNutStatusThrown,
    nITRemixDekuNutStatusUnused4,
    nITRemixDekuNutStatusUnused5,
    nITRemixDekuNutStatusCollideWall,
    nITRemixDekuNutStatusFlash,
    nITRemixDekuNutStatusEnumCount
};

/* One row of hurtbox_struct_: the multi value that applies the step, and the hitbox size. */
typedef struct ITRemixDekuNutFlashStep
{
    u16 timer;
    u16 size;
} ITRemixDekuNutFlashStep;

extern void* func_800269C0_275C0(u16 id);

static void itRemixDekuNutThrowInitial(GObj *item_gobj);
static void itRemixDekuNutPrepickup(GObj *item_gobj);
static void itRemixDekuNutSetup(GObj *item_gobj);
static void itRemixDekuNutExplodeTransition(GObj *item_gobj);
static void itRemixDekuNutExplode(GObj *item_gobj);
static void itRemixDekuNutSetDamageAll(GObj *item_gobj);
static void itRemixDekuNutInitExplode(GObj *item_gobj);
static void itRemixDekuNutSetFlash(GObj *item_gobj);
static void itRemixDekuNutFlashSetStatus(GObj *item_gobj);
static sb32 itRemixDekuNutFlashProcUpdate(GObj *item_gobj);
static sb32 itRemixDekuNutCollidePlayer(GObj *item_gobj);
static sb32 itRemixDekuNutCollideWall(GObj *item_gobj);
static void itRemixDekuNutCollideTransition(GObj *item_gobj);
static sb32 itRemixDekuNutThrownProcUpdate(GObj *item_gobj);
static sb32 itRemixDekuNutThrownProcMap(GObj *item_gobj);

/* asm hurtbox_struct_ */
static const ITRemixDekuNutFlashStep sITRemixDekuNutFlashSteps[ITREMIX_DEKUNUT_FLASH_STEPS] =
{
    { 0x00, 382 },
    { 0x02, 250 },
    { 0x04, 150 },
    { 0x06, 0 }
};

/* asm item_state_table. Statuses 4 and 5 are empty in the asm. */
static ITStatusDesc sITRemixDekuNutStatusDescs[nITRemixDekuNutStatusEnumCount] =
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
        itRemixDekuNutThrownProcUpdate,
        itRemixDekuNutThrownProcMap,
        itRemixDekuNutCollidePlayer,
        itRemixDekuNutCollidePlayer,
        itMainCommonProcHop,
        itRemixDekuNutCollidePlayer,
        itMainCommonProcReflector,
        itRemixDekuNutCollidePlayer
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
        itRemixDekuNutCollideWall,
        NULL,
        itRemixDekuNutCollideWall,
        itRemixDekuNutCollideWall,
        NULL,
        NULL,
        itRemixDekuNutCollideWall,
        itRemixDekuNutCollideWall
    },
    {
        itRemixDekuNutFlashProcUpdate,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/* asm item_info_array. p_file is the common item file, so it is set once here. */
static ITDesc sITRemixDekuNutItemInfo =
{
    ITREMIX_DEKUNUT_ID,
    &gITManagerCommonData,
    ITREMIX_DEKUNUT_FILE_OFFSET,

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

/** DekuNut.spawn_deku_nut_ setup after itManagerMakeItem: translate, hitbox defaults, arrow. */
static void itRemixDekuNutSetup(GObj *item_gobj)
{
    DObj *joint = DObjGetStruct(item_gobj);
    ITStruct *ip = itGetStruct(item_gobj);
    Vec3f translate = joint->translate.vec.f;

    ip->multi = 0;
    gcAddXObjForDObjFixed(joint, 0x2E, 0);
    joint->translate.vec.f = translate;
    joint->rotate.vec.f.z = 0.0F;
    ip->attack_coll.fgm_id = ITREMIX_DEKUNUT_HURTBOX_FGM;
    ip->attack_coll.knockback_scale = ITREMIX_DEKUNUT_KNOCKBACK_SCALE;
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);
}

/**
 * DekuNut.spawn_deku_nut_ (SPAWN_ITEM). The vanilla ABI is (parent, pos, vel, flags).
 */
GObj* itRemixDekuNutMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixDekuNutItemInfo, pos, vel, flags);

    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixDekuNutSetup(item_gobj);

    return item_gobj;
}

/** DekuNut.prepickup_ (PICKUP_ITEM_INIT): disables the hitbox and switches to the picked-up status. */
static void itRemixDekuNutPrepickup(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNone(item_gobj);
    itMainSetStatus(item_gobj, sITRemixDekuNutStatusDescs, nITRemixDekuNutStatusPickup);
}

/** DekuNut.throw_initial_ (DROP_ITEM and THROW_ITEM): normal hitbox, deku stun element, thrown status. */
static void itRemixDekuNutThrowInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itBombHeiCommonSetHitStatusNormal(item_gobj);
    ip->attack_coll.element = ITREMIX_DEKUNUT_ELEMENT_DEKU_STUN;
    ip->attack_coll.knockback_scale = ITREMIX_DEKUNUT_KNOCKBACK_SCALE;
    itMainSetStatus(item_gobj, sITRemixDekuNutStatusDescs, nITRemixDekuNutStatusThrown);
}

/** DekuNut.throw_physics_ (thrown proc_update): gravity, then the spin update. */
static sb32 itRemixDekuNutThrownProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itMainApplyGravityClampTVel(ip, ITREMIX_DEKUNUT_THROW_GRAVITY, ITREMIX_DEKUNUT_THROW_TERMINAL_VEL);
    itVisualsUpdateSpin(item_gobj);

    return FALSE;
}

/** DekuNut.throw_duration_ (thrown proc_map): map check that moves to the wall status on contact. */
static sb32 itRemixDekuNutThrownProcMap(GObj *item_gobj)
{
    return itMapCheckMapProcAll(item_gobj, itRemixDekuNutCollideTransition);
}

/** DekuNut.collide_transition: normal hitbox, then the collide-with-wall status. */
static void itRemixDekuNutCollideTransition(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixDekuNutStatusDescs, nITRemixDekuNutStatusCollideWall);
}

/** DekuNut.collide_transition_player_ (thrown hit, shield, setoff and damage procs): explodes. */
static sb32 itRemixDekuNutCollidePlayer(GObj *item_gobj)
{
    itRemixDekuNutExplodeTransition(item_gobj);

    return FALSE;
}

/** DekuNut.collide_with_wall_ (collide-with-wall procs): dust effect, then explodes. */
static sb32 itRemixDekuNutCollideWall(GObj *item_gobj)
{
    itBombHeiCommonCheckMakeDustEffect(item_gobj, FALSE);
    itRemixDekuNutExplodeTransition(item_gobj);

    return FALSE;
}

/** DekuNut.explode_transition_: zeroes the velocity, explodes and plays the explosion FGM. */
static void itRemixDekuNutExplodeTransition(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    itRemixDekuNutExplode(item_gobj);
    func_800269C0_275C0(ITREMIX_DEKUNUT_EXPLODE_FGM);
}

/** DekuNut.explode_nut_: disables the hitbox, shakes the screen and starts the flash status. */
static void itRemixDekuNutExplode(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itBombHeiCommonSetHitStatusNone(item_gobj);
    /* DekuNut.asm _check_flash: the screen flash is drawn before the shake. */
    itRemixScreenFlashDekuNut();
    efManagerQuakeMakeEffect(ITREMIX_DEKUNUT_SHAKE_SEVERITY);
    /* REMIX-TODO(unverified): asm stores 2 at DObj 0x54 here (hurtbox sound or state byte, not identified). Skipped. */
    ip->attack_coll.fgm_id = ITREMIX_DEKUNUT_HURTBOX_FGM;
    itRemixDekuNutSetDamageAll(item_gobj);
    itRemixDekuNutFlashSetStatus(item_gobj);
}

/** DekuNut.explode_subroutine_: teamless, and able to hit the owner (the asm's owner clear is removed). */
static void itRemixDekuNutSetDamageAll(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->team = ITEM_TEAM_DEFAULT;
    ip->is_damage_all = TRUE;
}

/** DekuNut.explode_initial_2: resets the timer and the hurtbox step, then applies the first step. */
static void itRemixDekuNutInitExplode(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->multi = 0;
    ip->attack_coll.throw_mul = 1.0F;
    ip->event_id = 0;
    itRemixDekuNutSetFlash(item_gobj);
}

/** DekuNut.explode_initial_: starts the flash status (status 7). */
static void itRemixDekuNutFlashSetStatus(GObj *item_gobj)
{
    itRemixDekuNutInitExplode(item_gobj);
    itMainSetStatus(item_gobj, sITRemixDekuNutStatusDescs, nITRemixDekuNutStatusFlash);
}

/**
 * DekuNut.set_nut_flash_: applies the current hurtbox step once the multi timer
 * reaches its value, then advances to the next step (staying on the last one).
 */
static void itRemixDekuNutSetFlash(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    const ITRemixDekuNutFlashStep *step = &sITRemixDekuNutFlashSteps[ip->event_id];

    if (ip->multi != step->timer)
    {
        return;
    }
    ip->attack_coll.angle = ITREMIX_DEKUNUT_ANGLE;
    ip->attack_coll.damage = ITREMIX_DEKUNUT_DAMAGE;
    ip->attack_coll.size = (f32) step->size;
    ip->attack_coll.element = ITREMIX_DEKUNUT_ELEMENT_DEKU_STUN;
    ip->attack_coll.can_rehit_item = TRUE;
    ip->attack_coll.can_hop = FALSE;
    ip->attack_coll.can_reflect = FALSE;
    ip->attack_coll.can_setoff = FALSE;
    ip->event_id++;
    if (ip->event_id == ITREMIX_DEKUNUT_FLASH_STEPS)
    {
        ip->event_id = ITREMIX_DEKUNUT_FLASH_STEPS - 1;
    }
}

/** DekuNut.nut_active_ (flash proc_update): applies the flash step, then destroys the item at its last frame. */
static sb32 itRemixDekuNutFlashProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixDekuNutSetFlash(item_gobj);
    ip->multi++;

    return ip->multi == ITREMIX_DEKUNUT_FLASH_LIFETIME;
}

/** Descriptor of Deku Nut (item id 0x32) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixDekuNutItemDesc =
{
    ITREMIX_DEKUNUT_ID,                 /* Item.DekuNut.id */
    itRemixDekuNutMakeItem,             /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    itRemixDekuNutPrepickup,            /* PICKUP_ITEM_INIT */
    itRemixConsumeTomatoTail,           /* PICKUP_ITEM_MAIN: pickup_deku_nut jumps to 0x80145C4C */
    itRemixDekuNutThrowInitial,         /* DROP_ITEM */
    itRemixDekuNutThrowInitial,         /* THROW_ITEM */
    NULL                                /* PLAYER_COLLISION */
};

#endif
