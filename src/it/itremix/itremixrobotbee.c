/**
 * itremixrobotbee.c - C port of the Smash Remix Robot Bee item (.toolchain/pkgs/smashremix-src/src/items/RobotBee.asm).
 *
 * The bee fires its laser in the window 97..256 tics of its life, repeats the fire every 13 tics, plays its
 * buzz every 48 tics and is destroyed after 390 tics. Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixrobotbee.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "sys/objman.h"
#include "gr/grremix/grremixmetallic.h"
#include "gr/grremix/grremixmetallicbeelaser.h"

#define ITREMIX_ROBOTBEE_FILE_OFFSET 0xE0             /* asm FILE_OFFSET: item footer in the stage file */
#define ITREMIX_ROBOTBEE_FIRE_TIMER_MIN 0x60          /* asm robot_bee_main_: fire only when the timer is above 96 */
#define ITREMIX_ROBOTBEE_FIRE_TIMER_MAX 0x100         /* asm robot_bee_main_: ... and at most 256 */
#define ITREMIX_ROBOTBEE_LIFE_TIMER_MAX 0x186         /* asm robot_bee_main_: destroyed after 390 tics */
#define ITREMIX_ROBOTBEE_REPEAT_TIMER 0x0D            /* asm REPEAT_TIMER: the laser repeats every 13 tics */
#define ITREMIX_ROBOTBEE_SFX_TIMER_START 0x30         /* asm robot_bee_stage_setting_: starts with the buzz */
#define ITREMIX_ROBOTBEE_SFX_TIMER_PERIOD 0x30        /* asm robot_bee_main_: buzz every 48 tics */
#define ITREMIX_ROBOTBEE_SFX_FGM 0x3FB                /* asm FGM id 0x03FB (Shrinkray) */
#define ITREMIX_ROBOTBEE_DESTROY_FGM 0x3E8            /* asm FGM id 0x03E8 (Badnik Destroy) */
#define ITREMIX_ROBOTBEE_HITBOX_DAMAGE 5
#define ITREMIX_ROBOTBEE_HITBOX_SIZE 150.0F           /* asm 0x43160000 */
#define ITREMIX_ROBOTBEE_HITBOX_ANGLE 0x361           /* asm sakurai angle */
#define ITREMIX_ROBOTBEE_HITBOX_KNOCKBACK_SCALE 0x25  /* asm kbs at 0x0140 */
#define ITREMIX_ROBOTBEE_HITBOX_KNOCKBACK_BASE 0x50   /* asm bkb at 0x0148 */
#define ITREMIX_ROBOTBEE_HITBOX_FGM 0x26              /* asm hitbox sound (medium kick) */

extern void* func_800269C0_275C0(u16 id);

/* The asm keeps these in the item GObj (0x40, 0x44, 0x48) and the factory spawns one bee at a time, so one static copy is enough. */
typedef struct ITRemixRobotBeeVars
{
    s32 laser_timer;
    s32 sfx_timer;
    s32 repeat_timer;
} ITRemixRobotBeeVars;

static ITRemixRobotBeeVars sITRemixRobotBeeVars;

static sb32 itRemixRobotBeeMainProcUpdate(GObj *item_gobj);
static sb32 itRemixRobotBeeDestroyProcHurtbox(GObj *item_gobj);
static void itRemixRobotBeeFireWindow(GObj *item_gobj);
static void itRemixRobotBeeSfxTick(void);
static void itRemixRobotBeeResetVars(void);
static void itRemixRobotBeeSetupHitbox(ITStruct *ip);

/* asm item_info_array and item_state_table: state 0 main, state 1 main, state 2 explosion; the hurtbox hit destroys. */
static ITDesc sITRemixRobotBeeDesc =
{
    ITREMIX_ROBOTBEE_KIND,
    NULL,                                   /* p_file, set per spawn to the stage header pointer */
    ITREMIX_ROBOTBEE_FILE_OFFSET,
    { 0x1B, 0, 0 },                         /* asm word 0x1B000000 of the item info array */
    nGMAttackStateOff,
    itRemixRobotBeeMainProcUpdate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    itRemixRobotBeeDestroyProcHurtbox
};

/**
 * robot_bee_stage_setting_ (asm SPAWN_ITEM). Makes the bee at the spawn position and sets its laser timers and
 * its hitbox. Returns the bee GObj, or NULL when the item could not be made.
 */
GObj* itRemixRobotBeeMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    ITDesc item_desc = sITRemixRobotBeeDesc;
    GObj *item_gobj;
    DObj *joint;
    ITStruct *ip;
    Vec3f origin = { 0.0F, 0.0F, 0.0F };

    item_desc.p_file = grRemixMetallicStageHeaderRef();
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixRobotBeeResetVars();
    joint = DObjGetStruct(item_gobj);
    joint->translate.vec.f = origin;
    ip = itGetStruct(item_gobj);
    itRemixRobotBeeSetupHitbox(ip);
    return item_gobj;
}

