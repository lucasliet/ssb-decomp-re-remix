/**
 * ftremixyounglink.c - C ports of Smash Remix Young Link routines
 * (.toolchain/pkgs/smashremix-src/src/YoungLink/YoungLinkSpecial.asm).
 *
 * YoungLinkUSP.direction_ is the Up Special air interrupt. YoungLinkDSP is the
 * Bombchu item (Link's bomb cloned as item id 0x15): its state routines are
 * ported below, and dFTRemixYoungLinkBombchuStatusDescs is the state table
 * that the item hook recorded in build/remixasm/notes/1F_YLINK.md installs.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <it/item.h>
#include <it/itmain.h>
#include <it/itmap.h>
#include <it/itfighter/itlinkbomb.h>
#include <mp/mpprocess.h>
#include <ef/efmanager.h>
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"

extern void ftCaptainSpecialHiProcInterrupt(GObj *fighter_gobj);
extern void *func_800269C0_275C0(u16 id);
extern ITStatusDesc dFTRemixYoungLinkBombchuStatusDescs[];

#define YLINK_USP_DIRECTION_FLAG 2                 /* temp variable 2 (motion_vars.flags.flag1) */
#define YLINK_USP_DIRECTION_SPEED 40.0F            /* 0x42200000 */
#define YLINK_USP_STICK_SPEED_SCALE 0.0125F        /* 0x3C4CCCCD */

#define BOMBCHU_MOVING_SPEED 34.0F                 /* 0x42080000 */
#define BOMBCHU_FALL_SPEED -20.0F                  /* 0xC1A00000 */
#define BOMBCHU_PI 3.1415927F                      /* 0x40490FDB, 180 degrees in radians */
#define BOMBCHU_BOUNCE_MULTIPLIER 0.375F           /* 0x3EC00000 */
#define BOMBCHU_EXPLODE_WARN_TIMER 96              /* 0x42C00000 as float */
#define BOMBCHU_EXPLODE_WARN_COLANIM 0x4F
#define BOMBCHU_EXPLODE_WARN_COLANIM_LENGTH 0x60
#define BOMBCHU_FOOTSTEP_MASK 0x0007
#define BOMBCHU_FGM_ID 0x0455
#define BOMBCHU_COLLIDE_FLAGS 0x0C21
#define BOMBCHU_GROUND_FLAG 0x0800
#define BOMBCHU_WALL_FLAGS 0x0021
#define BOMBCHU_LEFT_WALL_FLAG 0x0001

/* Bits of the N64 0x2CE/0x2CF bytes, which the decomp names pickup_wait (12 bits from 0x2CE). */
#define BOMBCHU_PICKUP_WAIT_CLEARED_ON_MOVE 0x300  /* andi 0xCF on 0x2CE clears 0x30 of that byte */
#define BOMBCHU_PICKUP_WAIT_BONUS_DAMAGE 0x002     /* ori 0x20 on 0x2CF */
#define BOMBCHU_PICKUP_WAIT_UNKNOWN_FLAG 0x008     /* ori 0x80 on 0x2CF */

enum
{
    BOMBCHU_STATUS_RESTING = 0,
    BOMBCHU_STATUS_BOUNCE = 1,
    BOMBCHU_STATUS_HELD = 2,
    BOMBCHU_STATUS_THROWN = 3,
    BOMBCHU_STATUS_DROPPED_FALLING = 4,
    BOMBCHU_STATUS_EXPLODING = 5,
    BOMBCHU_STATUS_MOVING = 6
};

/* Remix moving-Bombchu frame timer, an integer in the N64 0x2C0 slot that the decomp names vel_scale (f32). */
static s32 *ftRemixYoungLinkBombchuTimer(ITStruct *ip)
{
    return (s32 *)&ip->vel_scale;
}

/* Remix update_direction_: flips the item graphic to match its facing (rotate y = pi when facing left). */
static void ftRemixYoungLinkBombchuUpdateDirection(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = item_gobj->obj;

    joint->rotate.vec.f.y = (ip->lr < 0) ? BOMBCHU_PI : 0.0F;
}

