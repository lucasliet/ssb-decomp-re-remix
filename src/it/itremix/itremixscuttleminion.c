/**
 * itremixscuttleminion.c - C port of the Smash Remix Scuttle Minion item (Scuttle Town).
 * Ported from Hazards.asm minion_stage_setting_, minion_main_, minion_destroy_ and minion_blast_zone_, and from
 * src/items/ScuttleMinion.asm item_info_array.
 *
 * The minion lives for 390 tics and is destroyed when a hurtbox hits it. Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixscuttleminion.h"
#include "gr/grremix/grremixscuttleminion.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "sys/objman.h"

#define ITREMIX_SCUTTLEMINION_FILE_OFFSET 0x40        /* asm item_info_array: offset to item footer */
#define ITREMIX_SCUTTLEMINION_LIFE_TIMER_MAX 0x186    /* asm minion_main_: destroyed after 390 tics */
#define ITREMIX_SCUTTLEMINION_DESTROY_FGM 0x3E8       /* asm FGM id 0x03E8 (Badnik Destroy) */
#define ITREMIX_SCUTTLEMINION_HITBOX_DAMAGE 5
#define ITREMIX_SCUTTLEMINION_HITBOX_SIZE 150.0F      /* asm 0x43160000 */
#define ITREMIX_SCUTTLEMINION_HITBOX_ANGLE 0x361      /* asm sakurai angle */
#define ITREMIX_SCUTTLEMINION_HITBOX_KNOCKBACK_SCALE 0x25 /* asm kbs at 0x0140 */
#define ITREMIX_SCUTTLEMINION_HITBOX_KNOCKBACK_BASE 0x50  /* asm bkb at 0x0148 */
#define ITREMIX_SCUTTLEMINION_HITBOX_FGM 0x26         /* asm hitbox sound (medium kick) */

extern void* func_800269C0_275C0(u16 id);

/* The asm keeps the life timer in the item GObj (0x40). The factory spawns one minion at a time, so one static copy is enough. */
static s32 sITRemixScuttleMinionLifeTimer;

static sb32 itRemixScuttleMinionMainProcUpdate(GObj *item_gobj);
static sb32 itRemixScuttleMinionDestroyProcHurtbox(GObj *item_gobj);
static void itRemixScuttleMinionSetupHitbox(ITStruct *ip);

/* asm item_info_array: word 0x1B000000 at 0x0C, file offset 0x40 at 0x08; state 0 main, hurtbox hit destroys. */
static ITDesc sITRemixScuttleMinionDesc =
{
    ITREMIX_SCUTTLEMINION_KIND,
    NULL,                                   /* p_file, set per spawn to the stage header pointer */
    ITREMIX_SCUTTLEMINION_FILE_OFFSET,
    { 0x1B, 0, 0 },
    nGMAttackStateOff,
    itRemixScuttleMinionMainProcUpdate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    itRemixScuttleMinionDestroyProcHurtbox
};

/**
 * minion_stage_setting_ (asm SPAWN_ITEM). Makes the minion, zeroes its life timer and sets its hitbox. Returns the
 * minion GObj, or NULL when the item could not be made.
 * The asm ignores `pos` and spawns at minion_coordinates (zero); the factory passes zero too. The asm passes 1 as the
 * item's unknown argument (used by the piranha plant) and the caller's `flags` are not used.
 */
GObj* itRemixScuttleMinionMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    ITDesc item_desc = sITRemixScuttleMinionDesc;
    GObj *item_gobj;
    DObj *joint;
    ITStruct *ip;
    Vec3f origin = { 0.0F, 0.0F, 0.0F };

    item_desc.p_file = grRemixScuttleMinionStageHeaderRef();
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, 1);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    sITRemixScuttleMinionLifeTimer = 0;
    joint = DObjGetStruct(item_gobj);
    joint->translate.vec.f = origin;
    ip = itGetStruct(item_gobj);
    itRemixScuttleMinionSetupHitbox(ip);
    return item_gobj;
}

/**
 * Enables the minion's hitbox and sets its damage, size, angle, knockback and sound (asm minion_stage_setting_).
 * REMIX-TODO(ITStruct): the asm also sets unnamed item fields that are not ported: 0x158 |= 0x10, 0x2D3 |= 0x08,
 * 0x1D0 = 0 (hitbox refresh), 0x1D4 = 0 (collision flag), 0x35C = 0. The sfx timer at 0x44 = 0x30 is not read by minion_main_.
 */
static void itRemixScuttleMinionSetupHitbox(ITStruct *ip)
{
    ITAttackColl *attack_coll = &ip->attack_coll;

    attack_coll->attack_state = 1;
    attack_coll->damage = ITREMIX_SCUTTLEMINION_HITBOX_DAMAGE;
    attack_coll->element = 0;
    attack_coll->size = ITREMIX_SCUTTLEMINION_HITBOX_SIZE;
    attack_coll->angle = ITREMIX_SCUTTLEMINION_HITBOX_ANGLE;
    attack_coll->knockback_scale = ITREMIX_SCUTTLEMINION_HITBOX_KNOCKBACK_SCALE;
    attack_coll->knockback_base = ITREMIX_SCUTTLEMINION_HITBOX_KNOCKBACK_BASE;
    attack_coll->fgm_id = ITREMIX_SCUTTLEMINION_HITBOX_FGM;
}

/**
 * minion_main_ (asm). Advances the life timer by one tic and destroys the minion when its life passes 390 tics.
 * Returns TRUE to destroy the minion.
 */
static sb32 itRemixScuttleMinionMainProcUpdate(GObj *item_gobj)
{
    s32 life_timer = sITRemixScuttleMinionLifeTimer;

    sITRemixScuttleMinionLifeTimer = life_timer + 1;
    return life_timer > ITREMIX_SCUTTLEMINION_LIFE_TIMER_MAX;
}

/**
 * minion_destroy_ (asm). Plays the badnik destroy sound and destroys the minion when a hurtbox hits it.
 * REMIX-TODO(blast zone): asm minion_blast_zone_ destroys the minion at the blast zone through the item's ITStruct
 * 0x398 routine table, which is not wired to Remix blast walls (same as the Robot Bee).
 */
static sb32 itRemixScuttleMinionDestroyProcHurtbox(GObj *item_gobj)
{
    func_800269C0_275C0(ITREMIX_SCUTTLEMINION_DESTROY_FGM);
    return TRUE;
}

/** Descriptor of Scuttle Minion (item id 0x3E) for the Remix item dispatch. SHOW_GFX_WHEN_SPAWNED and the pickup, drop, throw and touch routines are absent in the asm. */
const ITRemixItemDesc dITRemixScuttleMinionItemDesc =
{
    ITREMIX_SCUTTLEMINION_KIND,             /* Item.ScuttleMinion.id */
    itRemixScuttleMinionMakeItem,           /* SPAWN_ITEM */
    FALSE,                                  /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                                   /* PICKUP_ITEM_INIT */
    NULL,                                   /* PICKUP_ITEM_MAIN */
    NULL,                                   /* DROP_ITEM */
    NULL,                                   /* THROW_ITEM */
    NULL                                    /* PLAYER_COLLISION */
};

#endif