/** Restores the timers the asm sets at spawn: laser timer 0, buzz timer 48 (so the buzz starts at once), repeat 13. */
static void itRemixRobotBeeResetVars(void)
{
    sITRemixRobotBeeVars.laser_timer = 0;
    sITRemixRobotBeeVars.sfx_timer = ITREMIX_ROBOTBEE_SFX_TIMER_START;
    sITRemixRobotBeeVars.repeat_timer = ITREMIX_ROBOTBEE_REPEAT_TIMER;
}

/**
 * Enables the bee's hitbox and sets its damage, size, angle, knockback and sound (asm robot_bee_stage_setting_).
 * REMIX-TODO(ITStruct): the asm also sets unnamed item fields that are not ported: 0x158 |= 0x10, 0x33E = 0xB4,
 * 0x2D3 |= 0x08, 0x1D0 = 0 (hitbox refresh), 0x1D4 = 0 (collision flag), 0x35C = 0.
 */
static void itRemixRobotBeeSetupHitbox(ITStruct *ip)
{
    ITAttackColl *attack_coll = &ip->attack_coll;

    attack_coll->attack_state = 1;
    attack_coll->damage = ITREMIX_ROBOTBEE_HITBOX_DAMAGE;
    attack_coll->element = 0;
    attack_coll->size = ITREMIX_ROBOTBEE_HITBOX_SIZE;
    attack_coll->angle = ITREMIX_ROBOTBEE_HITBOX_ANGLE;
    attack_coll->knockback_scale = ITREMIX_ROBOTBEE_HITBOX_KNOCKBACK_SCALE;
    attack_coll->knockback_base = ITREMIX_ROBOTBEE_HITBOX_KNOCKBACK_BASE;
    attack_coll->fgm_id = ITREMIX_ROBOTBEE_HITBOX_FGM;
}

/**
 * robot_bee_main_ (asm). Fires in the window of 97..256 tics, advances the laser timer by one and destroys the bee
 * when its life passes 390 tics. Returns TRUE to destroy the bee.
 */
static sb32 itRemixRobotBeeMainProcUpdate(GObj *item_gobj)
{
    s32 laser_timer = sITRemixRobotBeeVars.laser_timer;

    if ((laser_timer > ITREMIX_ROBOTBEE_FIRE_TIMER_MIN) && (laser_timer <= ITREMIX_ROBOTBEE_FIRE_TIMER_MAX))
    {
        itRemixRobotBeeFireWindow(item_gobj);
    }
    sITRemixRobotBeeVars.laser_timer = laser_timer + 1;
    return laser_timer > ITREMIX_ROBOTBEE_LIFE_TIMER_MAX;
}

/** Fires the laser every 13 tics of the window, then ticks the buzz (asm robot_bee_main_, repeat and buzz blocks). */
static void itRemixRobotBeeFireWindow(GObj *item_gobj)
{
    if (sITRemixRobotBeeVars.repeat_timer == ITREMIX_ROBOTBEE_REPEAT_TIMER)
    {
        sITRemixRobotBeeVars.repeat_timer = 0;
        grRemixMetallicBeeLaserFire(item_gobj);
    }
    else
    {
        sITRemixRobotBeeVars.repeat_timer++;
    }
    itRemixRobotBeeSfxTick();
}

/** Plays the buzz every 48 tics; the buzz timer counts up between plays. */
static void itRemixRobotBeeSfxTick(void)
{
    if (sITRemixRobotBeeVars.sfx_timer != ITREMIX_ROBOTBEE_SFX_TIMER_PERIOD)
    {
        sITRemixRobotBeeVars.sfx_timer++;
        return;
    }
    sITRemixRobotBeeVars.sfx_timer = 0;
    func_800269C0_275C0(ITREMIX_ROBOTBEE_SFX_FGM);
}

/**
 * robot_bee_destroy_ (asm). Plays the badnik destroy sound and destroys the bee when a hurtbox hits it.
 * REMIX-TODO(blast zone): asm robot_bee_blast_zone_ destroys the bee at the blast zone through the item's
 * ITStruct 0x398 routine table, which is not wired to Remix blast walls.
 */
static sb32 itRemixRobotBeeDestroyProcHurtbox(GObj *item_gobj)
{
    func_800269C0_275C0(ITREMIX_ROBOTBEE_DESTROY_FGM);
    return TRUE;
}

/** Descriptor of Robot Bee (item id 0x3B) for the Remix item dispatch. SHOW_GFX_WHEN_SPAWNED and the pickup, drop, throw and touch routines are all absent in the asm. */
const ITRemixItemDesc dITRemixRobotBeeItemDesc =
{
    ITREMIX_ROBOTBEE_KIND,                  /* Item.RobotBee.id */
    itRemixRobotBeeMakeItem,                /* SPAWN_ITEM */
    FALSE,                                  /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                                   /* PICKUP_ITEM_INIT */
    NULL,                                   /* PICKUP_ITEM_MAIN */
    NULL,                                   /* DROP_ITEM */
    NULL,                                   /* THROW_ITEM */
    NULL                                    /* PLAYER_COLLISION */
};

#endif
