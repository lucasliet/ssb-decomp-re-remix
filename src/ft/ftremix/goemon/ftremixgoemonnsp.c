/**
 * ftremixgoemonnsp.c - C ports of Smash Remix Goemon neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Goemon/GoemonSpecial.asm, scope GoemonNSP)
 * and the Ryo coin toss projectile they launch.
 *
 * The Ryo weapon descriptors are built at run time because their file pointer is the fighter's
 * special file (fp->data->p_file_special1), so they cannot be static const tables.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <sys/obj.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include <wp/wpdef.h>
#include <wp/wpmain.h>
#include <wp/wpmanager.h>
#include <wp/wpmap.h>

enum
{
    nFTRemixGoemonStatusNSPGroundBegin = 0xE5,
    nFTRemixGoemonStatusNSPGroundWait = 0xE6,
    nFTRemixGoemonStatusNSPGroundWalk1 = 0xE7,
    nFTRemixGoemonStatusNSPGroundWalk2 = 0xE8,
    nFTRemixGoemonStatusNSPGroundBWalk1 = 0xE9,
    nFTRemixGoemonStatusNSPGroundBWalk2 = 0xEA,
    nFTRemixGoemonStatusNSPGroundEnd = 0xEB,
    nFTRemixGoemonStatusNSPAirBegin = 0xEC,
    nFTRemixGoemonStatusNSPAirWait = 0xED,
    nFTRemixGoemonStatusNSPAirEnd = 0xEE
};

/* Kirby and J Kirby use these ids for the same specials. Kirby.asm add_new_action order: 0xDC + 83 + index. */
enum
{
    nFTRemixGoemonKirbyStatusNSPGroundBegin = 0x189,
    nFTRemixGoemonKirbyStatusNSPGroundWait = 0x18A,
    nFTRemixGoemonKirbyStatusNSPGroundWalk1 = 0x18B,
    nFTRemixGoemonKirbyStatusNSPGroundWalk2 = 0x18C,
    nFTRemixGoemonKirbyStatusNSPGroundBWalk1 = 0x18D,
    nFTRemixGoemonKirbyStatusNSPGroundBWalk2 = 0x18E,
    nFTRemixGoemonKirbyStatusNSPGroundEnd = 0x18F,
    nFTRemixGoemonKirbyStatusNSPAirBegin = 0x190,
    nFTRemixGoemonKirbyStatusNSPAirWait = 0x191,
    nFTRemixGoemonKirbyStatusNSPAirEnd = 0x192
};

#define GOEMON_NSP_CHARGE_TIME 18
#define GOEMON_NSP_SFX_CHARGE_1 28
#define GOEMON_NSP_SFX_CHARGE_2 65
#define GOEMON_NSP_SFX_FIRE 27
#define GOEMON_NSP_SFX_EXPLODE 0
#define GOEMON_NSP_JOINT_RIGHT_HAND 16           /* FTStruct.joints[16] at N64 0x0928 (part 0xC) */
#define GOEMON_NSP_SPAWN_Y_OFFSET 30.0F          /* 0x41F00000 */
#define GOEMON_NSP_WALK_MULTIPLIER 0.25F         /* 0x3E800000 */
#define GOEMON_NSP_WALK_TRACTION 30.0F           /* 0x41F00000 */
#define GOEMON_NSP_AIR_SPEED 28.0F               /* 0x41E00000 */
#define GOEMON_NSP_AIR_CONTROL_STICK_MIN 8
#define GOEMON_NSP_CONTINUE_FLAGS (FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX) /* 0x0802 */
#define GOEMON_RYO_PROJECTILE_ID 0x22            /* Projectile.id.GOEMON_RYO (bass.log) */
#define GOEMON_RYO_TRANSFORM_TK2 0x45
#define GOEMON_RYO_EFFECT_TRANSFORM_TK2 0x48
#define GOEMON_RYO_EFFECT_FIRE_OFFSET 0x80       /* offset to hitbox of fire_effect_projectile_struct */
#define GOEMON_RYO_EFFECT_SPARKLE_OFFSET 0xC0    /* offset to hitbox of sparkle_effect_projectile_struct */
#define GOEMON_RYO_BASE_ANGLE_BITS 0x3FC90FDB   /* 90 degrees, as float bits */
#define GOEMON_RYO_SIGN_BIT 0x80000000

/* Walk status slots, from the most backward stick range to the most forward. */
#define GOEMON_NSP_WALK_SLOT_BWALK2 0
#define GOEMON_NSP_WALK_SLOT_BWALK1 1
#define GOEMON_NSP_WALK_SLOT_WAIT 2
#define GOEMON_NSP_WALK_SLOT_WALK1 3
#define GOEMON_NSP_WALK_SLOT_WALK2 4

/* Per-fighter charge state, which Remix keeps in the status variables (N64 0x0B18, 0x0B1C). */
typedef struct FTRemixGoemonChargeVars
{
    s32 charge_timer;
    s32 charge_flag;
} FTRemixGoemonChargeVars;