/* Remix end_fgm_: stops the Bombchu's FGM. REMIX-TODO(ITStruct): the FGM handle lives at N64 0x1D0/0x1D4, which the decomp ITStruct does not declare, so no FGM is ever ended. */
static void ftRemixYoungLinkBombchuEndFgm(GObj *item_gobj)
{
    (void)item_gobj;
}

/* Remix begin_falling_: moving Bombchu leaves the floor and falls. */
static void ftRemixYoungLinkBombchuBeginFalling(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itLinkBombCommonSetHitStatusNormal(item_gobj);
    ip->item_vars.linkbomb.scale_int = 0;
    ip->pickup_wait |= BOMBCHU_PICKUP_WAIT_UNKNOWN_FLAG;
    itMainSetStatus(item_gobj, dFTRemixYoungLinkBombchuStatusDescs, BOMBCHU_STATUS_DROPPED_FALLING);
    ip->physics.vel_air.y = BOMBCHU_FALL_SPEED;
    ip->ga = TRUE;
    ftRemixYoungLinkBombchuUpdateDirection(item_gobj);
}

/* Remix begin_explosion_: starts the explosion without destroying the item. */
static sb32 ftRemixYoungLinkBombchuBeginExplosion(GObj *item_gobj)
{
    itLinkBombExplodeInitVars(item_gobj);
    return FALSE;
}

/* Remix begin_moving_: a Bombchu that touched the floor starts rolling toward its facing. */
static void ftRemixYoungLinkBombchuBeginMoving(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->pickup_wait &= ~BOMBCHU_PICKUP_WAIT_CLEARED_ON_MOVE;
    ip->ga = FALSE;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    ip->physics.vel_air.x = BOMBCHU_MOVING_SPEED * (f32)ip->lr;
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, dFTRemixYoungLinkBombchuStatusDescs, BOMBCHU_STATUS_MOVING);
    ip->pickup_wait |= BOMBCHU_PICKUP_WAIT_BONUS_DAMAGE;
    ftRemixYoungLinkBombchuUpdateDirection(item_gobj);
    /* REMIX-TODO(ITStruct): the FGM handle returned here is not stored (see ftRemixYoungLinkBombchuEndFgm). */
    func_800269C0_275C0(BOMBCHU_FGM_ID);
}

/* Remix resting_main_: the Bombchu on the floor, before it is picked up or touched. */
static sb32 ftRemixYoungLinkBombchuRestingProcUpdate(GObj *item_gobj)
{
    sb32 is_done = itLinkBombWaitProcUpdate(item_gobj);

    ftRemixYoungLinkBombchuEndFgm(item_gobj);
    return is_done;
}

/* Remix thrown_main_: the bounce (state 1) and thrown (state 3) Bombchu. */
static sb32 ftRemixYoungLinkBombchuThrownProcUpdate(GObj *item_gobj)
{
    sb32 is_done = itLinkBombFallProcUpdate(item_gobj);

    ftRemixYoungLinkBombchuEndFgm(item_gobj);
    return is_done;
}

/* Remix dropped_main_: the Bombchu dropped by its owner. */
static sb32 ftRemixYoungLinkBombchuDroppedProcUpdate(GObj *item_gobj)
{
    sb32 is_done = itLinkBombDroppedProcUpdate(item_gobj);

    ftRemixYoungLinkBombchuEndFgm(item_gobj);
    return is_done;
}

/* Remix held_main_: the Bombchu held by its owner, facing the owner's direction. */
static sb32 ftRemixYoungLinkBombchuHeldProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint_root = item_gobj->obj;
    DObj *joint_child = joint_root->child;
    sb32 is_done = itLinkBombHoldProcUpdate(item_gobj);

    if (ip->owner_gobj != NULL)
    {
        FTStruct *owner_fp = ftGetStruct(ip->owner_gobj);

        ip->lr = owner_fp->lr;
        joint_child->rotate.vec.f.y = (ip->lr < 0) ? BOMBCHU_PI : 0.0F;
    }
    ftRemixYoungLinkBombchuEndFgm(item_gobj);
    return is_done;
}

