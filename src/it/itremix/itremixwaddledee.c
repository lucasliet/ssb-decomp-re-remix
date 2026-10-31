/**
 * itremixwaddledee.c - Smash Remix Waddle Dee (Item.WaddleDee.id 0x46), the
 * minion from Dedede's down special. Port of items/WaddleDee.asm.
 *
 * Waddle Doo (itremixwaddledoo.c) and Gordo reuse the routines exported in
 * itremixwaddledee.h. SKIPPED(Dedede minion slots): the owner minion pointers are not named in the port; see
 * docs/remix/notes/item_WaddleDee.md for every deviation.
 */

#ifdef PORT

#include <it/item.h>
#include <it/itmain.h>
#include <it/itmap.h>
#include <it/itmanager.h>
#include <it/itcommon/itbombhei.h>
#include <it/itfighter/itlinkbomb.h>
#include <ft/fighter.h>
#include <ft/ftdef.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <gm/gmcollision.h>
#include <mp/mpcollision.h>
#include <mp/mpdef.h>
#include <mp/mpprocess.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objdef.h>
#include <math.h>
#include "it/itremix/itremix.h"
#include "it/itremix/itremixwaddledee.h"
#include "it/itremix/itremixwaddledoo.h"
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremix.h"

extern void *func_800269C0_275C0(u16 id);

#define ITREMIX_WADDLEDEE_KIND 0x46 /* Item.WaddleDee.id */
#define ITREMIX_WADDLEDOO_KIND 0x47 /* Item.WaddleDoo.id */
#define ITREMIX_WADDLEDEE_FAKE_KIND 0x1C /* kind set before a silent destroy, so the destroy skips its smoke puff */
#define ITREMIX_WADDLEDEE_FILE_OFFSET 0x40 /* item footer offset, used as o_attributes */
#define ITREMIX_WADDLEDEE_HAND_JOINT 10 /* FTStruct.joints[10] = N64 0x0910, the left hand */
#define ITREMIX_WADDLEDEE_NO_COMBO_PORT 4 /* combo ownership port when the hitting object is not a fighter */

#define ITREMIX_WADDLEDEE_FGM_LAND 0x044B
#define ITREMIX_WADDLEDEE_FGM_JUMP 0x044A
#define ITREMIX_WADDLEDEE_COLL_MASK 0x0C21
#define ITREMIX_WADDLEDEE_CHECK_COLL_MASK 0x0421
#define ITREMIX_WADDLEDEE_ANIM_BASE 0x33F8 /* item file offset base for the walk animation vectors */
#define ITREMIX_WADDLEDEE_ANIM_RIGHT 0x3310
#define ITREMIX_WADDLEDEE_ANIM_LEFT 0x34C0
#define ITREMIX_WADDLEDEE_IMAGE_THROWN 0
#define ITREMIX_WADDLEDEE_IMAGE_IDLE 1
#define ITREMIX_WADDLEDEE_IMAGE_LANDING 1
#define ITREMIX_WADDLEDEE_IMAGE_JUMPSQUAT 5
#define ITREMIX_WADDLEDEE_IMAGE_JUMP 6
#define ITREMIX_WADDLEDEE_HIT_RECORDS 4

#define ITREMIX_WADDLEDEE_DURATION 480
#define ITREMIX_WADDLEDEE_GRAVITY 2.0F
#define ITREMIX_WADDLEDEE_MAX_SPEED 48.0F
#define ITREMIX_WADDLEDEE_BOUNCE 0.3F
#define ITREMIX_WADDLEDEE_ANGLE 0.872665F
#define ITREMIX_WADDLEDEE_ROTATION 0.003F
#define ITREMIX_WADDLEDEE_ROTATION_BASE 0.017578125F /* 0x3C900000 */
#define ITREMIX_WADDLEDEE_GROUND_SPEED 15.0F
#define ITREMIX_WADDLEDEE_LAND_LAG 16
#define ITREMIX_WADDLEDEE_JUMP_SPEED_X 8.0F
#define ITREMIX_WADDLEDEE_JUMP_SPEED_Y 40.0F
#define ITREMIX_WADDLEDEE_ATTACK_DELAY 4
#define ITREMIX_WADDLEDOO_ATTACK_DELAY 8
#define ITREMIX_WADDLEDEE_ANGLE_STICK_SCALE -0.0078125F /* 0xBC000000 */
#define ITREMIX_WADDLEDEE_SLIDE_MULTIPLIER 0.9375F /* 0x3F700000; the N64 comment says 0.875 */
#define ITREMIX_WADDLEDEE_SLIDE_MIN_SPEED 2.0F
#define ITREMIX_WADDLEDEE_AIR_SPEED_DECAY 1.0F
#define ITREMIX_WADDLEDEE_HITBOX_REFRESH_FRAMES 16
#define ITREMIX_WADDLEDEE_HIT_DURATION_MIN 20
#define ITREMIX_WADDLEDEE_HIT_DURATION_MUL 8
#define ITREMIX_WADDLEDEE_LANDING_DURATION_MIN 12
#define ITREMIX_WADDLEDEE_THROW_HEIGHT 200.0F
#define ITREMIX_WADDLEDEE_HOLD_OFFSET_X 144.0F

#define ITREMIX_WADDLEDEE_RANGE_X 600.0F
#define ITREMIX_WADDLEDEE_RANGE_Y 700.0F
#define ITREMIX_WADDLEDEE_RANGE_X_DOO 1096.0F
#define ITREMIX_WADDLEDEE_RANGE_Y_DOO 600.0F
#define ITREMIX_WADDLEDEE_RANGE_EDGE 1280.0F
#define ITREMIX_WADDLEDEE_TARGET_MIN_STATUS 7