/* Per-projectile Ryo state, kept in the weapon-specific union (N64 0x01B4, 0x01B8 in the Remix build). */
typedef struct FTRemixGoemonRyoVars
{
    s32 charge_flag;
    GObj *linked_gobj;
} FTRemixGoemonRyoVars;

typedef struct FTRemixGoemonRyoProperties
{
    s32 duration;
    f32 max_speed;
    f32 gravity;
    f32 rotation_speed;
    f32 initial_angle_ground;
    f32 initial_angle_air;
    f32 initial_speed;
    intptr_t hitbox_offset;
} FTRemixGoemonRyoProperties;

typedef union FTRemixGoemonFloatBits
{
    f32 f;
    u32 u;
} FTRemixGoemonFloatBits;

static const f32 sFTRemixGoemonCycleLength[5] = { 40.0F, 80.0F, 60.0F, 80.0F, 60.0F };

static const s32 sFTRemixGoemonGroundWalkStatus[5] =
{
    nFTRemixGoemonStatusNSPGroundBWalk2,
    nFTRemixGoemonStatusNSPGroundBWalk1,
    nFTRemixGoemonStatusNSPGroundWait,
    nFTRemixGoemonStatusNSPGroundWalk1,
    nFTRemixGoemonStatusNSPGroundWalk2
};

static const s32 sFTRemixGoemonKirbyGroundWalkStatus[5] =
{
    nFTRemixGoemonKirbyStatusNSPGroundBWalk2,
    nFTRemixGoemonKirbyStatusNSPGroundBWalk1,
    nFTRemixGoemonKirbyStatusNSPGroundWait,
    nFTRemixGoemonKirbyStatusNSPGroundWalk1,
    nFTRemixGoemonKirbyStatusNSPGroundWalk2
};

/* Indexed by current ground status minus nFTRemixGoemonStatusNSPGroundBegin. */
static const s32 sFTRemixGoemonGroundToAirStatus[7] =
{
    nFTRemixGoemonStatusNSPAirBegin,
    nFTRemixGoemonStatusNSPAirWait,
    nFTRemixGoemonStatusNSPAirWait,
    nFTRemixGoemonStatusNSPAirWait,
    nFTRemixGoemonStatusNSPAirWait,
    nFTRemixGoemonStatusNSPAirWait,
    nFTRemixGoemonStatusNSPAirEnd
};

/* Indexed by current air status minus nFTRemixGoemonStatusNSPAirBegin. */
static const s32 sFTRemixGoemonAirToGroundStatus[3] =
{
    nFTRemixGoemonStatusNSPGroundBegin,
    nFTRemixGoemonStatusNSPGroundWait,
    nFTRemixGoemonStatusNSPGroundEnd
};

/* Indexed by current Kirby ground status minus nFTRemixGoemonKirbyStatusNSPGroundBegin (kirby_ground_to_air_table). */
static const s32 sFTRemixGoemonKirbyGroundToAirStatus[7] =
{
    nFTRemixGoemonKirbyStatusNSPAirBegin,
    nFTRemixGoemonKirbyStatusNSPAirWait,
    nFTRemixGoemonKirbyStatusNSPAirWait,
    nFTRemixGoemonKirbyStatusNSPAirWait,
    nFTRemixGoemonKirbyStatusNSPAirWait,
    nFTRemixGoemonKirbyStatusNSPAirWait,
    nFTRemixGoemonKirbyStatusNSPAirEnd
};

/* Indexed by current Kirby air status minus nFTRemixGoemonKirbyStatusNSPAirBegin (kirby_air_to_ground_table). */
static const s32 sFTRemixGoemonKirbyAirToGroundStatus[3] =
{
    nFTRemixGoemonKirbyStatusNSPGroundBegin,
    nFTRemixGoemonKirbyStatusNSPGroundWait,
    nFTRemixGoemonKirbyStatusNSPGroundEnd
};

/* N64 ryo_properties_struct and fire_ryo_properties_struct. The asm's min speed, bounce, file and palette fields are not used. */
static const FTRemixGoemonRyoProperties sFTRemixGoemonRyoProperties =
{
    .duration = 40,
    .max_speed = 100.0F,
    .gravity = 0.0F,
    .rotation_speed = 0.3F,
    .initial_angle_ground = 0.0F,
    .initial_angle_air = 0.0F,
    .initial_speed = 100.0F,
    .hitbox_offset = 0x00000000
};

static const FTRemixGoemonRyoProperties sFTRemixGoemonFireRyoProperties =
{
    .duration = 60,
    .max_speed = 130.0F,
    .gravity = 0.0F,
    .rotation_speed = 0.5F,
    .initial_angle_ground = 0.0F,
    .initial_angle_air = 0.0F,
    .initial_speed = 130.0F,
    .hitbox_offset = 0x00000040
};

extern alSoundEffect *func_800269C0_275C0(u16 sfx_id);
extern sb32 wpSamusBombProcReflector(GObj *weapon_gobj);
extern sb32 wpMarioFireballProcHop(GObj *weapon_gobj);

