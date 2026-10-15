/**
 * itremixklaptrap.c - C port of the Smash Remix KlapTrap item
 * (.toolchain/pkgs/smashremix-src/src/items/KlapTrap.asm and the Jungle Japes
 * routines klaptrap_main_, klaptrap_stage_setting_ and klaptrap_blast_zone_ in
 * src/Hazards.asm).
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixklaptrap.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "it/itground/itpakkun.h"
#include "sys/obj.h"
#include "sys/utils.h"

#define ITREMIX_KLAPTRAP_FILE_OFFSET 0xE0 /* asm FILE_OFFSET: item footer in the Japes stage file */
#define ITREMIX_KLAPTRAP_RESPAWN_WAIT 484 /* asm slti 0x1E4: frames before the first roll */
#define ITREMIX_KLAPTRAP_RESPAWN_FORCED 600 /* asm 0x258: timer value that forces a spawn */
#define ITREMIX_KLAPTRAP_RESPAWN_ROLLS 160 /* asm Global.get_random_int_(0xA0) */
#define ITREMIX_KLAPTRAP_RESPAWN_ROLL_HIT 50 /* asm 0x50: roll that spawns the KlapTrap */
#define ITREMIX_KLAPTRAP_HITBOX_DAMAGE 20 /* asm 0x14 */
#define ITREMIX_KLAPTRAP_HITBOX_ANGLE 0x270 /* asm spike angle */
#define ITREMIX_KLAPTRAP_HITBOX_ELEMENT 0x3 /* asm slash effect id */
#define ITREMIX_KLAPTRAP_HITBOX_FGM 0x105 /* asm heavy slash sound */
#define ITREMIX_KLAPTRAP_HITBOX_KNOCKBACK 0x100 /* asm bkb and kbs */
#define ITREMIX_KLAPTRAP_ATTACK_STATE_FRESH 1 /* ITAttackColl.attack_state: fresh hitbox */

typedef struct ITRemixKlapTrapVars
{
    Vec3f spawn_pos;        /* asm item_vars 0x350..0x358, the coordinates the item returns to */
    s32 unk_0x35C;          /* asm 0x35C, zeroed at spawn, never read by the routines of this item */
} ITRemixKlapTrapVars;

typedef struct ITRemixKlapTrapRespawnState
{
    s32 timer;              /* asm 0x801313F0+0x08, frames since the last spawn */
    sb32 is_hitbox_armed;   /* asm 0x801313F0+0x10, set on spawn, consumed on the next frame */
} ITRemixKlapTrapRespawnState;

_Static_assert(sizeof(ITRemixKlapTrapVars) <= sizeof(((ITStruct*)0)->item_vars),
    "ITRemixKlapTrapVars must fit in ITStruct.item_vars");

static void *sITRemixKlapTrapStageFile;
static ITRemixKlapTrapRespawnState sITRemixKlapTrapRespawn;

static sb32 itRemixKlapTrapProcUpdate(GObj *item_gobj);
static ITRemixKlapTrapVars* itRemixKlapTrapGetVars(ITStruct *ip);
static void itRemixKlapTrapArmHitbox(GObj *item_gobj);
static sb32 itRemixKlapTrapShouldSpawn(s32 timer_before);
static void itRemixKlapTrapSetupHitbox(ITStruct *ip);
static void itRemixKlapTrapSetupVars(ITStruct *ip, Vec3f *pos);

