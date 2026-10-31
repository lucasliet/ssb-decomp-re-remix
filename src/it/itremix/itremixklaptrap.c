/**
 * itremixklaptrap.c - C port of the Smash Remix KlapTrap item
 * (.toolchain/pkgs/smashremix-src/src/items/KlapTrap.asm and the Jungle Japes
 * routines klaptrap_main_, klaptrap_stage_setting_ and klaptrap_blast_zone_ in
 * src/Hazards.asm).
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixklaptrap.h"
#include "gr/grremix/grremixhazards.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "it/itground/itpakkun.h"
#include "sys/obj.h"
#include "sys/objanim.h"
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
#define ITREMIX_KLAPTRAP_ANIM_JOINT_OFFSET 0x0908 /* asm animation track 1, relative to the Japes file */
#define ITREMIX_KLAPTRAP_ANIM_MATERIAL_OFFSET 0x0E20 /* asm animation track 2 (materials) */
#define ITREMIX_KLAPTRAP_CAMERA_TAG_HIDDEN 0 /* asm GObj 0x38: no camera tag matches, nothing is drawn */
#define ITREMIX_KLAPTRAP_MULTI 0xB4 /* asm 0x033E halfword at spawn (Hazards.asm 1745), the pirhana flag */

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
static u32 sITRemixKlapTrapVisibleCameraTag;

static sb32 itRemixKlapTrapProcUpdate(GObj *item_gobj);
static void itRemixKlapTrapStartAnimation(GObj *item_gobj);
static void itRemixKlapTrapSetVisible(GObj *item_gobj, sb32 is_visible);
static ITRemixKlapTrapVars* itRemixKlapTrapGetVars(ITStruct *ip);
static void itRemixKlapTrapArmHitbox(GObj *item_gobj);
static sb32 itRemixKlapTrapShouldSpawn(s32 timer_before);
static void itRemixKlapTrapSetupHitbox(ITStruct *ip);
static void itRemixKlapTrapSetupVars(ITStruct *ip, Vec3f *pos);
static sb32 itRemixKlapTrapProcDead(GObj *item_gobj);

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
    itRemixLogEvent("klaptrap-spawn", ITREMIX_KLAPTRAP_ID);
    itRemixKlapTrapStartAnimation(item_gobj);
    return FALSE;
}

/** Restarts the joint and material animation tracks of the KlapTrap (asm 0x8000BD1C and 0x8000BD54). */
static void itRemixKlapTrapStartAnimation(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    u8 *file = (u8*) grRemixHazardsItemFileTop(sITRemixKlapTrapStageFile);

    gcAddDObjAnimJoint(dobj, (AObjEvent32*) (file + ITREMIX_KLAPTRAP_ANIM_JOINT_OFFSET), 0.0F);
    if (dobj->mobj != NULL)
    {
        gcAddMObjMatAnimJoint(dobj->mobj, (AObjEvent32*) (file + ITREMIX_KLAPTRAP_ANIM_MATERIAL_OFFSET), 0.0F);
    }
}

/** Shows or hides the item by the GObj camera tag (asm word 0x38 of the object). */
static void itRemixKlapTrapSetVisible(GObj *item_gobj, sb32 is_visible)
{
    item_gobj->camera_tag = (is_visible != FALSE) ? sITRemixKlapTrapVisibleCameraTag : ITREMIX_KLAPTRAP_CAMERA_TAG_HIDDEN;
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
    itRemixKlapTrapSetVisible(item_gobj, TRUE);
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
    /* REMIX-TODO(ITStruct 0x1D0, 0x1D4): asm clears them (Hazards.asm 1780-1781). They alias attack_pos[1].pos_prev.x/y (0x1C4 + 0xC); vanilla reads attack_pos[1] only when attack_count > 1, KlapTrap attack_count not verified. */
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
 * KlapTrap SPAWN_ITEM (asm klaptrap_stage_setting_). The item is created with no owner. The asm's
 * flags map to ITStruct fields: 0x33E = multi, 0x158 bit 0x10 = can_rehit_shield, 0x2D3 bit 0x08 =
 * is_allow_knockback, 0x398 = proc_dead.
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

    sITRemixKlapTrapVisibleCameraTag = item_gobj->camera_tag;
    itRemixKlapTrapSetVisible(item_gobj, FALSE);
    ip->multi = ITREMIX_KLAPTRAP_MULTI;
    ip->attack_coll.can_rehit_shield = TRUE;
    ip->is_allow_knockback = TRUE;
    ip->proc_dead = itRemixKlapTrapProcDead;
    itRemixKlapTrapSetupVars(ip, pos);
    itRemixKlapTrapSetupHitbox(ip);
    joint->translate.vec.f = *pos;

    return item_gobj;
}

/**
 * KlapTrap blast zone (ITStruct proc_dead, asm klaptrap_blast_zone_ Hazards.asm 1798). Disables the
 * hitbox, returns the model to the spawn x, hides it and returns FALSE. The asm's final jump to
 * 0x8016F8C0 lands after the proc_dead check of itProcessProcItemMain, so the item is not destroyed.
 * The asm writes the spawn x and y into item_vars (0x350/0x354); kv->spawn_pos already holds them.
 * The asm hides the item by clearing GObj 0x38 (camera_tag), done here by itRemixKlapTrapSetVisible.
 */
static sb32 itRemixKlapTrapProcDead(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixKlapTrapVars *kv = itRemixKlapTrapGetVars(ip);

    ip->attack_coll.attack_state = 0;
    DObjGetStruct(item_gobj)->translate.vec.f.x = kv->spawn_pos.x;
    itRemixKlapTrapSetVisible(item_gobj, FALSE);

    return FALSE;
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