static sb32 ftRemixGoemonIsKirbyKind(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Kirby and J Kirby get their own status ids for the same special. */
static s32 ftRemixGoemonNSPStatusForFighter(FTStruct *fp, s32 goemon_status, s32 kirby_status)
{
    if (ftRemixGoemonIsKirbyKind(fp))
    {
        return kirby_status;
    }
    return goemon_status;
}

/* Sets the Goemon or Kirby status id, then plays the anim events, as the *_initial_ routines do. */
static void ftRemixGoemonNSPSetStatus(GObj *fighter_gobj, s32 goemon_status, s32 kirby_status, u32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixGoemonNSPStatusForFighter(fp, goemon_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, flags);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Clears temp variables 1 to 3 (motion_vars.flags) that the begin routines reset before the moveset runs. */
static void ftRemixGoemonNSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

static FTRemixGoemonChargeVars *ftRemixGoemonChargeVars(FTStruct *fp)
{
    return (FTRemixGoemonChargeVars *) &fp->status_vars;
}

static FTRemixGoemonRyoVars *ftRemixGoemonRyoVars(GObj *ryo_gobj)
{
    WPStruct *wp = wpGetStruct(ryo_gobj);

    return (FTRemixGoemonRyoVars *) &wp->weapon_vars;
}

static f32 ftRemixGoemonFloatFromBits(u32 bits)
{
    FTRemixGoemonFloatBits value;

    value.u = bits;
    return value.f;
}

static u32 ftRemixGoemonFloatToBits(f32 value)
{
    FTRemixGoemonFloatBits bits;

    bits.f = value;
    return bits.u;
}

/* ryo_rotation_: the Ryo's facing angle is +/-90 degrees, with the sign taken from the x velocity. */
static f32 ftRemixGoemonRyoFacingAngle(f32 vel_x)
{
    u32 sign_bit = ftRemixGoemonFloatToBits(vel_x) & GOEMON_RYO_SIGN_BIT;

    return ftRemixGoemonFloatFromBits(sign_bit | GOEMON_RYO_BASE_ANGLE_BITS);
}

/* ryo_rotation_: sets the Ryo's rotation and, when it has an effect object, the effect's y rotation. */
static void ftRemixGoemonRyoRotate(GObj *ryo_gobj)
{
    WPStruct *wp = wpGetStruct(ryo_gobj);
    FTRemixGoemonRyoVars *vars = ftRemixGoemonRyoVars(ryo_gobj);
    DObj *dobj = DObjGetStruct(ryo_gobj);
    f32 facing = ftRemixGoemonRyoFacingAngle(wp->physics.vel_air.x);

    if (vars->linked_gobj != NULL)
    {
        DObjGetStruct(vars->linked_gobj)->rotate.vec.f.y = facing;
    }
    dobj->rotate.vec.f.x = facing;
    dobj->rotate.vec.f.y = 0.0F;
    dobj->rotate.vec.f.z = ftRemixGoemonFloatFromBits(GOEMON_RYO_BASE_ANGLE_BITS);
}

/* Sets the linked effect object's destruction flag, which ends the sparkle or fire effect with the Ryo. */
static void ftRemixGoemonRyoEndEffect(FTRemixGoemonRyoVars *vars)
{
    if (vars->linked_gobj != NULL)
    {
        ftRemixGoemonRyoVars(vars->linked_gobj)->charge_flag = TRUE;
    }
}

/* Smoke for a normal Ryo, and the explosion plus FGM 0 for a charged one. */
static void ftRemixGoemonRyoBurst(GObj *ryo_gobj, sb32 is_charged)
{
    DObj *dobj = DObjGetStruct(ryo_gobj);

    if (is_charged)
    {
        /* asm passes the DObj address here; the port uses the translate position. See notes. */
        efManagerSparkleWhiteMakeEffect(&dobj->translate.vec.f);
        func_800269C0_275C0(GOEMON_NSP_SFX_EXPLODE);
        return;
    }
    efManagerDustExpandSmallMakeEffect(&dobj->translate.vec.f, 1.0F);
}

/* ryo_destruction_: destroys the Ryo with its burst. Used by the hurtbox, shield, hitbox, absorb and expire paths. */
static sb32 ftRemixGoemonRyoDestruction(GObj *ryo_gobj)
{
    FTRemixGoemonRyoVars *vars = ftRemixGoemonRyoVars(ryo_gobj);

    ftRemixGoemonRyoEndEffect(vars);
    ftRemixGoemonRyoBurst(ryo_gobj, vars->charge_flag != FALSE);
    return TRUE;
}

/* ryo_main_: the Ryo's per-frame update (proc_update). Returns TRUE when the Ryo is destroyed. */
static sb32 ftRemixGoemonRyoMain(GObj *ryo_gobj)
{
    WPStruct *wp = wpGetStruct(ryo_gobj);
    FTRemixGoemonRyoVars *vars = ftRemixGoemonRyoVars(ryo_gobj);
    const FTRemixGoemonRyoProperties *props = (vars->charge_flag != FALSE) ? &sFTRemixGoemonFireRyoProperties : &sFTRemixGoemonRyoProperties;
    DObj *dobj = DObjGetStruct(ryo_gobj);

    if (wpMainDecLifeCheckExpire(wp))
    {
        return ftRemixGoemonRyoDestruction(ryo_gobj);
    }
    wpMainApplyGravityClampTVel(wp, props->gravity, props->max_speed);
    dobj->rotate.vec.f.y -= props->rotation_speed;
    return FALSE;
}

/* ryo_collision_: destroys the Ryo when the map collision test hits. */
static sb32 ftRemixGoemonRyoProcMap(GObj *ryo_gobj)
{
    if (wpMapTestAllCheckCollEnd(ryo_gobj))
    {
        return ftRemixGoemonRyoDestruction(ryo_gobj);
    }
    return FALSE;
}

/* ryo_reflect_: the Samus bomb reflector, then the Ryo's rotation is updated. */
static sb32 ftRemixGoemonRyoReflect(GObj *ryo_gobj)
{
    wpSamusBombProcReflector(ryo_gobj);
    ftRemixGoemonRyoRotate(ryo_gobj);
    return FALSE;
}

/* ryo_shield_bounce_: the Mario fireball hop, then the Ryo's rotation is updated. */
static sb32 ftRemixGoemonRyoShieldBounce(GObj *ryo_gobj)
{
    wpMarioFireballProcHop(ryo_gobj);
    ftRemixGoemonRyoRotate(ryo_gobj);
    return FALSE;
}

/* effect_projectile_main_: follows the Ryo's translate, and destroys itself once the Ryo is gone. */
static sb32 ftRemixGoemonEffectMain(GObj *effect_gobj)
{
    FTRemixGoemonRyoVars *vars = ftRemixGoemonRyoVars(effect_gobj);
    DObj *ryo_dobj;

    if (vars->charge_flag != FALSE)
    {
        return TRUE;
    }
    if (vars->linked_gobj == NULL)
    {
        return TRUE;
    }
    ryo_dobj = DObjGetStruct(vars->linked_gobj);
    if (ryo_dobj == NULL)
    {
        return TRUE;
    }
    DObjGetStruct(effect_gobj)->translate.vec.f = ryo_dobj->translate.vec.f;
    return FALSE;
}

/* Ryo weapon descriptor fields shared by the Ryo and its effect. */
static void ftRemixGoemonFillDescBase(WPDesc *desc, FTStruct *fp, intptr_t o_attributes, u8 transform_tk2)
{
    desc->flags = 0;
    desc->kind = GOEMON_RYO_PROJECTILE_ID;
    desc->p_weapon = fp->data->p_file_special1;
    desc->o_attributes = o_attributes;
    desc->transform_types.tk1 = nGCMatrixKindTra;
    desc->transform_types.tk2 = transform_tk2;
    desc->transform_types.tk3 = 0;
}

/* ryo_projectile_struct: the Ryo's procs, with the shield bounce and reflector in their N64 slots. */
static void ftRemixGoemonFillRyoDesc(WPDesc *desc, FTStruct *fp, intptr_t hitbox_offset)
{
    ftRemixGoemonFillDescBase(desc, fp, hitbox_offset, GOEMON_RYO_TRANSFORM_TK2);
    desc->proc_update = ftRemixGoemonRyoMain;
    desc->proc_map = ftRemixGoemonRyoProcMap;
    desc->proc_hit = ftRemixGoemonRyoDestruction;
    desc->proc_shield = ftRemixGoemonRyoDestruction;
    desc->proc_hop = ftRemixGoemonRyoShieldBounce;
    desc->proc_setoff = ftRemixGoemonRyoDestruction;
    desc->proc_reflector = ftRemixGoemonRyoReflect;
    desc->proc_absorb = ftRemixGoemonRyoDestruction;
}

/* fire_effect_projectile_struct and sparkle_effect_projectile_struct: only the main proc runs. */
static void ftRemixGoemonFillEffectDesc(WPDesc *desc, FTStruct *fp, intptr_t hitbox_offset)
{
    ftRemixGoemonFillDescBase(desc, fp, hitbox_offset, GOEMON_RYO_EFFECT_TRANSFORM_TK2);
    desc->proc_update = ftRemixGoemonEffectMain;
    desc->proc_map = NULL;
    desc->proc_hit = NULL;
    desc->proc_shield = NULL;
    desc->proc_hop = NULL;
    desc->proc_setoff = NULL;
    desc->proc_reflector = NULL;
    desc->proc_absorb = NULL;
}

/* Sets the Ryo's velocity from its launch angle and speed, mirroring the cos/sin pair of ryo_stage_setting_. */
static void ftRemixGoemonRyoSetVelocity(GObj *ryo_gobj, FTStruct *fp, f32 angle, f32 speed)
{
    WPStruct *wp = wpGetStruct(ryo_gobj);

    wp->physics.vel_air.x = __cosf(angle) * speed * (f32) fp->lr;
    wp->physics.vel_air.y = __sinf(angle) * speed;
}

/* ryo_stage_setting_: creates the Ryo and its effect object at spawn_pos, using the charge flag to pick the fire set. */
static GObj *ftRemixGoemonRyoStageSetting(GObj *fighter_gobj, Vec3f *spawn_pos)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixGoemonChargeVars *charge = ftRemixGoemonChargeVars(fp);
    sb32 is_charged = (charge->charge_flag != FALSE);
    const FTRemixGoemonRyoProperties *props = is_charged ? &sFTRemixGoemonFireRyoProperties : &sFTRemixGoemonRyoProperties;
    intptr_t effect_offset = is_charged ? GOEMON_RYO_EFFECT_FIRE_OFFSET : GOEMON_RYO_EFFECT_SPARKLE_OFFSET;
    WPDesc ryo_desc;
    WPDesc effect_desc;
    GObj *ryo_gobj;
    GObj *effect_gobj;
    WPStruct *ryo_wp;
    FTRemixGoemonRyoVars *ryo_vars;
    f32 angle;

    ftRemixGoemonFillRyoDesc(&ryo_desc, fp, props->hitbox_offset);
    ryo_gobj = wpManagerMakeWeapon(fighter_gobj, &ryo_desc, spawn_pos, WEAPON_FLAG_COLLPROJECT);
    if (ryo_gobj == NULL)
    {
        return NULL;
    }
    ryo_wp = wpGetStruct(ryo_gobj);
    ryo_wp->group_id = wpManagerGetGroupID();
    ryo_vars = ftRemixGoemonRyoVars(ryo_gobj);
    ryo_vars->charge_flag = charge->charge_flag;
    ryo_vars->linked_gobj = NULL;

    ftRemixGoemonFillEffectDesc(&effect_desc, fp, effect_offset);
    effect_gobj = wpManagerMakeWeapon(fighter_gobj, &effect_desc, spawn_pos, 0);
    if (effect_gobj != NULL)
    {
        FTRemixGoemonRyoVars *effect_vars = ftRemixGoemonRyoVars(effect_gobj);

        wpGetStruct(effect_gobj)->group_id = ryo_wp->group_id;
        effect_vars->charge_flag = FALSE;
        effect_vars->linked_gobj = ryo_gobj;
        ryo_vars->linked_gobj = effect_gobj;
    }

    /* N64 WPStruct 0x264 is group_id and 0x268 is lifetime (wpMainDecLifeCheckExpire reads 0x268). */
    ryo_wp->lifetime = props->duration;

    angle = (fp->ga == nMPKineticsGround) ? props->initial_angle_ground : props->initial_angle_air;
    if (is_charged)
    {
        func_800269C0_275C0(GOEMON_NSP_SFX_FIRE);
    }
    ftRemixGoemonRyoSetVelocity(ryo_gobj, fp, angle, props->initial_speed);
    /* Palette index 0 in both property sets, which is 0.0F as raw bits. */
    DObjGetStruct(ryo_gobj)->mobj->palette_id = 0.0F;
    ftRemixGoemonRyoRotate(ryo_gobj);
    return ryo_gobj;
}

/* ground_wait_main_ and air_wait_main_ start the charge: FGM 28 and 65, and the GOEMON_CHARGE GFX. */
static void ftRemixGoemonNSPStartCharge(FTRemixGoemonChargeVars *charge)
{
    charge->charge_flag = TRUE;
    func_800269C0_275C0(GOEMON_NSP_SFX_CHARGE_1);
    func_800269C0_275C0(GOEMON_NSP_SFX_CHARGE_2);
    /* REMIX-TODO(GFXRoutine): GOEMON_CHARGE (GFXRoutine id 0x73) glow is not begun. */
}

/* GoemonNSP.end_main_ spawn path: the Ryo starts at the right hand joint, raised by a fixed offset. */
static void ftRemixGoemonNSPCreateRyo(GObj *fighter_gobj, FTStruct *fp)
{
    Vec3f spawn_pos = { 0.0F, 0.0F, 0.0F };

    gmCollisionGetFighterPartsWorldPosition(fp->joints[GOEMON_NSP_JOINT_RIGHT_HAND], &spawn_pos);
    spawn_pos.y += GOEMON_NSP_SPAWN_Y_OFFSET;
    spawn_pos.z = 0.0F;
    ftRemixGoemonRyoStageSetting(fighter_gobj, &spawn_pos);
}

/* GoemonNSP.ground_begin_initial_ */
static void ftRemixGoemonNSPGroundBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixGoemonChargeVars *charge = ftRemixGoemonChargeVars(fp);

    ftRemixGoemonNSPSetStatus(fighter_gobj, nFTRemixGoemonStatusNSPGroundBegin, nFTRemixGoemonKirbyStatusNSPGroundBegin, FTSTATUS_PRESERVE_NONE);
    ftRemixGoemonNSPClearTempVars(fp);
    charge->charge_timer = 0;
    charge->charge_flag = FALSE;
}

