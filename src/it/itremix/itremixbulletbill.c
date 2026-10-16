/**
 * itremixbulletbill.c - C port of the Smash Remix Bullet Bill item (Banzai Bill), from the Banzai Bill routines of
 * .toolchain/pkgs/smashremix-src/src/Hazards.asm: banzai_bill_stage_setting_, banzai_bill_main_,
 * banzai_bill_destroy_ and banzai_bill_blast_zone_. Each routine names its Remix label in its doc comment.
 *
 * The bullet lives 390 tics and is destroyed when a hurtbox hits it. Only one bullet exists at a time.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixbulletbill.h"
#include "gr/grremix/grremixhazards.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "sys/obj.h"
#include "sys/objanim.h"

#define ITREMIX_BULLETBILL_FILE_OFFSET 0xE0             /* asm banzai_bill item footer, same offset as Robot Bee */
#define ITREMIX_BULLETBILL_LIFE_TIMER_MAX 0x186         /* asm banzai_bill_main_: destroyed when the timer is over 390 */
#define ITREMIX_BULLETBILL_DESTROY_FGM 0x3E8            /* asm banzai_bill_destroy_: fgm id 0x03E8 (Badnik Destroy) */
#define ITREMIX_BULLETBILL_HITBOX_DAMAGE 5              /* asm banzai_bill_stage_setting_: damage 5 */
#define ITREMIX_BULLETBILL_HITBOX_SIZE 150.0F           /* asm 0x43160000 */
#define ITREMIX_BULLETBILL_HITBOX_ANGLE 0x361           /* asm sakurai angle */
#define ITREMIX_BULLETBILL_HITBOX_KNOCKBACK_SCALE 0x25  /* asm kbs at 0x0140 */
#define ITREMIX_BULLETBILL_HITBOX_KNOCKBACK_BASE 0x50   /* asm bkb at 0x0148 */
#define ITREMIX_BULLETBILL_HITBOX_FGM 0x26              /* asm hitbox sound (medium kick) */
#define ITREMIX_BULLETBILL_ATTACK_STATE_FRESH 1         /* ITAttackColl.attack_state: fresh hitbox */
#define ITREMIX_BULLETBILL_ANIM_JOINT_OFFSET 0x0908     /* asm animation track 1, relative to the stage file top */
#define ITREMIX_BULLETBILL_ANIM_MATERIAL_OFFSET 0x09B4  /* asm animation track 2 (materials) */

extern void* func_800269C0_275C0(u16 id);

/* Bullet Bill is spawned one at a time by the factory, so one static copy of the life timer is enough. */
static void *sITRemixBulletBillStageHeader;
static s32 sITRemixBulletBillLifeTimer;

static sb32 itRemixBulletBillMainProcUpdate(GObj *item_gobj);
static sb32 itRemixBulletBillDestroyProcHurtbox(GObj *item_gobj);
static void itRemixBulletBillSetupHitbox(ITStruct *ip);

/* asm item info of the Banzai Bill spawn: p_file is the stage header, which the footer at 0xE0 is read from. */
static ITDesc sITRemixBulletBillDesc =
{
    ITREMIX_BULLETBILL_KIND,
    &sITRemixBulletBillStageHeader,
    ITREMIX_BULLETBILL_FILE_OFFSET,
    { 0x1B, 0, 0 },                         /* asm word 0x1B000000 of the item info array */
    nGMAttackStateOff,
    itRemixBulletBillMainProcUpdate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    itRemixBulletBillDestroyProcHurtbox
};

/**
 * banzai_bill_stage_setting_ (asm SPAWN_ITEM). Creates the bullet, arms its hitbox and starts its life timer at 0.
 * Returns the bullet GObj, or NULL when the item could not be made.
 * REMIX-TODO(ITStruct 0x33E, 0x158 bit 0x10, 0x2D3 bit 0x08, 0x1D0, 0x1D4, 0x35C): raw flags of the asm, fields not named yet.
 * REMIX-TODO(ITStruct 0x398 blast zone slot): asm stores banzai_bill_blast_zone_ there, not wired to the item yet.
 */