enum itRemixWaddleDeeStatus
{
    nITRemixWaddleDeeStatusNull,
    nITRemixWaddleDeeStatusAerial,
    nITRemixWaddleDeeStatusLanding,
    nITRemixWaddleDeeStatusWalk,
    nITRemixWaddleDeeStatusJumpsquat,
    nITRemixWaddleDeeStatusJump
};


typedef struct ITRemixWaddleDeeTarget
{
    GObj *gobj;
    f32 x_diff;
} ITRemixWaddleDeeTarget;

static sb32 itRemixWaddleDeeMinionDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags);
static void itRemixWaddleDeeInitSpawn(GObj *item_gobj, GObj *parent_gobj);
static void itRemixWaddleDeePinToHand(GObj *item_gobj, ITStruct *ip, ITRemixWaddleDeeVars *vars);
static void itRemixWaddleDeeBeginMain(GObj *item_gobj);
static void itRemixWaddleDeeLandingInitial(GObj *item_gobj);
static void itRemixWaddleDeeGroundedWalkInitial(GObj *item_gobj);
static void itRemixWaddleDeeGroundMoveInitial(GObj *item_gobj, sb32 is_right);
static void itRemixWaddleDeeThrowInitial(GObj *item_gobj, GObj *player_gobj);
static void itRemixWaddleDeeGroundToAirTransition(GObj *item_gobj);
static void itRemixWaddleDeeEdgeCheck(GObj *item_gobj, sb32 is_right);
static void itRemixWaddleDeeAttackInitial(GObj *item_gobj);
static void itRemixWaddleDeeJumpInitial(GObj *item_gobj);
static void itRemixWaddleDeeApplyAirSpeed(GObj *item_gobj, ITStruct *ip, ITRemixWaddleDeeVars *vars);
static void itRemixWaddleDeeUpdateFacing(ITStruct *ip);
static void itRemixWaddleDeeSlideGround(ITStruct *ip);
static void itRemixWaddleDeeUpdateRotationDirection(ITStruct *ip, ITRemixWaddleDeeVars *vars);
static void itRemixWaddleDeeApplyRotation(GObj *item_gobj, ITStruct *ip, ITRemixWaddleDeeVars *vars);
static void itRemixWaddleDeeTickHitboxRefresh(ITStruct *ip, ITRemixWaddleDeeVars *vars);
static sb32 itRemixWaddleDeeTickLifetime(GObj *item_gobj, ITStruct *ip);
static sb32 itRemixWaddleDeeJumpsquatMain(GObj *item_gobj);
static sb32 itRemixWaddleDeeGroundJumpsquatStep(GObj *item_gobj, ITStruct *ip);
static u8 itRemixWaddleDeeGetComboPort(GObj *attacker_gobj);
static void itRemixWaddleDeeCheckWalkEdge(GObj *item_gobj, ITStruct *ip);
static void itRemixWaddleDeeTryWalkAttack(GObj *item_gobj, ITStruct *ip);
static GObj* itRemixWaddleDeeCheckForTargets(ITStruct *ip, sb32 is_edge);
static void itRemixWaddleDeeScanFighters(GObj *owner_gobj, ITStruct *ip, sb32 is_edge, ITRemixWaddleDeeTarget *target);
static sb32 itRemixWaddleDeeIsTeamProtected(FTStruct *owner_fp, FTStruct *fp);
static void itRemixWaddleDeeScanItems(ITStruct *ip, GObj *owner_gobj, sb32 is_edge, ITRemixWaddleDeeTarget *target);
static void itRemixWaddleDeeCheckTarget(ITStruct *ip, GObj *target_gobj, sb32 is_edge, ITRemixWaddleDeeTarget *target);
static f32 itRemixWaddleDeeGetRangeX(ITStruct *ip, sb32 is_edge);
static f32 itRemixWaddleDeeGetRangeY(ITStruct *ip, sb32 is_edge);