/* GoemonNSP.air_begin_initial_ */
static void ftRemixGoemonNSPAirBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixGoemonChargeVars *charge = ftRemixGoemonChargeVars(fp);

    ftRemixGoemonNSPSetStatus(fighter_gobj, nFTRemixGoemonStatusNSPAirBegin, nFTRemixGoemonKirbyStatusNSPAirBegin, FTSTATUS_PRESERVE_NONE);
    ftRemixGoemonNSPClearTempVars(fp);
    charge->charge_timer = 0;
    charge->charge_flag = FALSE;
}

/* GoemonNSP.ground_wait_initial_ */
static void ftRemixGoemonNSPGroundWaitInitial(GObj *fighter_gobj)
{
    ftRemixGoemonNSPSetStatus(fighter_gobj, nFTRemixGoemonStatusNSPGroundWait, nFTRemixGoemonKirbyStatusNSPGroundWait, FTSTATUS_PRESERVE_LOOPSFX);
}

/* GoemonNSP.air_wait_initial_ */
static void ftRemixGoemonNSPAirWaitInitial(GObj *fighter_gobj)
{
    ftRemixGoemonNSPSetStatus(fighter_gobj, nFTRemixGoemonStatusNSPAirWait, nFTRemixGoemonKirbyStatusNSPAirWait, FTSTATUS_PRESERVE_LOOPSFX);
}