GObj* itRemixBulletBillMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj;

    item_gobj = itManagerMakeItem(parent_gobj, &sITRemixBulletBillDesc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    sITRemixBulletBillLifeTimer = 0;
    itRemixBulletBillSetupHitbox(itGetStruct(item_gobj));
    return item_gobj;
}

/**
 * Hitbox of the bullet (asm banzai_bill_stage_setting_): attack state 1, damage 5, size 150, angle 0x361,
 * knockback base 0x50 and scale 0x25, medium kick sound.
 */
static void itRemixBulletBillSetupHitbox(ITStruct *ip)
{
    ITAttackColl *attack_coll = &ip->attack_coll;

    attack_coll->attack_state = ITREMIX_BULLETBILL_ATTACK_STATE_FRESH;
    attack_coll->damage = ITREMIX_BULLETBILL_HITBOX_DAMAGE;
    attack_coll->element = 0;
    attack_coll->size = ITREMIX_BULLETBILL_HITBOX_SIZE;
    attack_coll->angle = ITREMIX_BULLETBILL_HITBOX_ANGLE;
    attack_coll->knockback_scale = ITREMIX_BULLETBILL_HITBOX_KNOCKBACK_SCALE;
    attack_coll->knockback_base = ITREMIX_BULLETBILL_HITBOX_KNOCKBACK_BASE;
    attack_coll->fgm_id = ITREMIX_BULLETBILL_HITBOX_FGM;
}

/**
 * banzai_bill_main_ (asm). Counts the life timer by one and destroys the bullet once the timer is over 390.
 * Returns TRUE to destroy the bullet. The asm's laser and buzz code is commented out there and is not ported.
 */
static sb32 itRemixBulletBillMainProcUpdate(GObj *item_gobj)
{
    s32 timer_before = sITRemixBulletBillLifeTimer;

    sITRemixBulletBillLifeTimer = timer_before + 1;
    return timer_before > ITREMIX_BULLETBILL_LIFE_TIMER_MAX;
}

/**
 * banzai_bill_destroy_ (asm). Plays the badnik destroy sound and destroys the bullet when a hurtbox hits it.
 */
static sb32 itRemixBulletBillDestroyProcHurtbox(GObj *item_gobj)
{
    func_800269C0_275C0(ITREMIX_BULLETBILL_DESTROY_FGM);
    return TRUE;
}

/**
 * Starts the joint and material animation tracks of the bullet from the stage file (asm 0x8000BD1C and 0x8000BD54).
 * Called by the factory after a spawn.
 */
void itRemixBulletBillStartAnimation(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    u8 *file = (u8*) grRemixHazardsItemFileTop(sITRemixBulletBillStageHeader);

    gcAddDObjAnimJoint(dobj, (AObjEvent32*) (file + ITREMIX_BULLETBILL_ANIM_JOINT_OFFSET), 0.0F);
    if (dobj->mobj != NULL)
    {
        gcAddMObjMatAnimJoint(dobj->mobj, (AObjEvent32*) (file + ITREMIX_BULLETBILL_ANIM_MATERIAL_OFFSET), 0.0F);
    }
}

/**
 * Binds the stage header the item footer is read from and clears the life timer. Called by the Peach's Castle II
 * setup before the factory runs.
 */
void itRemixBulletBillSetStageFile(void *stage_header)
{
    sITRemixBulletBillStageHeader = stage_header;
    sITRemixBulletBillLifeTimer = 0;
}

/** Descriptor of Bullet Bill (item id 0x3D) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixBulletBillItemDesc =
{
    ITREMIX_BULLETBILL_KIND,            /* Item.BulletBill.id */
    itRemixBulletBillMakeItem,          /* SPAWN_ITEM */
    FALSE,                              /* SHOW_GFX_WHEN_SPAWNED (OS.FALSE) */
    NULL,                               /* PICKUP_ITEM_INIT (0) */
    NULL,                               /* PICKUP_ITEM_MAIN (0) */
    NULL,                               /* DROP_ITEM (0) */
    NULL,                               /* THROW_ITEM (0) */
    NULL                                /* PLAYER_COLLISION (0) */
};

#endif