static ITDesc sITRemixWaddleDeeItemInfo =
{
    ITREMIX_WADDLEDEE_KIND,                 /* Item.WaddleDee.id */
    NULL,                                   /* p_file, set from the owner before each spawn */
    ITREMIX_WADDLEDEE_FILE_OFFSET,          /* item footer offset, used as o_attributes */
    { nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0 },
    nGMAttackStateOff,
    itRemixWaddleDeeMinionNullMain,  /* 0x14: item_info_array runs into item_state_table, so the desc starts in state 0 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* Item.WaddleDee.item_state_table, indexed by status id */
static ITStatusDesc sITRemixWaddleDeeStatusDescs[] =
{
    /* STATE 0 - held by the owner */
    {
        itRemixWaddleDeeMinionNullMain,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* STATE 1 - aerial/main */
    {
        itRemixWaddleDeeWaddleDeeAerialMain,
        itRemixWaddleDeeWaddleDeeCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixWaddleDeeMinionHitboxCollision
    },

    /* STATE 2 - landing */
    {
        itRemixWaddleDeeWaddleDeeLandingMain,
        itRemixWaddleDeeWaddleDeeRestingCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixWaddleDeeMinionHitboxCollision
    },

    /* STATE 3 - walking */
    {
        itRemixWaddleDeeMinionWalkMain,
        itRemixWaddleDeeWaddleGroundCollision,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixWaddleDeeMinionHitboxCollision
    },

    /* STATE 4 - jump squat before the attack */
    {
        itRemixWaddleDeeJumpsquatMain,
        itRemixWaddleDeeWaddleGroundCollision,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixWaddleDeeMinionHitboxCollision
    },

    /* STATE 5 - jump */
    {
        itRemixWaddleDeeJumpsquatMain,
        itRemixWaddleDeeWaddleDeeCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        NULL,
        NULL,
        NULL,
        itRemixWaddleDeeMinionHitboxCollision
    }
};

/** Views the item-specific state of the Waddle Dee, which lives in ITStruct.item_vars. */
static ITRemixWaddleDeeVars* itRemixWaddleDeeGetVars(ITStruct *ip)
{
    return (ITRemixWaddleDeeVars*) &ip->item_vars;
}

/** Sets the image index and clears the animation pointers of the item's model. */
static void itRemixWaddleDeeResetImage(DObj *dobj, u16 image_index)
{
    MObj *mobj = dobj->mobj;

    mobj->texture_id_curr = image_index;
    mobj->aobj = NULL;
    mobj->matanim_joint.event16 = NULL;
}

/** Clears the hit object records, so the minion can hit the same targets again. */
static void itRemixWaddleDeeClearHitRecords(ITStruct *ip)
{
    s32 i;

    for (i = 0; i < ITREMIX_WADDLEDEE_HIT_RECORDS; i++)
    {
        ip->attack_coll.attack_records[i].victim_gobj = NULL;
    }
}

/** WaddleDee.stage_setting_: spawns a minion for `parent_gobj` from `item_desc`, which is copied so the item file pointer can be set. */
GObj* itRemixWaddleDeeSpawnMinion(GObj *parent_gobj, const ITDesc *item_desc, Vec3f *pos, Vec3f *vel)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITDesc spawn_desc = *item_desc;
    GObj *item_gobj;

    spawn_desc.p_file = ftRemixPowerData(fp)->p_file_special1;
    item_gobj = itManagerMakeItem(parent_gobj, &spawn_desc, pos, vel, FALSE);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixWaddleDeeInitSpawn(item_gobj, parent_gobj);

    return item_gobj;
}

/** WaddleDee.stage_setting_ (SPAWN_ITEM): spawns a Waddle Dee owned by `parent_gobj`. The Remix routine always passes flags 0 to the spawn, so the caller's flags are ignored. */
GObj* itRemixWaddleDeeStageSetting(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    return itRemixWaddleDeeSpawnMinion(parent_gobj, &sITRemixWaddleDeeItemInfo, pos, vel);
}

/** WaddleDee.stage_setting_ tail: owner, team, facing and vars of a freshly spawned minion. */
static void itRemixWaddleDeeInitSpawn(GObj *item_gobj, GObj *parent_gobj)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);
    DObj *dobj = DObjGetStruct(item_gobj);

    dobj->mobj->texture_id_curr = ITREMIX_WADDLEDEE_IMAGE_IDLE;
    dobj->translate.vec.f.z = 0.0F;
    ip->lifetime = 0;
    ip->multi = 0;
    gcAddXObjForDObjFixed(dobj, 0x2E, 0);

    ip->owner_gobj = parent_gobj;
    ip->player = fp->player;
    ip->team = fp->team;
    ip->handicap = fp->handicap;
    vars->player_gobj = parent_gobj;

    ip->damage_coll.hitstatus = FALSE;
    ip->attack_coll.attack_state = FALSE;
    vars->max_speed = ITREMIX_WADDLEDEE_MAX_SPEED;
    vars->rotation_direction = 0.0F;
    vars->hitbox_refresh_timer = 0;
    vars->is_hitbox_collided = FALSE;
    ip->proc_dead = itRemixWaddleDeeMinionBlastZone;
    ip->attack_coll.can_setoff = FALSE;

    itRemixWaddleDeeGroundMoveInitial(item_gobj, fp->lr == 1);
}

/** WaddleDee.minion_null_main_ (state 0): held by the owner, thrown by a timer flag, or destroyed. */
sb32 itRemixWaddleDeeMinionNullMain(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);

    if (ip->multi == 0)
    {
        itRemixWaddleDeePinToHand(item_gobj, ip, vars);
    }
    else if (ip->multi == 0xFFFF)
    {
        ip->kind = ITREMIX_WADDLEDEE_FAKE_KIND;
        return TRUE;
    }
    else
    {
        itRemixWaddleDeeThrowInitial(item_gobj, ip->owner_gobj);
        itRemixWaddleDeeRemoveMinionCheck(item_gobj);
    }
    ip->lifetime++;

    return FALSE;
}

/** WaddleDee.minion_null_main_ pin: keeps the minion at the owner's hand and flickers the image every 4 frames. */
static void itRemixWaddleDeePinToHand(GObj *item_gobj, ITStruct *ip, ITRemixWaddleDeeVars *vars)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    FTStruct *owner_fp = ftGetStruct(ip->owner_gobj);

    if ((ip->lifetime & 3) == 0)
    {
        dobj->mobj->texture_id_curr = (dobj->mobj->texture_id_curr == 0) ? ITREMIX_WADDLEDEE_IMAGE_IDLE : 0;
    }
    dobj->translate.vec.f.x = ITREMIX_WADDLEDEE_HOLD_OFFSET_X;
    dobj->translate.vec.f.y = 0.0F;
    dobj->translate.vec.f.z = 0.0F;
    gmCollisionGetFighterPartsWorldPosition(owner_fp->joints[ITREMIX_WADDLEDEE_HAND_JOINT], &dobj->translate.vec.f);

    dobj->rotate.vec.f.z = 0.0F;
    vars->rotation_direction = 0.0F;
}