/* GoemonNSP.ground_end_initial_ */
static void ftRemixGoemonNSPGroundEndInitial(GObj *fighter_gobj)
{
    ftRemixGoemonNSPSetStatus(fighter_gobj, nFTRemixGoemonStatusNSPGroundEnd, nFTRemixGoemonKirbyStatusNSPGroundEnd, FTSTATUS_PRESERVE_NONE);
}

/* GoemonNSP.air_end_initial_ */
static void ftRemixGoemonNSPAirEndInitial(GObj *fighter_gobj)
{
    ftRemixGoemonNSPSetStatus(fighter_gobj, nFTRemixGoemonStatusNSPAirEnd, nFTRemixGoemonKirbyStatusNSPAirEnd, FTSTATUS_PRESERVE_NONE);
}

/* Shared body of GoemonNSP.ground_begin_main_ and air_begin_main_: once temp variable 2 is set and B is released,
 * the end routine runs; otherwise the wait routine runs when the animation ends. */
static void ftRemixGoemonNSPBeginProcUpdate(GObj *fighter_gobj, void (*proc_end)(GObj *), void (*proc_wait)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_released = (fp->motion_vars.flags.flag1 != 0) && !(fp->input.pl.button_hold & B_BUTTON);

    if (is_released)
    {
        proc_end(fighter_gobj);
        return;
    }
    ftAnimEndCheckSetStatus(fighter_gobj, proc_wait);
}