/* Remix exploding_main_: the Bombchu explosion. */
static sb32 ftRemixYoungLinkBombchuExplodingProcUpdate(GObj *item_gobj)
{
    sb32 is_done = itLinkBombExplodeProcUpdate(item_gobj);

    ftRemixYoungLinkBombchuEndFgm(item_gobj);
    return is_done;
}

/* Remix moving_main_: the rolling Bombchu, based on Link bomb's 0x80185DCC with the horizontal deceleration removed. */
static sb32 ftRemixYoungLinkBombchuMovingProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = item_gobj->obj;
    s32 *timer = ftRemixYoungLinkBombchuTimer(ip);

    if (*timer == 0)
    {
        itLinkBombExplodeInitVars(item_gobj);
    }
    if (*timer == BOMBCHU_EXPLODE_WARN_TIMER)
    {
        itMainCheckSetColAnimID(item_gobj, BOMBCHU_EXPLODE_WARN_COLANIM, BOMBCHU_EXPLODE_WARN_COLANIM_LENGTH);
        /* REMIX-TODO(ITStruct): Remix also stores 1 as a u16 at N64 0x354, past the ITFighterItemVarsLinkBomb
         * struct, which the decomp does not declare, so this flag is not written. */
    }
    if ((f32)*timer < (f32)BOMBCHU_EXPLODE_WARN_TIMER)
    {
        itLinkBombExplodeWaitUpdateScale(item_gobj);
    }
    *timer -= 1;

    if ((*timer & BOMBCHU_FOOTSTEP_MASK) == 0)
    {
        efManagerDustLightMakeEffect(&joint->translate.vec.f, ip->lr, 1.0F);
    }
    return FALSE;
}

/* Remix thrown_collision_: map collision of thrown and dropped Bombchus; starts moving once it lands on the ground. */
static sb32 ftRemixYoungLinkBombchuThrownProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    mpProcessUpdateMain(&ip->coll_data, itMapProcAllCheckCollisionFlag, item_gobj, BOMBCHU_COLLIDE_FLAGS);
    if (!itMapCheckCollideAllRebound(item_gobj, BOMBCHU_COLLIDE_FLAGS, BOMBCHU_BOUNCE_MULTIPLIER, NULL))
    {
        return FALSE;
    }
    itLinkBombFallSetStatus(item_gobj);

    if ((ip->coll_data.mask_stat & BOMBCHU_GROUND_FLAG) == 0)
    {
        return FALSE;
    }
    ftRemixYoungLinkBombchuBeginMoving(item_gobj);
    return FALSE;
}

/* Remix moving_collision_: a rolling Bombchu reverses at walls and falls off when it leaves the floor. */
static sb32 ftRemixYoungLinkBombchuMovingProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    u16 wall_flags;
    sb32 is_left;

    itMapCheckLRWallProcNoFloor(item_gobj, ftRemixYoungLinkBombchuBeginFalling);
    if (ip->ga)
    {
        return FALSE;
    }
    wall_flags = ip->coll_data.mask_curr & BOMBCHU_WALL_FLAGS;
    if (wall_flags == 0)
    {
        return FALSE;
    }
    is_left = (wall_flags & BOMBCHU_LEFT_WALL_FLAG) != 0;
    ip->physics.vel_air.x = is_left ? -BOMBCHU_MOVING_SPEED : BOMBCHU_MOVING_SPEED;
    ip->lr = is_left ? -1 : 1;
    ftRemixYoungLinkBombchuUpdateDirection(item_gobj);
    return FALSE;
}

/* Remix hurtbox_collision_: general hurtbox handling of the Bombchu, then the graphic direction. */
static sb32 ftRemixYoungLinkBombchuProcDamage(GObj *item_gobj)
{
    itLinkBombCommonProcDamage(item_gobj);
    ftRemixYoungLinkBombchuUpdateDirection(item_gobj);
    return FALSE;
}

