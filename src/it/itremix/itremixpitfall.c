/**
 * itremixpitfall.c - C port of the Smash Remix Pitfall item
 * (.toolchain/pkgs/smashremix-src/src/items/Pitfall.asm).
 *
 * Pitfall is thrown like a Bob-omb, sticks to the ground when it lands on a
 * floor line, and buries itself. The owner loses the pitfall after a timer
 * runs out. Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixpitfall.h"
#include "it/itmain.h"
#include "it/itmap.h"
#include "it/itmanager.h"
#include "it/itcommon/itbombhei.h"
#include "it/itcommon/ittomato.h"
#include "it/itcommon/itmsbomb.h"
#include "it/itfighter/itlinkbomb.h"
#include "if/ifcommon.h"
#include "mp/mpcollision.h"
#include "sys/objman.h"

#define ITREMIX_PITFALL_ID 0x34
#define ITREMIX_PITFALL_FILE_OFFSET 0xF70 /* asm FILE_OFFSET: item footer in the common item file 0xFB */
#define ITREMIX_PITFALL_DAMAGE_BURY 0x0B /* Damage.id.BURY, as stored in attack_coll.element */
#define ITREMIX_PITFALL_DAMAGE 14
#define ITREMIX_PITFALL_KB_ANGLE 270
#define ITREMIX_PITFALL_BASE_KB 0x100
#define ITREMIX_PITFALL_AERIAL_FGM 0x25 /* asm AERIAL_HURTBOX_FGM, also GROUNDED_HURTBOX_FGM */
#define ITREMIX_PITFALL_PLANT_FGM 0x439
#define ITREMIX_PITFALL_BURY_TIMER 120 /* 2 seconds before the buried pitfall can damage its owner */
#define ITREMIX_PITFALL_COLLISION_MASK 0x0C21 /* asm: all collision types */
#define ITREMIX_PITFALL_GROUND_FLAG 0x0800 /* asm COLLISION_MASK: mask_stat bit set when the item lands on ground */
#define ITREMIX_PITFALL_BOUNCE_MUL 0.5F /* asm BOUNCE_MULTIPLIER(0x3F00) as lui: 0x3F000000 */
#define ITREMIX_PITFALL_BURIED_ANGLE_X (-1.5707999467849731F) /* asm 0xBFC90FF9, -90 degrees in radians */
#define ITREMIX_PITFALL_DISPLAY_LIST_OFFSET 0x3F0 /* asm DISPLAY_LIST_OFFSET: skips the item's display list block */
#define ITREMIX_PITFALL_SPAWN_GFX_KIND 0x2E
#define ITREMIX_PITFALL_BURY_GFX_KIND 0x20

enum itRemixPitfallStatus
{
    nITRemixPitfallStatusWait,
    nITRemixPitfallStatusFall,
    nITRemixPitfallStatusPickup,
    nITRemixPitfallStatusThrown,
    nITRemixPitfallStatusBuried,
    nITRemixPitfallStatusEnumCount
};

/* Lives in ITStruct.item_vars. The asm keeps the owner timer at offset 0x350. */
typedef struct ITRemixPitfallVars
{
    s32 owner_timer;
} ITRemixPitfallVars;

_Static_assert(sizeof(ITRemixPitfallVars) <= sizeof(((ITStruct *) 0)->item_vars), "ITRemixPitfallVars must fit in item_vars");

#define ITREMIX_PITFALL_VARS(ip) ((ITRemixPitfallVars *) &(ip)->item_vars)

extern void* func_800269C0_275C0(u16 id);

static void itRemixPitfallApplySpawnHitbox(ITStruct *ip);
static void itRemixPitfallStickToSurface(GObj *item_gobj);
static sb32 itRemixPitfallBeginBuried(GObj *item_gobj);
static sb32 itRemixPitfallThrownProcMap(GObj *item_gobj);
static sb32 itRemixPitfallBuriedMain(GObj *item_gobj);
static sb32 itRemixPitfallBuriedCollision(GObj *item_gobj);
static sb32 itRemixPitfallDestroy(GObj *item_gobj);
static void itRemixPitfallPrepickup(GObj *item_gobj);
static void itRemixPitfallThrowInitial(GObj *item_gobj);