/** WaddleDee.minion_throw_initial_: launches the minion from the owner's facing and stick angle. */
static void itRemixWaddleDeeThrowInitial(GObj *item_gobj, GObj *player_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);
    DObj *dobj = DObjGetStruct(item_gobj);
    FTStruct *fp = ftGetStruct(player_gobj);
    f32 angle;
    f32 speed = ITREMIX_WADDLEDEE_MAX_SPEED;

    dobj->translate.vec.f.z = 0.0F;
    dobj->translate.vec.f.y -= ITREMIX_WADDLEDEE_THROW_HEIGHT;
    dobj->mobj->texture_id_curr = ITREMIX_WADDLEDEE_IMAGE_IDLE;
    ip->attack_coll.can_rehit_shield = TRUE;
    ip->lifetime = ITREMIX_WADDLEDEE_DURATION;
    vars->throw_unknown = 4;

    angle = ITREMIX_WADDLEDEE_ANGLE + ((f32) fp->input.pl.stick_range.x * (f32) fp->lr * ITREMIX_WADDLEDEE_ANGLE_STICK_SCALE);

    ip->lr = fp->lr;
    ip->physics.vel_air.x = speed * __cosf(angle) * (f32) fp->lr;
    ip->physics.vel_air.y = speed * __sinf(angle);
    ip->physics.vel_air.z = 0.0F;
    ip->damage_coll.hitstatus = TRUE;
    ip->attack_coll.attack_state = TRUE;
    ip->is_damage_all = TRUE;

    itRemixWaddleDeeBeginMain(item_gobj);
    mpCommonRunItemCollisionDefault(item_gobj, fp->coll_data.p_translate, &fp->coll_data);
}

/** WaddleDee.remove_minion_check_: records the thrown minion in its owner's two field slots, retiring the oldest. */
void itRemixWaddleDeeRemoveMinionCheck(GObj *item_gobj)
{
    ftRemixDededeDSPAddFieldMinion(itGetStruct(item_gobj)->owner_gobj, item_gobj);
}

/** WaddleDee.minion_free_: removes the minion from its original owner's held and field slots. */
void itRemixWaddleDeeMinionFree(GObj *item_gobj)
{
    ftRemixDededeDSPFreeMinion(itRemixWaddleDeeGetVars(itGetStruct(item_gobj))->player_gobj, item_gobj);
}

/** WaddleDee.minion_blast_zone_: frees the minion. minion_free_ leaves v0 as the non-zero item struct pointer, so the proc reports TRUE and the item is destroyed. */
sb32 itRemixWaddleDeeMinionBlastZone(GObj *item_gobj)
{
    itRemixWaddleDeeMinionFree(item_gobj);

    return TRUE;
}

/** WaddleDee.minion_begin_main_: enters the aerial/main state with the hitbox on and pickup off. */
static void itRemixWaddleDeeBeginMain(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->is_allow_pickup = FALSE;
    ip->attack_coll.attack_state = TRUE;
    itMapSetAir(ip);
    ip->multi = 0;
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixWaddleDeeStatusDescs, nITRemixWaddleDeeStatusAerial);
}

/** WaddleDee.minion_landing_initial_: plays the step sfx and enters the landing state. */
static void itRemixWaddleDeeLandingInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);

    func_800269C0_275C0(ITREMIX_WADDLEDEE_FGM_LAND);
    dobj->rotate.vec.f.z = 0.0F;
    dobj->mobj->texture_id_curr = ITREMIX_WADDLEDEE_IMAGE_LANDING;

    ip->times_landed = 0;
    ip->multi = 0;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    itMapSetGround(ip);
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixWaddleDeeStatusDescs, nITRemixWaddleDeeStatusLanding);
}

/** WaddleDee.minion_grounded_walk_initial_: restores the original owner, stops the movement and starts walking. */
static void itRemixWaddleDeeGroundedWalkInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);

    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    ip->owner_gobj = vars->player_gobj;

    itRemixWaddleDeeMinionWalkInitial(item_gobj);
}

/** WaddleDee.minion_walk_intitial: sets up the walk, the bob-omb walk vars and the facing. */
void itRemixWaddleDeeMinionWalkInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    GObj *owner_gobj = ip->owner_gobj;

    ip->damage_coll.hitstatus = TRUE;
    ip->physics.vel_air.x = (f32) ip->lr * ITREMIX_WADDLEDEE_GROUND_SPEED;
    ip->event_id = 0;
    ip->multi = 0;
    ip->attack_coll.throw_mul = 1.0F;

    itBombHeiWalkInitVars(item_gobj);

    ip->attack_coll.attack_state = FALSE;
    ip->owner_gobj = owner_gobj;
    itMainSetStatus(item_gobj, sITRemixWaddleDeeStatusDescs, nITRemixWaddleDeeStatusWalk);
    itRemixWaddleDeeGroundMoveInitial(item_gobj, ip->lr == 1);
}

/** WaddleDee.ground_move_initial: walks in `is_right` direction at WALK_SPEED and selects the matching walk animation vector. */
static void itRemixWaddleDeeGroundMoveInitial(GObj *item_gobj, sb32 is_right)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);

    if (is_right)
    {
        ip->lr = 1;
        ip->physics.vel_air.x = ITREMIX_WADDLEDEE_GROUND_SPEED;
        dobj->dv = itGetPData(ip, ITREMIX_WADDLEDEE_ANIM_BASE, ITREMIX_WADDLEDEE_ANIM_RIGHT);
        return;
    }
    ip->lr = -1;
    ip->physics.vel_air.x = -ITREMIX_WADDLEDEE_GROUND_SPEED;
    dobj->dv = itGetPData(ip, ITREMIX_WADDLEDEE_ANIM_BASE, ITREMIX_WADDLEDEE_ANIM_LEFT);
}

/** WaddleDee.waddle_ground_to_air_transition: the ground collision with no floor left, going back to aerial. */
static void itRemixWaddleDeeGroundToAirTransition(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);

    itRemixWaddleDeeResetImage(dobj, ITREMIX_WADDLEDEE_IMAGE_THROWN);
    ip->ga = TRUE;
    ip->attack_coll.attack_state = TRUE;
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixWaddleDeeStatusDescs, nITRemixWaddleDeeStatusAerial);
}