/* GoemonNSP.ground_begin_main_ */
static void ftRemixGoemonNSPGroundBeginProcUpdate(GObj *fighter_gobj)
{
    ftRemixGoemonNSPBeginProcUpdate(fighter_gobj, ftRemixGoemonNSPGroundEndInitial, ftRemixGoemonNSPGroundWaitInitial);
}

/* GoemonNSP.air_begin_main_ */
static void ftRemixGoemonNSPAirBeginProcUpdate(GObj *fighter_gobj)
{
    ftRemixGoemonNSPBeginProcUpdate(fighter_gobj, ftRemixGoemonNSPAirEndInitial, ftRemixGoemonNSPAirWaitInitial);
}

/* Shared body of GoemonNSP.ground_wait_main_ and air_wait_main_: counts the charge, then ends the special on B release. */
static void ftRemixGoemonNSPWaitProcUpdate(GObj *fighter_gobj, void (*proc_end)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixGoemonChargeVars *charge = ftRemixGoemonChargeVars(fp);

    if (charge->charge_timer == GOEMON_NSP_CHARGE_TIME)
    {
        ftRemixGoemonNSPStartCharge(charge);
    }
    charge->charge_timer++;
    if (fp->input.pl.button_hold & B_BUTTON)
    {
        return;
    }
    proc_end(fighter_gobj);
}

/* GoemonNSP.ground_wait_main_ */
static void ftRemixGoemonNSPGroundWaitProcUpdate(GObj *fighter_gobj)
{
    ftRemixGoemonNSPWaitProcUpdate(fighter_gobj, ftRemixGoemonNSPGroundEndInitial);
}

/* GoemonNSP.air_wait_main_ */
static void ftRemixGoemonNSPAirWaitProcUpdate(GObj *fighter_gobj)
{
    ftRemixGoemonNSPWaitProcUpdate(fighter_gobj, ftRemixGoemonNSPAirEndInitial);
}

