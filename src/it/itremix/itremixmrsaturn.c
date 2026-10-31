#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixmrsaturn.h"
#include <it/item.h>
#include <it/itcommon/ittomato.h>
#include <it/itcommon/itbombhei.h>
#include <if/ifcommon.h>
#include <ft/fighter.h>

#define ITREMIX_MRSATURN_KIND 0x38
#define ITREMIX_MRSATURN_FILE_OFFSET 0x10B0
#define ITREMIX_MRSATURN_BASE_DAMAGE 6
#define ITREMIX_MRSATURN_SHIELD_DAMAGE 30
#define ITREMIX_MRSATURN_BKB 25
#define ITREMIX_MRSATURN_KBG 60
#define ITREMIX_MRSATURN_KB_ANGLE 361
#define ITREMIX_MRSATURN_HIT_FGM 0x0038
#define ITREMIX_MRSATURN_THROW_TIMER 50
#define ITREMIX_MRSATURN_WAIT_TIME 120
#define ITREMIX_MRSATURN_WALK_TIME (25 * 60)
#define ITREMIX_MRSATURN_IMAGE_WALK_A 4
#define ITREMIX_MRSATURN_IMAGE_WALK_B 5
#define ITREMIX_MRSATURN_IMAGE_SPAWN 6
#define ITREMIX_MRSATURN_WALK_SPEED 8.0F
#define ITREMIX_MRSATURN_GRAVITY 1.7546875F /* 0x3FE0999A */
#define ITREMIX_MRSATURN_THROWN_SPEED 100.0F /* 0x42C80000 */
#define ITREMIX_MRSATURN_HIT_X_MULTIPLIER -0.125F /* 0xBE000000 */
#define ITREMIX_MRSATURN_HIT_Y_VELOCITY 40.0F /* 0x42200000 */
#define ITREMIX_MRSATURN_BOUNCE_LAND_REBOUND 0.3F /* 0x3E99999A */
#define ITREMIX_MRSATURN_GROUND_REBOUND 0.5F /* 0x3F000000 */
#define ITREMIX_MRSATURN_WALL_MASK 0x0021 /* Remix mask applied to coll_data.mask_curr */
#define ITREMIX_MRSATURN_FGM_BASE 0x05FD /* hit sounds 0x5FD..0x5FF, one picked at random */
#define ITREMIX_MRSATURN_FGM_VARIANTS 3

enum itRemixMrSaturnStatus
{
    nITRemixMrSaturnStatusGroundWait,
    nITRemixMrSaturnStatusAerialWait,
    nITRemixMrSaturnStatusHeld,
    nITRemixMrSaturnStatusThrown,
    nITRemixMrSaturnStatusFall,
    nITRemixMrSaturnStatusWalk,
    nITRemixMrSaturnStatusEnumCount
};

/* Remix private state. Stored in the item_vars union, which the shell
 * fields use only at offset 8 and later (vel_x), so the two timers at 0 and 4
 * do not overlap the field the red-shell edge routine reads. */
typedef struct ITRemixMrSaturnVars
{
    s32 wait_timer;
    s32 throw_timer;
} ITRemixMrSaturnVars;