/** WaddleDee.waddle_dee_aerial_main_ (state 1): flight, rotation and the hitbox refresh timer. */
sb32 itRemixWaddleDeeWaddleDeeAerialMain(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);

    itRemixWaddleDeeUpdateFacing(ip);
    if (ip->ga != FALSE)
    {
        itRemixWaddleDeeApplyAirSpeed(item_gobj, ip, vars);
    }
    else
    {
        itRemixWaddleDeeSlideGround(ip);
    }
    if (itRemixWaddleDeeTickLifetime(item_gobj, ip))
    {
        vars->is_hitbox_collided = FALSE;
        return TRUE;
    }
    itRemixWaddleDeeUpdateRotationDirection(ip, vars);
    if (ip->ga == FALSE)
    {
        itRemixWaddleDeeLandingInitial(item_gobj);
        vars->is_hitbox_collided = FALSE;
        return FALSE;
    }
    itRemixWaddleDeeApplyRotation(item_gobj, ip, vars);
    itRemixWaddleDeeTickHitboxRefresh(ip, vars);
    vars->is_hitbox_collided = FALSE;

    return FALSE;
}

/** Remix max speed decay: reduces max_speed by 1 per frame down to MAX_SPEED, then applies gravity. */
static void itRemixWaddleDeeApplyAirSpeed(GObj *item_gobj, ITStruct *ip, ITRemixWaddleDeeVars *vars)
{
    vars->max_speed -= ITREMIX_WADDLEDEE_AIR_SPEED_DECAY;
    if (vars->max_speed <= ITREMIX_WADDLEDEE_MAX_SPEED)
    {
        vars->max_speed = ITREMIX_WADDLEDEE_MAX_SPEED;
    }
    itMainApplyGravityClampTVel(ip, ITREMIX_WADDLEDEE_GRAVITY, vars->max_speed);
}

/** Facing update shared by the aerial and landing routines: lr follows the sign of the x velocity when it is non-zero. */
static void itRemixWaddleDeeUpdateFacing(ITStruct *ip)
{
    if (ip->physics.vel_air.x == 0.0F)
    {
        return;
    }
    ip->lr = (ip->physics.vel_air.x < 0.0F) ? -1 : 1;
}

/** Ground slide shared by the aerial and landing routines: decays the x velocity and stops the minion below the minimum speed. */
static void itRemixWaddleDeeSlideGround(ITStruct *ip)
{
    ip->physics.vel_air.x *= ITREMIX_WADDLEDEE_SLIDE_MULTIPLIER;
    if (fabsf(ip->physics.vel_air.x) < ITREMIX_WADDLEDEE_SLIDE_MIN_SPEED)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->attack_coll.attack_state = FALSE;
    }
}

/** Rotation direction follows the sign of the x velocity, kept while the velocity is zero. */
static void itRemixWaddleDeeUpdateRotationDirection(ITStruct *ip, ITRemixWaddleDeeVars *vars)
{
    if (ip->physics.vel_air.x == 0.0F)
    {
        return;
    }
    vars->rotation_direction = ip->physics.vel_air.x / fabsf(ip->physics.vel_air.x);
}

/** Spins the model around z by a step proportional to the absolute speed. */
static void itRemixWaddleDeeApplyRotation(GObj *item_gobj, ITStruct *ip, ITRemixWaddleDeeVars *vars)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    f32 abs_speed = sqrtf(ip->physics.vel_air.x * ip->physics.vel_air.x + ip->physics.vel_air.y * ip->physics.vel_air.y);
    f32 step = (ITREMIX_WADDLEDEE_ROTATION_BASE + (ITREMIX_WADDLEDEE_ROTATION * abs_speed)) * vars->rotation_direction;

    dobj->rotate.vec.f.z -= step;
}

/** Hitbox refresh timer: clears the hit records when the timer reaches zero. */
static void itRemixWaddleDeeTickHitboxRefresh(ITStruct *ip, ITRemixWaddleDeeVars *vars)
{
    if (vars->hitbox_refresh_timer == 0)
    {
        return;
    }
    vars->hitbox_refresh_timer--;
    if (vars->hitbox_refresh_timer != 0)
    {
        return;
    }
    itRemixWaddleDeeClearHitRecords(ip);
}

/** Duration countdown shared by every minion state: returns TRUE when the duration has ended and the minion must be freed. */
static sb32 itRemixWaddleDeeTickLifetime(GObj *item_gobj, ITStruct *ip)
{
    if (ip->lifetime != 0)
    {
        ip->lifetime--;
        return FALSE;
    }
    itRemixWaddleDeeMinionFree(item_gobj);

    return TRUE;
}

/** WaddleDee.waddle_dee_landing_main_ (state 2): slides to a stop, then starts walking after LAND_LAG frames. */
sb32 itRemixWaddleDeeWaddleDeeLandingMain(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);

    itRemixWaddleDeeUpdateFacing(ip);
    itRemixWaddleDeeSlideGround(ip);
    if (itRemixWaddleDeeTickLifetime(item_gobj, ip))
    {
        vars->is_hitbox_collided = FALSE;
        return TRUE;
    }
    ip->multi = (u16) (ip->multi + 1);
    if (ip->multi == ITREMIX_WADDLEDEE_LAND_LAG)
    {
        itRemixWaddleDeeGroundedWalkInitial(item_gobj);
    }
    vars->is_hitbox_collided = FALSE;

    return FALSE;
}

/** WaddleDee.waddle_dee_resting_collision_ (state 2 map): the wall check of the landing state, without floor. */
sb32 itRemixWaddleDeeWaddleDeeRestingCollision(GObj *item_gobj)
{
    itMapCheckLRWallProcNoFloor(item_gobj, itRemixWaddleDeeBeginMain);

    return FALSE;
}