/* GoemonNSP.end_main_: releases the Ryo when temp variable 1 is set, then returns to idle at animation end. */
static void ftRemixGoemonNSPEndProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 is_firing = fp->motion_vars.flags.flag0;

    fp->motion_vars.flags.flag0 = 0;
    if (is_firing != 0)
    {
        ftRemixGoemonNSPCreateRyo(fighter_gobj, fp);
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* GoemonNSP.ground_to_air_: moves a grounded special to its aerial status. */
static void ftRemixGoemonNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 air_status;

    mpCommonSetFighterAir(fp);
    air_status = sFTRemixGoemonGroundToAirStatus[fp->status_id - nFTRemixGoemonStatusNSPGroundBegin];
    ftMainSetStatus(fighter_gobj, air_status, fighter_gobj->anim_frame, 1.0F, GOEMON_NSP_CONTINUE_FLAGS);
    ftPhysicsClampAirVelXMax(fp);
}

/* GoemonNSP.air_to_ground_: moves an aerial special to its grounded status. */
static void ftRemixGoemonNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 ground_status;

    mpCommonSetFighterGround(fp);
    ground_status = sFTRemixGoemonAirToGroundStatus[fp->status_id - nFTRemixGoemonStatusNSPAirBegin];
    ftMainSetStatus(fighter_gobj, ground_status, fighter_gobj->anim_frame, 1.0F, GOEMON_NSP_CONTINUE_FLAGS);
}

/* GoemonNSP.ground_collision_ */
static void ftRemixGoemonNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixGoemonNSPGroundToAir);
}

/* GoemonNSP.air_collision_ */
static void ftRemixGoemonNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixGoemonNSPAirToGround);
}

/* GoemonNSP.kirby_ground_to_air_: moves a grounded Kirby special to its aerial status. */
static void ftRemixGoemonNSPKirbyGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 air_status;

    mpCommonSetFighterAir(fp);
    air_status = sFTRemixGoemonKirbyGroundToAirStatus[fp->status_id - nFTRemixGoemonKirbyStatusNSPGroundBegin];
    ftMainSetStatus(fighter_gobj, air_status, fighter_gobj->anim_frame, 1.0F, GOEMON_NSP_CONTINUE_FLAGS);
    ftPhysicsClampAirVelXMax(fp);
}

/* GoemonNSP.kirby_air_to_ground_: moves an aerial Kirby special to its grounded status. */
static void ftRemixGoemonNSPKirbyAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 ground_status;

    mpCommonSetFighterGround(fp);
    ground_status = sFTRemixGoemonKirbyAirToGroundStatus[fp->status_id - nFTRemixGoemonKirbyStatusNSPAirBegin];
    ftMainSetStatus(fighter_gobj, ground_status, fighter_gobj->anim_frame, 1.0F, GOEMON_NSP_CONTINUE_FLAGS);
}

/* GoemonNSP.kirby_ground_collision_: the floor check (0x800DDDDC), with the Kirby ground-to-air transition. */
static void ftRemixGoemonNSPKirbyGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixGoemonNSPKirbyGroundToAir);
}

/* GoemonNSP.kirby_air_collision_: landing (0x800DE6E4), with the Kirby air-to-ground transition. */
static void ftRemixGoemonNSPKirbyAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixGoemonNSPKirbyAirToGround);
}

/* Start frame for a walk cycle change: the current cycle position scaled to the target cycle, truncated.
 * Returns 0 when either side is the wait status, as ground_transition_ does. */
static f32 ftRemixGoemonNSPCycleStartFrame(GObj *fighter_gobj, s32 target, s32 wait_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 current = fp->status_id;
    f32 multiplier;

    if ((current == wait_status) || (target == wait_status))
    {
        return 0.0F;
    }
    multiplier = fighter_gobj->anim_frame / sFTRemixGoemonCycleLength[current - wait_status];
    return (f32) (s32) (multiplier * sFTRemixGoemonCycleLength[target - wait_status]);
}

/* ground_transition_ and kirby_ground_transition_: changes the walk status, keeping the cycle position. */
static void ftRemixGoemonNSPGroundTransition(GObj *fighter_gobj, s32 target, s32 wait_status)
{
    f32 start_frame = ftRemixGoemonNSPCycleStartFrame(fighter_gobj, target, wait_status);

    ftMainSetStatus(fighter_gobj, target, start_frame, 1.0F, GOEMON_NSP_CONTINUE_FLAGS);
}

/* Maps the stick direction (stick_x times the facing) to a walk slot, as ground_interrupt_ does. */
static s32 ftRemixGoemonNSPGroundWalkSlot(s32 stick_dir)
{
    if (stick_dir < -44)
    {
        return GOEMON_NSP_WALK_SLOT_BWALK2;
    }
    if (stick_dir < -9)
    {
        return GOEMON_NSP_WALK_SLOT_BWALK1;
    }
    if (stick_dir < 10)
    {
        return GOEMON_NSP_WALK_SLOT_WAIT;
    }
    if (stick_dir < 45)
    {
        return GOEMON_NSP_WALK_SLOT_WALK1;
    }
    return GOEMON_NSP_WALK_SLOT_WALK2;
}