/* asm item_info_array. p_file is the common item file, so no owner is needed. */
static ITDesc sITRemixPitfallDesc =
{
    ITREMIX_PITFALL_ID,
    &gITManagerCommonData,
    ITREMIX_PITFALL_FILE_OFFSET,
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

/* asm item_state_table. Every state after the buried one uses destroy for hits. */
static ITStatusDesc sITRemixPitfallStatusDescs[nITRemixPitfallStatusEnumCount] =
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
        itRemixPitfallThrownProcMap,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy,
        itMainCommonProcReflector,
        itRemixPitfallDestroy
    },
    {
        itRemixPitfallBuriedMain,
        itRemixPitfallBuriedCollision,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy,
        itRemixPitfallDestroy
    }
};

/** Pitfall.destroy_: removes the item on any hit, shield, hop, set-off, absorb or damage. */
static sb32 itRemixPitfallDestroy(GObj *item_gobj)
{
    return TRUE;
}

/** Pitfall.stage_setting_ (SPAWN_ITEM). The vanilla ABI is (parent, pos, vel, flags). */
GObj* itRemixPitfallMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj;
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    item_gobj = itManagerMakeItem(parent_gobj, &sITRemixPitfallDesc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    joint = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);
    translate = joint->translate.vec.f;

    gcAddXObjForDObjFixed(joint, ITREMIX_PITFALL_SPAWN_GFX_KIND, 0);
    joint->translate.vec.f = translate;
    joint->rotate.vec.f.z = 0.0F;

    itRemixPitfallApplySpawnHitbox(ip);
    ip->multi = 0;
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/**
 * Pitfall.stage_setting_ hitbox part: bury damage type, damage, angle, knockback and
 * aerial hurtbox FGM. Knockback fields follow ITStruct offsets 0x140 (scale),
 * 0x144 (weight) and 0x148 (base); the asm writes the same value to all three.
 */
static void itRemixPitfallApplySpawnHitbox(ITStruct *ip)
{
    ip->attack_coll.element = ITREMIX_PITFALL_DAMAGE_BURY;
    ip->attack_coll.angle = ITREMIX_PITFALL_KB_ANGLE;
    ip->attack_coll.fgm_id = ITREMIX_PITFALL_AERIAL_FGM;
    ip->attack_coll.knockback_scale = ITREMIX_PITFALL_BASE_KB;
    ip->attack_coll.knockback_weight = ITREMIX_PITFALL_BASE_KB;
    ip->attack_coll.knockback_base = ITREMIX_PITFALL_BASE_KB;
    ip->attack_coll.damage = ITREMIX_PITFALL_DAMAGE;
}

/**
 * Pitfall.collision_ (thrown proc_map): bounces off map collisions, then buries the
 * pitfall when it lands on a ground line.
 */
static sb32 itRemixPitfallThrownProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    sb32 is_collided;

    is_collided = itMapTestAllCollisionFlag(item_gobj, ITREMIX_PITFALL_COLLISION_MASK);
    itMapCheckCollideAllRebound(item_gobj, ITREMIX_PITFALL_COLLISION_MASK, ITREMIX_PITFALL_BOUNCE_MUL, NULL);
    if (!is_collided)
    {
        return FALSE;
    }
    if (!(ip->coll_data.mask_stat & ITREMIX_PITFALL_GROUND_FLAG))
    {
        return FALSE;
    }

    return itRemixPitfallBeginBuried(item_gobj);
}

/**
 * Pitfall.begin_buried_: sticks the pitfall to the ground and switches to the buried
 * status. Returns TRUE when the item must be destroyed instead.
 *
 * REMIX-TODO(hazard): the asm destroys the pitfall on a hazardous floor (Surface clipping
 * flag, Toggles.entry_hazard_mode, SAFFRON_CITY exception). The port always buries it,
 * because the Surface clipping helper is not in the port yet.
 * The asm's 0x2CE mask 0xCF (times_landed) and clang bit 0x8000 of 0x158 (can_setoff) are cleared below.
 */
static sb32 itRemixPitfallBeginBuried(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itMapSetGround(ip);
    ip->times_landed = 0;
    ip->attack_coll.can_setoff = FALSE;
    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    itRemixPitfallStickToSurface(item_gobj);
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    itMainSetStatus(item_gobj, sITRemixPitfallStatusDescs, nITRemixPitfallStatusBuried);

    return FALSE;
}