/** WaddleDee.waddle_dee_collision_ (state 1 map): the aerial collision and bounce; lands on a floor line. */
sb32 itRemixWaddleDeeWaddleDeeCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    sb32 is_collided = mpProcessUpdateMain(&ip->coll_data, itRemixWaddleDeeMinionDetectCollision, item_gobj, ITREMIX_WADDLEDEE_COLL_MASK);

    itMapCheckCollideAllRebound(item_gobj, ITREMIX_WADDLEDEE_CHECK_COLL_MASK, ITREMIX_WADDLEDEE_BOUNCE, NULL);
    if ((is_collided == FALSE) || ((ip->coll_data.mask_stat & MAP_FLAG_FLOOR) == 0))
    {
        return FALSE;
    }
    itRemixWaddleDeeLandingInitial(item_gobj);
    ip->attack_coll.attack_state = TRUE;

    return FALSE;
}

/** Minion map callback (WaddleDee.detect_collision_): the vanilla bob-omb walls and ceiling, without the floor edge adjust and the floor landing sfx. Returns whether the map flags match `coll_flags`. */
static sb32 itRemixWaddleDeeMinionDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags)
{
    if (mpProcessCheckTestLWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunLWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestRWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunRWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestCeilCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunCeilCollisionAdjNew(coll_data);
        if ((coll_data->mask_stat & MAP_FLAG_CEIL) != 0)
        {
            mpProcessRunCeilEdgeAdjust(coll_data);
        }
    }
    if (mpProcessRunFloorCollisionAdjNewNULL(coll_data) != FALSE)
    {
        mpProcessSetLandingFloor(coll_data);
    }
    return (coll_data->mask_curr & coll_flags) != 0;
}

/** WaddleDee.minion_hurtbox_collision_ (hit): the minion is hit and enters the aerial state with a bounce. */
sb32 itRemixWaddleDeeMinionHurtboxCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);
    f32 abs_speed;
    f32 bounce_speed;
    sb32 was_hitbox_collided;
    s32 speed_int;
    s32 travel;

    itRemixWaddleDeeBeginMain(item_gobj);
    ip->attack_coll.attack_state = FALSE;

    abs_speed = sqrtf(ip->physics.vel_air.x * ip->physics.vel_air.x + ip->physics.vel_air.y * ip->physics.vel_air.y);
    bounce_speed = (abs_speed * 0.5F) + 5.0F;

    was_hitbox_collided = vars->is_hitbox_collided;
    vars->is_hitbox_collided = FALSE;
    if (was_hitbox_collided == FALSE)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->physics.vel_air.y = bounce_speed;
    }
    vars->hitbox_refresh_timer = 0;

    speed_int = (s32) rintf(abs_speed);
    travel = speed_int + (speed_int >> 1);
    ip->lifetime -= travel;
    if (ip->lifetime < ITREMIX_WADDLEDEE_LANDING_DURATION_MIN)
    {
        ip->lifetime = ITREMIX_WADDLEDEE_LANDING_DURATION_MIN;
    }
    return FALSE;
}

/** WaddleDee.minion_hitbox_collision_ (damage): the hitbox of the minion's owner launches the minion. */
sb32 itRemixWaddleDeeMinionHitboxCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);
    DObj *dobj = DObjGetStruct(item_gobj);
    GObj *attacker_gobj = ip->damage_gobj;
    f32 knockback;
    f32 angle;
    s32 direction;
    s32 travel;

    itRemixWaddleDeeBeginMain(item_gobj);
    itRemixWaddleDeeResetImage(dobj, ITREMIX_WADDLEDEE_IMAGE_THROWN);
    ip->ga = TRUE;
    ip->owner_gobj = attacker_gobj;
    ip->player = itRemixWaddleDeeGetComboPort(attacker_gobj);

    knockback = ((f32) ip->damage_queue * 4.0F) + 10.0F;
    vars->max_speed = knockback;
    angle = ftCommonDamageGetKnockbackAngle(ip->damage_angle, ip->ga, knockback);
    direction = -ip->damage_lr;
    ip->physics.vel_air.x = (f32) direction * knockback * __cosf(angle);
    ip->physics.vel_air.y = knockback * __sinf(angle);

    vars->hitbox_refresh_timer = ITREMIX_WADDLEDEE_HITBOX_REFRESH_FRAMES;
    vars->is_hitbox_collided = TRUE;

    travel = ip->damage_queue * ITREMIX_WADDLEDEE_HIT_DURATION_MUL;
    ip->lifetime -= travel;
    if (ip->lifetime < ITREMIX_WADDLEDEE_HIT_DURATION_MIN)
    {
        ip->lifetime = ITREMIX_WADDLEDEE_HIT_DURATION_MIN;
    }
    return FALSE;
}

/** Combo ownership port of the hitting object: its port when it is a fighter, otherwise ITREMIX_WADDLEDEE_NO_COMBO_PORT. */
static u8 itRemixWaddleDeeGetComboPort(GObj *attacker_gobj)
{
    if ((attacker_gobj == NULL) || (attacker_gobj->id != nGCCommonKindFighter))
    {
        return ITREMIX_WADDLEDEE_NO_COMBO_PORT;
    }
    return ftGetStruct(attacker_gobj)->player;
}

/** WaddleDee.waddle_ground_collision (state 3 and 4 map): wall turnaround, and the transition to aerial when the floor is gone. */
sb32 itRemixWaddleDeeWaddleGroundCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itMapCheckLRWallProcNoFloor(item_gobj, itRemixWaddleDeeGroundToAirTransition);
    if ((ip->coll_data.mask_curr & MAP_FLAG_LWALL) != 0)
    {
        itBombHeiCommonSetWalkLR(item_gobj, 0);
    }
    if ((ip->coll_data.mask_curr & MAP_FLAG_RWALL) != 0)
    {
        itBombHeiCommonSetWalkLR(item_gobj, 1);
    }
    return FALSE;
}