/* asm item_info_array. p_file points at the Japes stage file, bound in itRemixKlapTrapSetStageFile. */
static ITDesc sITRemixKlapTrapDesc =
{
    ITREMIX_KLAPTRAP_ID,
    &sITRemixKlapTrapStageFile,
    ITREMIX_KLAPTRAP_FILE_OFFSET,
    {
        nGCMatrixKindTra,                   /* item_info_array 0x0C = 0x12300000: Tra, secondary 0x30 */
        0x30,
        0
    },
    nGMAttackStateOff,
    itRemixKlapTrapProcUpdate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/**
 * KlapTrap main (asm klaptrap_main_): re-arms the hitbox after a spawn, then counts
 * frames and rolls for a respawn once the wait has passed.
 * REMIX-TODO(vanilla 0x8000BD1C, 0x8000BD54): the two animation track setups on spawn are not ported.
 */
static sb32 itRemixKlapTrapProcUpdate(GObj *item_gobj)
{
    s32 timer_before = sITRemixKlapTrapRespawn.timer;

    itRemixKlapTrapArmHitbox(item_gobj);
    sITRemixKlapTrapRespawn.timer++;

    if (timer_before < ITREMIX_KLAPTRAP_RESPAWN_WAIT)
    {
        return FALSE;
    }
    if (!itRemixKlapTrapShouldSpawn(timer_before))
    {
        return FALSE;
    }

    sITRemixKlapTrapRespawn.timer = 0;
    sITRemixKlapTrapRespawn.is_hitbox_armed = TRUE;
    return FALSE;
}

/** Roll of the respawn: a hit on the 1-in-160 roll, or the forced spawn once the timer reaches 600. */
static sb32 itRemixKlapTrapShouldSpawn(s32 timer_before)
{
    s32 roll = syUtilsRandIntRange(ITREMIX_KLAPTRAP_RESPAWN_ROLLS);

    if (roll == ITREMIX_KLAPTRAP_RESPAWN_ROLL_HIT)
    {
        return TRUE;
    }
    return timer_before == ITREMIX_KLAPTRAP_RESPAWN_FORCED;
}

/** Enables the hitbox on the frame after a spawn (asm klaptrap_main_, _timer_check path). */
static void itRemixKlapTrapArmHitbox(GObj *item_gobj)
{
    ITStruct *ip;

    if (!sITRemixKlapTrapRespawn.is_hitbox_armed)
    {
        return;
    }
    sITRemixKlapTrapRespawn.is_hitbox_armed = FALSE;
    ip = itGetStruct(item_gobj);
    ip->attack_coll.attack_state = ITREMIX_KLAPTRAP_ATTACK_STATE_FRESH;
    /* REMIX-TODO(GObj 0x38 visibility): asm 'sw at, 0x0038(s0)' makes the item visible again */
}

/** Returns the KlapTrap vars stored in the item's special union (asm item_vars at 0x350). */
static ITRemixKlapTrapVars* itRemixKlapTrapGetVars(ITStruct *ip)
{
    return (ITRemixKlapTrapVars*) &ip->item_vars;
}

/**
 * Hitbox and hurtbox setup at spawn (asm klaptrap_stage_setting_). The hitbox starts
 * disarmed; the hurtbox is removed so the item cannot be hit by fighters.
 */
static void itRemixKlapTrapSetupHitbox(ITStruct *ip)
{
    ip->attack_coll.attack_state = 0;
    ip->attack_coll.damage = ITREMIX_KLAPTRAP_HITBOX_DAMAGE;
    ip->attack_coll.angle = ITREMIX_KLAPTRAP_HITBOX_ANGLE;
    ip->attack_coll.element = ITREMIX_KLAPTRAP_HITBOX_ELEMENT;
    ip->attack_coll.fgm_id = ITREMIX_KLAPTRAP_HITBOX_FGM;
    ip->attack_coll.knockback_base = ITREMIX_KLAPTRAP_HITBOX_KNOCKBACK;
    ip->attack_coll.knockback_scale = ITREMIX_KLAPTRAP_HITBOX_KNOCKBACK;
    ip->damage_coll.hitstatus = 0;
    /* REMIX-TODO(ITStruct 0x1D0 hitbox refresh timer, 0x1D4 collision flag): asm clears them, fields not named yet */
}

/** Stores the spawn position and clears the spawn-time state of the item vars. */
static void itRemixKlapTrapSetupVars(ITStruct *ip, Vec3f *pos)
{
    ITRemixKlapTrapVars *kv = itRemixKlapTrapGetVars(ip);

    kv->spawn_pos = *pos;
    kv->unk_0x35C = 0;
}

/**
 * Sets the Japes stage file used as the item file and clears the respawn timer
 * (asm jungle_japes_setup). The file is the stage header that holds the item footer at 0xE0.
 */
void itRemixKlapTrapSetStageFile(void *stage_file)
{
    sITRemixKlapTrapStageFile = stage_file;
    sITRemixKlapTrapRespawn.timer = 0;
    sITRemixKlapTrapRespawn.is_hitbox_armed = FALSE;
}

/**
 * KlapTrap SPAWN_ITEM (asm klaptrap_stage_setting_). The item is created with no owner;
 * the asm's stage-only flags (0x33E, 0x158, 0x2D3, 0x398) are REMIX-TODO until their ITStruct fields are named.
 */
GObj* itRemixKlapTrapMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj;
    ITStruct *ip;
    DObj *joint;

    item_gobj = itManagerMakeItem(parent_gobj, &sITRemixKlapTrapDesc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    ip = itGetStruct(item_gobj);
    joint = DObjGetStruct(item_gobj);

    /* REMIX-TODO(GObj 0x38 hidden at spawn): asm 'sw r0, 0x0038(v0)' */
    /* REMIX-TODO(ITStruct 0x33E, 0x158 bit 0x10, 0x2D3 bit 0x08): raw flags of the Remix item, fields not named yet */
    /* REMIX-TODO(ITStruct 0x398 blast zone slot): asm stores itRemixKlapTrapBlastZone here, not wired */
    itRemixKlapTrapSetupVars(ip, pos);
    itRemixKlapTrapSetupHitbox(ip);
    joint->translate.vec.f = *pos;

    return item_gobj;
}

/**
 * KlapTrap blast zone (asm klaptrap_blast_zone_): disables the hitbox, returns the item to
 * its spawn x position and sends it to the bomb/grenade destroy path.
 * REMIX-TODO(vanilla 0x8016F8C0): the final jump to the bomb/grenade destroy routine and
 * the GObj 0x38 hide are not ported, so the callback is not registered yet.
 */
void itRemixKlapTrapBlastZone(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixKlapTrapVars *kv = itRemixKlapTrapGetVars(ip);

    ip->attack_coll.attack_state = 0;
    DObjGetStruct(item_gobj)->translate.vec.f.x = kv->spawn_pos.x;
}

/** Descriptor of KlapTrap (item id 0x3A) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixKlapTrapItemDesc =
{
    ITREMIX_KLAPTRAP_ID,                /* Item.KlapTrap.id */
    itRemixKlapTrapMakeItem,            /* SPAWN_ITEM */
    FALSE,                              /* SHOW_GFX_WHEN_SPAWNED (OS.FALSE) */
    NULL,                               /* PICKUP_ITEM_INIT (0) */
    NULL,                               /* PICKUP_ITEM_MAIN (0) */
    NULL,                               /* DROP_ITEM (0) */
    NULL,                               /* THROW_ITEM (0) */
    NULL                                /* PLAYER_COLLISION (0) */
};

#endif