/**
 * Pitfall.stick_to_surface: attaches the model to the floor, sets the owner timer and
 * the grounded hurtbox, and plays the plant sound.
 *
 * REMIX-TODO(rumble): the asm calls ftParamMakeRumble when the planting player's
 * match_info entry (offset 0x78) is set. The asm passes the item struct where the
 * function takes an FTStruct, and the match_info field is not named, so it is skipped.
 */
static void itRemixPitfallStickToSurface(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    ip->is_attach_surface = TRUE;
    joint->dl = (Gfx *) ((u8 *) joint->dl + ITREMIX_PITFALL_DISPLAY_LIST_OFFSET);
    gcAddXObjForDObjFixed(joint, ITREMIX_PITFALL_BURY_GFX_KIND, 0);
    itMSBombAttachedUpdateSurface(item_gobj);
    joint->rotate.vec.f.x = ITREMIX_PITFALL_BURIED_ANGLE_X;
    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    ip->attack_coll.knockback_scale = ITREMIX_PITFALL_BASE_KB;
    ip->attack_coll.fgm_id = ITREMIX_PITFALL_AERIAL_FGM;
    ITREMIX_PITFALL_VARS(ip)->owner_timer = ITREMIX_PITFALL_BURY_TIMER;
    func_800269C0_275C0(ITREMIX_PITFALL_PLANT_FGM);
}

/**
 * Pitfall.buried_main_ (buried proc_update): counts down the owner timer, then removes
 * the owner and hides the model so the buried pitfall can hurt its owner's enemies.
 */
static sb32 itRemixPitfallBuriedMain(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixPitfallVars *vars = ITREMIX_PITFALL_VARS(ip);
    DObj *joint = DObjGetStruct(item_gobj);

    vars->owner_timer -= 1;
    if (vars->owner_timer != 0)
    {
        return FALSE;
    }
    ip->owner_gobj = NULL;
    joint->dl = NULL;

    return FALSE;
}

/**
 * Pitfall.buried_collision_ (buried proc_map): destroys the pitfall when its ground line
 * no longer exists.
 *
 * Uncertainty: the asm reads the halfword at ITStruct offset 0x2D0 (the low half of the
 * attr pointer, per struct_offsets.txt) and the comment says "get a flag?". The port uses
 * attach_line_id (0x2CC), which is the line the pitfall is attached to.
 */
static sb32 itRemixPitfallBuriedCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (mpCollisionCheckExistLineID(ip->attach_line_id))
    {
        return FALSE;
    }

    return TRUE;
}

/** Pitfall.prepickup_ (PICKUP_ITEM_INIT): disables the hurtbox and switches to the picked-up status. */
static void itRemixPitfallPrepickup(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNone(item_gobj);
    itMainSetStatus(item_gobj, sITRemixPitfallStatusDescs, nITRemixPitfallStatusPickup);
}

/**
 * Pitfall.throw_initial_ (THROW_ITEM and DROP_ITEM): sets the bury damage type and the
 * aerial knockback, enables the hurtbox and switches to the thrown status.
 */
static void itRemixPitfallThrowInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->attack_coll.element = ITREMIX_PITFALL_DAMAGE_BURY;
    ip->attack_coll.knockback_scale = ITREMIX_PITFALL_BASE_KB;
    ip->attack_coll.knockback_weight = ITREMIX_PITFALL_BASE_KB;
    ip->attack_coll.knockback_base = ITREMIX_PITFALL_BASE_KB;
    ip->attack_coll.can_setoff = FALSE;
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixPitfallStatusDescs, nITRemixPitfallStatusThrown);
}

/** Descriptor of Pitfall (item id 0x34) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixPitfallItemDesc =
{
    ITREMIX_PITFALL_ID,                 /* Item.Pitfall.id */
    itRemixPitfallMakeItem,             /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    itRemixPitfallPrepickup,            /* PICKUP_ITEM_INIT */
    itRemixConsumeTomatoTail,           /* PICKUP_ITEM_MAIN: pickup_item jumps to 0x80145C4C */
    itRemixPitfallThrowInitial,         /* DROP_ITEM (asm throw_initial_) */
    itRemixPitfallThrowInitial,         /* THROW_ITEM (asm throw_initial_) */
    NULL                                /* PLAYER_COLLISION */
};

#endif