/* Remix dropped_hurtbox_collision_: hurtbox handling of a dropped Bombchu, then the graphic direction. */
static sb32 ftRemixYoungLinkBombchuDroppedProcDamage(GObj *item_gobj)
{
    itLinkBombDroppedProcDamage(item_gobj);
    ftRemixYoungLinkBombchuUpdateDirection(item_gobj);
    return FALSE;
}

/* Remix reflect_: a reflected Bombchu takes the owner's facing. REMIX-TODO(uncertain): Remix reads float f6 left by
 * itMainCommonProcReflector as the new direction; the owner's lr is assumed here (see the notes file). */
static sb32 ftRemixYoungLinkBombchuProcReflector(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    FTStruct *owner_fp = ftGetStruct(ip->owner_gobj);

    itMainCommonProcReflector(item_gobj);
    ip->lr = owner_fp->lr;
    ftRemixYoungLinkBombchuUpdateDirection(item_gobj);
    return FALSE;
}

/* Remix bombchu_blast_zone_: ends the Bombchu when it crosses the blast zone. Called by the item blast-zone hook
 * recorded in the notes file. Returns TRUE to destroy the item. */
sb32 ftRemixYoungLinkBombchuBlastZone(GObj *item_gobj)
{
    ftRemixYoungLinkBombchuEndFgm(item_gobj);
    return TRUE;
}

/* Bombchu item state table (Remix item_state_table), installed by the itMainSetStatus hook in the notes file. */
ITStatusDesc dFTRemixYoungLinkBombchuStatusDescs[] =
{
    /* Status 0 (resting) */
    {
        ftRemixYoungLinkBombchuRestingProcUpdate,
        itLinkBombWaitProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ftRemixYoungLinkBombchuProcDamage
    },
    /* Status 1 (bounce) */
    {
        ftRemixYoungLinkBombchuThrownProcUpdate,
        itLinkBombFallProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ftRemixYoungLinkBombchuProcDamage
    },
    /* Status 2 (held) */
    {
        ftRemixYoungLinkBombchuHeldProcUpdate,
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
        ftRemixYoungLinkBombchuThrownProcUpdate,
        ftRemixYoungLinkBombchuThrownProcMap,
        itLinkBombThrownProcHit,
        itLinkBombCommonProcShield,
        itMainCommonProcHop,
        NULL,
        ftRemixYoungLinkBombchuProcReflector,
        ftRemixYoungLinkBombchuProcDamage
    },
    /* Status 4 (dropped / falling) */
    {
        ftRemixYoungLinkBombchuDroppedProcUpdate,
        ftRemixYoungLinkBombchuThrownProcMap,
        itLinkBombDroppedProcHit,
        itLinkBombCommonProcShield,
        itMainCommonProcHop,
        NULL,
        ftRemixYoungLinkBombchuProcReflector,
        ftRemixYoungLinkBombchuDroppedProcDamage
    },
    /* Status 5 (exploding) */
    {
        ftRemixYoungLinkBombchuExplodingProcUpdate,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },
    /* Status 6 (moving) */
    {
        ftRemixYoungLinkBombchuMovingProcUpdate,
        ftRemixYoungLinkBombchuMovingProcMap,
        ftRemixYoungLinkBombchuBeginExplosion,
        ftRemixYoungLinkBombchuBeginExplosion,
        NULL,
        NULL,
        ftRemixYoungLinkBombchuProcReflector,
        ftRemixYoungLinkBombchuProcDamage
    }
};

/* YoungLinkUSP.direction_: Up Special air interrupt. With temp variable 2 set to 2 it turns with the stick and sets
 * the x velocity from the stick's x magnitude. */
static void ftRemixYoungLinkUSPDirectionProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 stick_x;

    if (fp->motion_vars.flags.flag1 != YLINK_USP_DIRECTION_FLAG)
    {
        return;
    }
    ftCaptainSpecialHiProcInterrupt(fighter_gobj);

    stick_x = fp->input.pl.stick_range.x;
    if (stick_x < 0)
    {
        stick_x = -stick_x;
    }
    fp->physics.vel_air.x = YLINK_USP_DIRECTION_SPEED * ((f32)stick_x * YLINK_USP_STICK_SPEED_SCALE) * (f32)fp->lr;
}