_Static_assert(sizeof(ITRemixMrSaturnVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixMrSaturnVars must fit in ITStruct.item_vars");

extern void *func_800269C0_275C0(u16 id);
extern void itRShellSpinCheckCollisionEdge(GObj *item_gobj);

static sb32 itRemixMrSaturnGroundWaitProcMap(GObj *item_gobj);
static sb32 itRemixMrSaturnGroundedCheckProcMap(GObj *item_gobj);
static sb32 itRemixMrSaturnGroundProcMap(GObj *item_gobj);
static void itRemixMrSaturnWaitSetStatus(GObj *item_gobj);
static sb32 itRemixMrSaturnWaitProcUpdate(GObj *item_gobj);
static sb32 itRemixMrSaturnWalkProcUpdate(GObj *item_gobj);
static sb32 itRemixMrSaturnHeldProcUpdate(GObj *item_gobj);
static sb32 itRemixMrSaturnThrownProcUpdate(GObj *item_gobj);
static sb32 itRemixMrSaturnThrowCollide(GObj *item_gobj);
static sb32 itRemixMrSaturnThrowCollideHitbox(GObj *item_gobj);
static sb32 itRemixMrSaturnReflectProc(GObj *item_gobj);
static void itRemixMrSaturnFallSetStatus(GObj *item_gobj);
static void itRemixMrSaturnThrowSetStatus(GObj *item_gobj);
static void itRemixMrSaturnReflectedSetStatus(GObj *item_gobj);
static void itRemixMrSaturnHoldProc(GObj *item_gobj);

static ITStatusDesc sITRemixMrSaturnStatusDescs[nITRemixMrSaturnStatusEnumCount] =
{
    /* Status 0 (grounded wait) */
    {
        itRemixMrSaturnWaitProcUpdate,      /* wait_main */
        itRemixMrSaturnGroundWaitProcMap,   /* grounded_wait_collision */
        NULL,                               /* Proc Hit */
        NULL,                               /* Proc Shield */
        NULL,                               /* Proc Hop */
        NULL,                               /* Proc Set-Off */
        NULL,                               /* Proc Reflector */
        NULL                                /* Proc Damage */
    },

    /* Status 1 (aerial wait). The Remix code never enters it; kept so the indexes match. */
    {
        itTomatoFallProcUpdate,             /* 0x801744C0 */
        itRemixMrSaturnGroundProcMap,       /* ground_collision */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Status 2 (held) */
    {
        itRemixMrSaturnHeldProcUpdate,      /* held_main */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Status 3 (thrown) */
    {
        itRemixMrSaturnThrownProcUpdate,    /* thrown_main_ */
        itRemixMrSaturnGroundProcMap,       /* ground_collision */
        itRemixMrSaturnThrowCollide,        /* hitbox collision w/ hurtbox */
        itRemixMrSaturnThrowCollide,        /* hitbox collision w/ shield */
        itMainCommonProcHop,                /* 0x801733E4 */
        itRemixMrSaturnThrowCollide,        /* clang */
        itRemixMrSaturnReflectProc,         /* hitbox collision w/ reflector */
        itRemixMrSaturnThrowCollideHitbox   /* hurtbox collision w/ hitbox */
    },

    /* Status 4 (falling) */
    {
        itTomatoFallProcUpdate,             /* 0x801744C0 */
        itRemixMrSaturnGroundProcMap,       /* ground_collision */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Status 5 (walking) */
    {
        itRemixMrSaturnWalkProcUpdate,      /* walking_main */
        itRemixMrSaturnGroundedCheckProcMap, /* grounded_check_collision_ */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

static ITDesc sITRemixMrSaturnItemInfo =
{
    ITREMIX_MRSATURN_KIND,                  /* Item.MrSaturn.id */
    &gITManagerCommonData,                  /* item file 0xFB: asm dw 0x8018D040 */
    ITREMIX_MRSATURN_FILE_OFFSET,           /* item footer offset */

    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itTomatoFallProcUpdate,                 /* 0x801744C0 */
    itRemixMrSaturnGroundProcMap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

const ITRemixItemDesc dITRemixMrSaturnItemDesc =
{
    ITREMIX_MRSATURN_KIND,
    itRemixMrSaturnMakeItem,
    TRUE,
    itRemixMrSaturnHoldProc,                /* prepickup_ */
    NULL,                                   /* pickup_: tail-jumps back to the vanilla pickup return, no effect */
    itRemixMrSaturnFallSetStatus,           /* falling_initial_ */
    itRemixMrSaturnThrowSetStatus,          /* throw_initial_ */
    NULL
};

static ITRemixMrSaturnVars* itRemixMrSaturnGetVars(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    return (ITRemixMrSaturnVars*)&ip->item_vars;
}

/* MrSaturn: stage_setting_ image setup (texture 5 is overwritten by 6 in the same routine) */
static void itRemixMrSaturnSetupImage(GObj *item_gobj)
{
    DObj *joint = DObjGetStruct(item_gobj);

    joint->mobj->texture_id_curr = ITREMIX_MRSATURN_IMAGE_SPAWN;
}

/* MrSaturn: stage_setting_ hitbox and knockback values */
static void itRemixMrSaturnSetupAttack(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->attack_coll.attack_state = nGMAttackStateOff;
    ip->attack_coll.damage = ITREMIX_MRSATURN_BASE_DAMAGE;
    ip->attack_coll.knockback_scale = ITREMIX_MRSATURN_KBG;
    ip->attack_coll.knockback_base = ITREMIX_MRSATURN_BKB;
    ip->attack_coll.angle = ITREMIX_MRSATURN_KB_ANGLE;
    ip->attack_coll.shield_damage = ITREMIX_MRSATURN_SHIELD_DAMAGE;
    ip->attack_coll.fgm_id = ITREMIX_MRSATURN_HIT_FGM;
}

/* MrSaturn: stage_setting_ state, direction and arrow setup */
static void itRemixMrSaturnSetupState(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixMrSaturnVars *vars = itRemixMrSaturnGetVars(item_gobj);

    vars->throw_timer = 0;
    vars->wait_timer = ITREMIX_MRSATURN_WAIT_TIME;
    ip->lr = 1;
    ip->item_vars.shell.vel_x = 0.0F;
    DObjGetStruct(item_gobj)->rotate.vec.f.z = 0.0F;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);
}

/* MrSaturn: stage_setting_ (after itManagerMakeItem) */
static void itRemixMrSaturnSetup(GObj *item_gobj)
{
    itRemixMrSaturnSetupImage(item_gobj);
    itRemixMrSaturnSetupAttack(item_gobj);
    itRemixMrSaturnSetupState(item_gobj);
}

/* MrSaturn: stage_setting_. Remix SPAWN_ITEM (a0 parent, a1 pos, a2 vel, a3 flags) */
GObj* itRemixMrSaturnMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixMrSaturnItemInfo, pos, vel, flags);

    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixMrSaturnSetup(item_gobj);

    return item_gobj;
}

/* MrSaturn: grounded_wait_collision */
static sb32 itRemixMrSaturnGroundWaitProcMap(GObj *item_gobj)
{
    itMapCheckLRWallProcNoFloor(item_gobj, itRemixMrSaturnFallSetStatus);

    return FALSE;
}

/* MrSaturn: grounded_check_collision_ */
static sb32 itRemixMrSaturnGroundedCheckProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (itMapCheckLRWallProcNoFloor(item_gobj, itRemixMrSaturnFallSetStatus) == FALSE)
    {
        return FALSE;
    }
    if ((ip->coll_data.mask_curr & ITREMIX_MRSATURN_WALL_MASK) == 0)
    {
        return FALSE;
    }
    ip->physics.vel_air.x = -ip->physics.vel_air.x;
    itMainSetSpinVelLR(item_gobj);
    itMainClearOwnerStats(item_gobj);
    ip->item_vars.shell.vel_x = -ip->item_vars.shell.vel_x;

    return FALSE;
}

/* MrSaturn: ground_collision */
static sb32 itRemixMrSaturnGroundProcMap(GObj *item_gobj)
{
    return itMapCheckDestroyDropped(item_gobj, ITREMIX_MRSATURN_BOUNCE_LAND_REBOUND, ITREMIX_MRSATURN_GROUND_REBOUND, itRemixMrSaturnWaitSetStatus);
}

/* MrSaturn: wait_initial */
static void itRemixMrSaturnWaitSetStatus(GObj *item_gobj)
{
    itRemixMrSaturnGetVars(item_gobj)->wait_timer = ITREMIX_MRSATURN_WAIT_TIME;
    itMainSetGroundAllowPickup(item_gobj);
    itMainSetStatus(item_gobj, sITRemixMrSaturnStatusDescs, nITRemixMrSaturnStatusGroundWait);
}

/* MrSaturn: wait_main */
static sb32 itRemixMrSaturnWaitProcUpdate(GObj *item_gobj)
{
    ITRemixMrSaturnVars *vars = itRemixMrSaturnGetVars(item_gobj);

    vars->wait_timer -= 1;
    if (vars->wait_timer != 0)
    {
        return FALSE;
    }
    itMainSetStatus(item_gobj, sITRemixMrSaturnStatusDescs, nITRemixMrSaturnStatusWalk);
    vars->wait_timer = ITREMIX_MRSATURN_WALK_TIME;

    return FALSE;
}

/* MrSaturn: _update_x_scale. Walk and held routines flip the joint scale by direction */
static void itRemixMrSaturnUpdateXScale(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    joint->scale.vec.f.x = (ip->lr == 1) ? 1.0F : -1.0F;
}

/* MrSaturn: _update_held_x_scale */
static void itRemixMrSaturnUpdateHeldXScale(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *held = DObjGetStruct(item_gobj)->child;

    held->scale.vec.f.x = (ip->lr == 1) ? 1.0F : -1.0F;
}

/* MrSaturn: _animate_saturn. Toggles the image every 4 frames of the walk timer */
static void itRemixMrSaturnAnimateWalk(GObj *item_gobj)
{
    ITRemixMrSaturnVars *vars = itRemixMrSaturnGetVars(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    if ((vars->wait_timer & 3) != 0)
    {
        return;
    }
    if (joint->mobj->texture_id_curr == ITREMIX_MRSATURN_IMAGE_WALK_A)
    {
        joint->mobj->texture_id_curr = ITREMIX_MRSATURN_IMAGE_WALK_B;
    }
    else
    {
        joint->mobj->texture_id_curr = ITREMIX_MRSATURN_IMAGE_WALK_A;
    }
}

/* MrSaturn: _animate_held_saturn. Same toggle on the held child DObj */
static void itRemixMrSaturnAnimateHeld(GObj *item_gobj)
{
    ITRemixMrSaturnVars *vars = itRemixMrSaturnGetVars(item_gobj);
    DObj *held = DObjGetStruct(item_gobj)->child;

    if ((vars->wait_timer & 3) != 0)
    {
        return;
    }
    if (held->mobj->texture_id_curr == ITREMIX_MRSATURN_IMAGE_WALK_A)
    {
        held->mobj->texture_id_curr = ITREMIX_MRSATURN_IMAGE_WALK_B;
    }
    else
    {
        held->mobj->texture_id_curr = ITREMIX_MRSATURN_IMAGE_WALK_A;
    }
}

/* MrSaturn: walking_main. The vanilla edge routine flips lr and velocity at a ledge */
static sb32 itRemixMrSaturnWalkProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixMrSaturnVars *vars = itRemixMrSaturnGetVars(item_gobj);

    vars->wait_timer -= 1;
    if (vars->wait_timer == 0)
    {
        return TRUE;
    }
    itRShellSpinCheckCollisionEdge(item_gobj);
    ip->physics.vel_air.x = (ip->lr == -1) ? -ITREMIX_MRSATURN_WALK_SPEED : ITREMIX_MRSATURN_WALK_SPEED;
    itRemixMrSaturnUpdateXScale(item_gobj);
    itRemixMrSaturnAnimateWalk(item_gobj);

    return FALSE;
}

/* MrSaturn: held_main */
static sb32 itRemixMrSaturnHeldProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixMrSaturnVars *vars = itRemixMrSaturnGetVars(item_gobj);
    FTStruct *fp;

    itRemixMrSaturnAnimateHeld(item_gobj);
    if (ip->owner_gobj != NULL)
    {
        fp = ftGetStruct(ip->owner_gobj);
        ip->lr = fp->lr;
        itRemixMrSaturnUpdateHeldXScale(item_gobj);
    }
    vars->wait_timer += 1;

    return FALSE;
}

/* MrSaturn: thrown_main_ */
static sb32 itRemixMrSaturnThrownProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixMrSaturnVars *vars = itRemixMrSaturnGetVars(item_gobj);

    itVisualsUpdateSpin(item_gobj);
    vars->throw_timer -= 1;
    itMainApplyGravityClampTVel(ip, ITREMIX_MRSATURN_GRAVITY, ITREMIX_MRSATURN_THROWN_SPEED);
    itVisualsUpdateSpin(item_gobj);

    return FALSE;
}

/* MrSaturn: throw_collide_ bounce, then the shared fall setup */
static void itRemixMrSaturnBounceOff(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x *= ITREMIX_MRSATURN_HIT_X_MULTIPLIER;
    ip->physics.vel_air.y = ITREMIX_MRSATURN_HIT_Y_VELOCITY;
    itRemixMrSaturnFallSetStatus(item_gobj);
}

/* MrSaturn: throw_collide_ hit sound (random one of three), picked by Global.get_random_int_(3) */
static void itRemixMrSaturnPlayHitSound(void)
{
    s32 variant = syUtilsRandIntRange(ITREMIX_MRSATURN_FGM_VARIANTS);

    func_800269C0_275C0(ITREMIX_MRSATURN_FGM_BASE + variant);
}

/* MrSaturn: throw_collide_ */
static sb32 itRemixMrSaturnThrowCollide(GObj *item_gobj)
{
    itRemixMrSaturnBounceOff(item_gobj);
    itRemixMrSaturnPlayHitSound();

    return FALSE;
}

/* MrSaturn: throw_collide_hitbox_ (no sound, disables the hurtbox) */
static sb32 itRemixMrSaturnThrowCollideHitbox(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x *= ITREMIX_MRSATURN_HIT_X_MULTIPLIER;
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    ip->physics.vel_air.y = ITREMIX_MRSATURN_HIT_Y_VELOCITY;
    itRemixMrSaturnFallSetStatus(item_gobj);

    return FALSE;
}

/* MrSaturn: reflect_ */
static sb32 itRemixMrSaturnReflectProc(GObj *item_gobj)
{
    itMainCommonProcReflector(item_gobj);
    itRemixMrSaturnReflectedSetStatus(item_gobj);

    return FALSE;
}

/* MrSaturn: reflected_initial_ */
static void itRemixMrSaturnReflectedSetStatus(GObj *item_gobj)
{
    itRemixMrSaturnGetVars(item_gobj)->throw_timer = ITREMIX_MRSATURN_THROW_TIMER;
    itMainSetStatus(item_gobj, sITRemixMrSaturnStatusDescs, nITRemixMrSaturnStatusThrown);
}

/* MrSaturn: falling_initial_ (drop and post-hit fall) */
static void itRemixMrSaturnFallSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->attack_coll.attack_state = nGMAttackStateOff;
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    itMainSetStatus(item_gobj, sITRemixMrSaturnStatusDescs, nITRemixMrSaturnStatusFall);
}

/* MrSaturn: throw_initial_ */
static void itRemixMrSaturnThrowSetStatus(GObj *item_gobj)
{
    itMainSetStatus(item_gobj, sITRemixMrSaturnStatusDescs, nITRemixMrSaturnStatusThrown);
    itRemixMrSaturnUpdateXScale(item_gobj);
}

/* MrSaturn: prepickup_ (pre-pickup hold) */
static void itRemixMrSaturnHoldProc(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNone(item_gobj);
    itMainSetStatus(item_gobj, sITRemixMrSaturnStatusDescs, nITRemixMrSaturnStatusHeld);
}

#endif