/** WaddleDee.walk_main (state 3): walks, turns at floor edges and attacks while a target is near. */
sb32 itRemixWaddleDeeMinionWalkMain(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    sb32 is_destroyed;

    if (mpCollisionCheckExistLineID(ip->coll_data.floor_line_id) != FALSE)
    {
        itRemixWaddleDeeCheckWalkEdge(item_gobj, ip);
    }
    if ((ip->multi & 0xF) == 0)
    {
        itRemixWaddleDeeTryWalkAttack(item_gobj, ip);
    }
    is_destroyed = itRemixWaddleDeeTickLifetime(item_gobj, ip);
    ip->multi = (u16) (ip->multi + 1);

    return is_destroyed;
}

/** Walk edge check: turns around when the floor edge ahead is within the minion's half-width. */
static void itRemixWaddleDeeCheckWalkEdge(GObj *item_gobj, ITStruct *ip)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    Vec3f edge_pos;
    f32 half_width = (f32) ip->attr->map_coll_width;

    if (ip->lr == -1)
    {
        mpCollisionGetFloorEdgeL(ip->coll_data.floor_line_id, &edge_pos);
        if ((dobj->translate.vec.f.x - half_width) <= edge_pos.x)
        {
            itRemixWaddleDeeEdgeCheck(item_gobj, TRUE);
        }
        return;
    }
    mpCollisionGetFloorEdgeR(ip->coll_data.floor_line_id, &edge_pos);
    if (edge_pos.x <= (dobj->translate.vec.f.x + half_width))
    {
        itRemixWaddleDeeEdgeCheck(item_gobj, FALSE);
    }
}

/** WaddleDee.waddle_edge_check_: turns to `is_right` when no target is ahead. */
static void itRemixWaddleDeeEdgeCheck(GObj *item_gobj, sb32 is_right)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (itRemixWaddleDeeCheckForTargets(ip, TRUE) != NULL)
    {
        return;
    }
    itRemixWaddleDeeGroundMoveInitial(item_gobj, is_right);
}

/** Every 16 frames of the walk timer: checks for a target and starts the attack after the attack delay. */
static void itRemixWaddleDeeTryWalkAttack(GObj *item_gobj, ITStruct *ip)
{
    u16 next_timer = (u16) (ip->multi + 1);
    u16 attack_delay = (ip->kind == ITREMIX_WADDLEDEE_KIND) ? ITREMIX_WADDLEDEE_ATTACK_DELAY : ITREMIX_WADDLEDOO_ATTACK_DELAY;

    if (next_timer < attack_delay)
    {
        return;
    }
    if (itRemixWaddleDeeCheckForTargets(ip, FALSE) == NULL)
    {
        return;
    }
    if (ip->kind == ITREMIX_WADDLEDEE_KIND)
    {
        itRemixWaddleDeeAttackInitial(item_gobj);
    }
    else
    {
        itRemixWaddleDooAttackInitial(item_gobj);
    }
    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
}

/** WaddleDee.waddle_dee_attack_initial: starts the jump squat before the jump attack. */
static void itRemixWaddleDeeAttackInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);

    dobj->rotate.vec.f.z = 0.0F;
    itRemixWaddleDeeResetImage(dobj, ITREMIX_WADDLEDEE_IMAGE_JUMPSQUAT);
    ip->attack_coll.attack_state = FALSE;
    ip->times_landed = 0;
    ip->multi = 0;
    itMapSetGround(ip);
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixWaddleDeeStatusDescs, nITRemixWaddleDeeStatusJumpsquat);
}

/** WaddleDee.waddle_dee_jumpsquat_main (states 4 and 5): squats on the ground, then jumps; in the air it keeps the aerial speed. */
static sb32 itRemixWaddleDeeJumpsquatMain(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDeeVars *vars = itRemixWaddleDeeGetVars(ip);

    if (ip->ga != FALSE)
    {
        itRemixWaddleDeeApplyAirSpeed(item_gobj, ip, vars);
    }
    else
    {
        if (itRemixWaddleDeeGroundJumpsquatStep(item_gobj, ip))
        {
            vars->is_hitbox_collided = FALSE;
            return FALSE;
        }
    }
    if (itRemixWaddleDeeTickLifetime(item_gobj, ip))
    {
        vars->is_hitbox_collided = FALSE;
        return TRUE;
    }
    itRemixWaddleDeeTickHitboxRefresh(ip, vars);
    vars->is_hitbox_collided = FALSE;

    return FALSE;
}

/** Grounded branch of the jump squat: counts the land lag, then jumps or lands. Returns TRUE when the branch ends the frame. */
static sb32 itRemixWaddleDeeGroundJumpsquatStep(GObj *item_gobj, ITStruct *ip)
{
    ip->multi = (u16) (ip->multi + 1);
    if (ip->multi > ITREMIX_WADDLEDEE_LAND_LAG)
    {
        itRemixWaddleDeeLandingInitial(item_gobj);
        return TRUE;
    }
    if (ip->multi != ITREMIX_WADDLEDEE_LAND_LAG)
    {
        return FALSE;
    }
    itRemixWaddleDeeJumpInitial(item_gobj);

    return TRUE;
}

/** WaddleDee.jump_initial: launches the jump toward the facing and enters the jump state. */
static void itRemixWaddleDeeJumpInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);

    func_800269C0_275C0(ITREMIX_WADDLEDEE_FGM_JUMP);
    dobj->mobj->texture_id_curr = ITREMIX_WADDLEDEE_IMAGE_JUMP;
    ip->attack_coll.attack_state = TRUE;
    ip->physics.vel_air.x = ITREMIX_WADDLEDEE_JUMP_SPEED_X * (f32) ip->lr;
    ip->physics.vel_air.y = ITREMIX_WADDLEDEE_JUMP_SPEED_Y;
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    ip->ga = TRUE;
    itMainSetStatus(item_gobj, sITRemixWaddleDeeStatusDescs, nITRemixWaddleDeeStatusJump);
}