#define YLINK_USP_SPIN_AIR_VEL_Y 52.0F             /* 0x42500000, vanilla Link is FTLINK_SPINATTACK_AIR_VEL_Y */
#define YLINK_USP_SPIN_LANDING_LAG 0.33F           /* 0x3EA8F5C3, 25 frames of landing lag */
#define YLINK_BOOMERANG_TILT_VEL 70.0F             /* 0x428C0000; the asm comment says 65 (U11) */
#define YLINK_BOOMERANG_SMASH_VEL 99.0F            /* 0x42C60000 */

/* Owner character for the boomerang: a Kirby or J Kirby that copied a power uses the copied character (N64 0x0ADC). */
static sb32 ftRemixYoungLinkBoomerangOwnerIsYoungLink(FTStruct *fp)
{
    if ((fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return *ftRemixFighterWord(fp, 0x0ADC) == REMIX_ID_YLINK;
    }
    return fp->fkind == REMIX_ID_YLINK;
}

/* H6, linkshared.asm up_special_velocity_: vertical velocity of the Up Special spin. */
f32 ftRemixYoungLinkUpSpecialAirVelY(FTStruct *fp, f32 vanilla_vel_y)
{
    if (fp->fkind != REMIX_ID_YLINK)
    {
        return vanilla_vel_y;
    }
    return YLINK_USP_SPIN_AIR_VEL_Y;
}

/* H7, linkshared.asm/YoungLink.asm up_special_landing_: landing lag passed to ftCommonFallSpecialSetStatus. */
f32 ftRemixYoungLinkUpSpecialLandingLag(FTStruct *fp, f32 vanilla_lag)
{
    if (fp->fkind != REMIX_ID_YLINK)
    {
        return vanilla_lag;
    }
    return YLINK_USP_SPIN_LANDING_LAG;
}

/* H9 slow path (boomerang_speed_, TILT): replaces the vel_mul argument of wpLinkBoomerangGetAngleSetVel. */
f32 ftRemixYoungLinkBoomerangTiltVel(FTStruct *fp, f32 vanilla_vel)
{
    if (!ftRemixYoungLinkBoomerangOwnerIsYoungLink(fp))
    {
        return vanilla_vel;
    }
    return YLINK_BOOMERANG_TILT_VEL;
}

/* H9 fast path (boomerang_speed_, SMASH): replaces the vel_mul argument of wpLinkBoomerangGetAngleSetVel. */
f32 ftRemixYoungLinkBoomerangSmashVel(FTStruct *fp, f32 vanilla_vel)
{
    if (!ftRemixYoungLinkBoomerangOwnerIsYoungLink(fp))
    {
        return vanilla_vel;
    }
    return YLINK_BOOMERANG_SMASH_VEL;
}

/* H2, linkshared.asm save_character_id_: a bomb owned by Young Link records the owner id in coll_data.cliff_id,
 * which the Bombchu state-table hook (H1) reads. Other owners are left untouched. */
void ftRemixYoungLinkBombSetOwner(GObj *item_gobj, FTStruct *owner_fp)
{
    ITStruct *ip;

    if (owner_fp->fkind != REMIX_ID_YLINK)
    {
        return;
    }
    ip = itGetStruct(item_gobj);
    ip->coll_data.cliff_id = owner_fp->fkind;
    /* REMIX-TODO(ITStruct): the Remix code also sets item+0x1D0 (FGM handle = NULL) and item+0x398 (blast zone =
     * ftRemixYoungLinkBombchuBlastZone). Neither field is in the decomp ITStruct yet. */
}

/* Remix address -> port routine for every Young Link routine its action array or special tables reference. */
const FTRemixProc dFTRemixYoungLinkProcs[] =
{
    { REMIX_SYM_YoungLinkUSP, ftRemixYoungLinkUSPDirectionProcInterrupt }
};

const s32 dFTRemixYoungLinkProcsNum = ARRAY_COUNT(dFTRemixYoungLinkProcs);

#endif