/* GoemonNSP.ground_interrupt_: switches between the grounded wait and walk statuses from the stick. */
static void ftRemixGoemonNSPGroundProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 stick_dir = fp->input.pl.stick_range.x * fp->lr;
    s32 slot = ftRemixGoemonNSPGroundWalkSlot(stick_dir);
    s32 target;

    if (fp->fkind == REMIX_ID_GOEMON)
    {
        target = sFTRemixGoemonGroundWalkStatus[slot];
        if (target != fp->status_id)
        {
            ftRemixGoemonNSPGroundTransition(fighter_gobj, target, nFTRemixGoemonStatusNSPGroundWait);
        }
        return;
    }
    target = sFTRemixGoemonKirbyGroundWalkStatus[slot];
    if (target != fp->status_id)
    {
        ftRemixGoemonNSPGroundTransition(fighter_gobj, target, nFTRemixGoemonKirbyStatusNSPGroundWait);
    }
}

/* GoemonNSP.ground_walk_physics_ */
static void ftRemixGoemonNSPGroundWalkProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftPhysicsSetGroundVelAbsStickRange(fp, GOEMON_NSP_WALK_MULTIPLIER, GOEMON_NSP_WALK_TRACTION);
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/* GoemonNSP.ground_back_walk_physics_ */
static void ftRemixGoemonNSPGroundBackWalkProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftPhysicsSetGroundVelAbsStickRange(fp, -GOEMON_NSP_WALK_MULTIPLIER, GOEMON_NSP_WALK_TRACTION);
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/* The vanilla air physics copied from ROM 0x548F0 (N64 0x800D90E0): fast fall or default gravity, then the x
 * decay clamp. Returns the clamp's result, which skips Remix air control and friction when TRUE. */
static sb32 ftRemixGoemonNSPAirVanillaPhysics(FTStruct *fp, FTAttributes *attr)
{
    if (fp->is_fastfall)
    {
        ftPhysicsApplyFastFall(fp, attr);
    }
    else
    {
        ftPhysicsApplyGravityDefault(fp, attr);
    }
    return ftPhysicsCheckClampAirVelXDecMax(fp, attr);
}

/* GoemonNSP.air_physics_ */
static void ftRemixGoemonNSPAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    if (ftRemixGoemonNSPAirVanillaPhysics(fp, attr))
    {
        return;
    }
    ftPhysicsClampAirVelXStickRange(fp, GOEMON_NSP_AIR_CONTROL_STICK_MIN, attr->air_accel, GOEMON_NSP_AIR_SPEED);
    ftPhysicsApplyAirVelXFriction(fp, attr);
}

/* Remix address -> port routine for every GoemonNSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixGoemonNSPProcs[] =
{
    { REMIX_SYM_GoemonNSP__air_begin_initial_, ftRemixGoemonNSPAirBeginInitial },
    { REMIX_SYM_GoemonNSP__air_begin_main_, ftRemixGoemonNSPAirBeginProcUpdate },
    { REMIX_SYM_GoemonNSP__air_collision_, ftRemixGoemonNSPAirProcMap },
    { REMIX_SYM_GoemonNSP__air_physics_, ftRemixGoemonNSPAirProcPhysics },
    { REMIX_SYM_GoemonNSP__air_wait_main_, ftRemixGoemonNSPAirWaitProcUpdate },
    { REMIX_SYM_GoemonNSP__end_main_, ftRemixGoemonNSPEndProcUpdate },
    { REMIX_SYM_GoemonNSP__ground_back_walk_physics_, ftRemixGoemonNSPGroundBackWalkProcPhysics },
    { REMIX_SYM_GoemonNSP__ground_begin_initial_, ftRemixGoemonNSPGroundBeginInitial },
    { REMIX_SYM_GoemonNSP__ground_begin_main_, ftRemixGoemonNSPGroundBeginProcUpdate },
    { REMIX_SYM_GoemonNSP__ground_collision_, ftRemixGoemonNSPGroundProcMap },
    { REMIX_SYM_GoemonNSP__ground_interrupt_, ftRemixGoemonNSPGroundProcInterrupt },
    { REMIX_SYM_GoemonNSP__ground_wait_main_, ftRemixGoemonNSPGroundWaitProcUpdate },
    { REMIX_SYM_GoemonNSP__ground_walk_physics_, ftRemixGoemonNSPGroundWalkProcPhysics },
    { REMIX_SYM_GoemonNSP__kirby_ground_collision_, ftRemixGoemonNSPKirbyGroundProcMap },
    { REMIX_SYM_GoemonNSP__kirby_air_collision_, ftRemixGoemonNSPKirbyAirProcMap }
};

const s32 dFTRemixGoemonNSPProcsNum = ARRAY_COUNT(dFTRemixGoemonNSPProcs);

#endif