/** Finds the nearest valid target for the walk and edge checks. Returns the target GObj, or NULL when none is in range. */
static GObj* itRemixWaddleDeeCheckForTargets(ITStruct *ip, sb32 is_edge)
{
    ITRemixWaddleDeeTarget target = { NULL, 0.0F };
    GObj *owner_gobj = ip->owner_gobj;

    if (owner_gobj != NULL)
    {
        itRemixWaddleDeeScanFighters(owner_gobj, ip, is_edge, &target);
    }
    if (target.gobj == NULL)
    {
        itRemixWaddleDeeScanItems(ip, owner_gobj, is_edge, &target);
    }
    return target.gobj;
}

/** Checks every fighter except the owner, skipping team mates when team attack is off and fighters in a KO status. */
static void itRemixWaddleDeeScanFighters(GObj *owner_gobj, ITStruct *ip, sb32 is_edge, ITRemixWaddleDeeTarget *target)
{
    FTStruct *owner_fp = ftGetStruct(owner_gobj);
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

    for (; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        if (fighter_gobj == owner_gobj)
        {
            continue;
        }
        if (itRemixWaddleDeeIsTeamProtected(owner_fp, fp))
        {
            continue;
        }
        if (fp->status_id < ITREMIX_WADDLEDEE_TARGET_MIN_STATUS)
        {
            continue;
        }
        itRemixWaddleDeeCheckTarget(ip, fighter_gobj, is_edge, target);
    }
}

/** Team battle with team attack off: a fighter on the owner's team is not a target. */
static sb32 itRemixWaddleDeeIsTeamProtected(FTStruct *owner_fp, FTStruct *fp)
{
    if (gSCManagerBattleState->is_team_battle == FALSE)
    {
        return FALSE;
    }
    if (gSCManagerBattleState->is_team_attack != FALSE)
    {
        return FALSE;
    }
    return fp->team == owner_fp->team;
}

/** Checks every item with an active hurtbox that is not owned by the same owner. */
static void itRemixWaddleDeeScanItems(ITStruct *ip, GObj *owner_gobj, sb32 is_edge, ITRemixWaddleDeeTarget *target)
{
    GObj *item_gobj = gGCCommonLinks[nGCCommonLinkIDItem];

    for (; item_gobj != NULL; item_gobj = item_gobj->link_next)
    {
        ITStruct *target_ip = itGetStruct(item_gobj);

        if (target_ip == ip)
        {
            continue;
        }
        if ((target_ip->damage_coll.hitstatus & 0x1) == 0)
        {
            continue;
        }
        if (target_ip->owner_gobj == owner_gobj)
        {
            continue;
        }
        itRemixWaddleDeeCheckTarget(ip, item_gobj, is_edge, target);
    }
}

/** Range test for a candidate target. Keeps the target when it is in front of the minion and closer than the stored one. */
static void itRemixWaddleDeeCheckTarget(ITStruct *ip, GObj *target_gobj, sb32 is_edge, ITRemixWaddleDeeTarget *target)
{
    DObj *minion_dobj = DObjGetStruct(ip->item_gobj);
    DObj *target_dobj = DObjGetStruct(target_gobj);
    f32 x_diff = (target_dobj->translate.vec.f.x - minion_dobj->translate.vec.f.x) * (f32) ip->lr;
    f32 y_diff = fabsf(target_dobj->translate.vec.f.y - minion_dobj->translate.vec.f.y);
    f32 y_range = itRemixWaddleDeeGetRangeY(ip, is_edge) + (x_diff * 0.5F);

    if ((x_diff > itRemixWaddleDeeGetRangeX(ip, is_edge)) || (x_diff < 0.0F))
    {
        return;
    }
    if ((target->gobj != NULL) && (x_diff > target->x_diff))
    {
        return;
    }
    if (y_diff > y_range)
    {
        return;
    }
    target->gobj = target_gobj;
    target->x_diff = x_diff;
}

/** Horizontal target range: the edge range for an edge check, 1096 for Waddle Doo, otherwise 600. */
static f32 itRemixWaddleDeeGetRangeX(ITStruct *ip, sb32 is_edge)
{
    if (is_edge != FALSE)
    {
        return ITREMIX_WADDLEDEE_RANGE_EDGE;
    }
    return (ip->kind == ITREMIX_WADDLEDOO_KIND) ? ITREMIX_WADDLEDEE_RANGE_X_DOO : ITREMIX_WADDLEDEE_RANGE_X;
}

/** Vertical target range: the edge range for an edge check, 600 for Waddle Doo, otherwise 700. */
static f32 itRemixWaddleDeeGetRangeY(ITStruct *ip, sb32 is_edge)
{
    if (is_edge != FALSE)
    {
        return ITREMIX_WADDLEDEE_RANGE_EDGE;
    }
    return (ip->kind == ITREMIX_WADDLEDOO_KIND) ? ITREMIX_WADDLEDEE_RANGE_Y_DOO : ITREMIX_WADDLEDEE_RANGE_Y;
}

/* Descriptor for the Remix item table (WaddleDee.asm: no pickup, drop, throw or touch routines). */
const ITRemixItemDesc dITRemixWaddleDeeItemDesc =
{
    ITREMIX_WADDLEDEE_KIND,         /* Item.WaddleDee.id */
    itRemixWaddleDeeStageSetting,   /* SPAWN_ITEM */
    FALSE,                          /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                           /* PICKUP_ITEM_INIT */
    NULL,                           /* PICKUP_ITEM_MAIN */
    NULL,                           /* DROP_ITEM */
    NULL,                           /* THROW_ITEM */
    NULL                            /* PLAYER_COLLISION */
};

#endif
