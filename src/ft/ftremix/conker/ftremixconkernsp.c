/**
 * ftremixconkernsp.c - C ports of Smash Remix Conker neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Conker/ConkerSpecial.asm, scope ConkerNSP).
 *
 * The nut weapon descriptor is built at run time because its file pointer is
 * the fighter's special file (Character.CONKER_file_6_ptr), so it cannot be a
 * static const table.
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

enum
{
    nFTRemixConkerStatusNSPGroundBegin = 0xF6,
    nFTRemixConkerStatusNSPGroundWait = 0xF7,
    nFTRemixConkerStatusNSPGroundEnd = 0xF8,
    nFTRemixConkerStatusNSPAirBegin = 0xF9,
    nFTRemixConkerStatusNSPAirWait = 0xFA,
    nFTRemixConkerStatusNSPAirEnd = 0xFB
};

/* Kirby and J Kirby use these ids for the same specials. */
enum
{
    nFTRemixConkerKirbyStatusNSPGroundBegin = 0x144,
    nFTRemixConkerKirbyStatusNSPGroundWait = 0x145,
    nFTRemixConkerKirbyStatusNSPGroundEnd = 0x146,
    nFTRemixConkerKirbyStatusNSPAirBegin = 0x147,
    nFTRemixConkerKirbyStatusNSPAirWait = 0x148,
    nFTRemixConkerKirbyStatusNSPAirEnd = 0x149
};

#define CONKER_NSP_FIRED_FRAME 6.0F               /* 0x40C00000: nut is released from this animation frame */
#define CONKER_NSP_SPAWN_JOINT 17                 /* FTStruct.joints[17] at N64 0x092C, the nut's hand */
#define CONKER_NUT_PROJECTILE_ID 0x24             /* Projectile.id.CONKER_NUT (Projectile.asm) */
#define CONKER_NUT_TRANSFORM_TK2 0x47             /* Secondary matrix transformation, as in wpmariofireball */
#define CONKER_NUT_SPAWN_Y_OFFSET 60.0F           /* 0x42700000 */
#define CONKER_NUT_INITIAL_SPEED 60.0F
#define CONKER_NUT_SPEED_PER_POWER 12.0F          /* 0x41400000 */
#define CONKER_NUT_ANGLE_PER_POWER 0.00872665F    /* 0x3C0EFA39: half a degree in radians */
/* The ground (0x18) and air (0x1C) initial angles are both 0.174533 in the properties struct. */
#define CONKER_NUT_INITIAL_ANGLE 0.174533F
#define CONKER_NUT_MAX_SPEED 120.0F
#define CONKER_NUT_DURATION 30                    /* nut properties 0x00 (ConkerSpecial.asm:970) */
#define CONKER_NUT_GRAVITY 1.2F
#define CONKER_NUT_ROTATION_SPEED 0.1F
#define CONKER_NUT_DAMAGE_PER_POWER 2
#define CONKER_NUT_KBG_PER_POWER 12
#define CONKER_NUT_BKB_PER_POWER 4
#define CONKER_NUT_MAX_POWER 4
#define CONKER_NUT_FGM_POWER_NONE 0x104
#define CONKER_NUT_FGM_POWER_LOW 0x103
#define CONKER_NUT_FGM_POWER_MAX 0x102
#define CONKER_NUT_FGM_HIT_MAX 0x1F

extern alSoundEffect *func_800269C0_275C0(u16 sfx_id);
extern sb32 itLGunWeaponAmmoProcMap(GObj *weapon_gobj);
extern sb32 wpMarioFireballProcHop(GObj *weapon_gobj);
extern sb32 wpSamusBombProcReflector(GObj *weapon_gobj);

static void ftRemixConkerNSPGroundToAir(GObj *fighter_gobj);
static void ftRemixConkerNSPAirToGround(GObj *fighter_gobj);
static sb32 ftRemixConkerNSPNutMain(GObj *nut_gobj);
static sb32 ftRemixConkerNSPNutDestruction(GObj *nut_gobj);

/* Kirby and J Kirby get their own status ids for the same special. */
static s32 ftRemixConkerNSPStatusForFighter(FTStruct *fp, s32 conker_status, s32 kirby_status)
{
    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return kirby_status;
    }
    return conker_status;
}

/* Sets the fighter's status to the Conker or Kirby id, then plays the anim events, as the *_initial_ routines do. */
static void ftRemixConkerNSPSetStatus(GObj *fighter_gobj, s32 conker_status, s32 kirby_status, u32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixConkerNSPStatusForFighter(fp, conker_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, flags);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Clears temp variables 1 to 3, which the begin routines do before the moveset runs. */
static void ftRemixConkerNSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* ConkerNSP.ground_end_initial_ */
static void ftRemixConkerNSPGroundEndInitial(GObj *fighter_gobj)
{
    ftRemixConkerNSPSetStatus(fighter_gobj, nFTRemixConkerStatusNSPGroundEnd, nFTRemixConkerKirbyStatusNSPGroundEnd, FTSTATUS_PRESERVE_NONE);
}

/* ConkerNSP.air_end_initial_ */
static void ftRemixConkerNSPAirEndInitial(GObj *fighter_gobj)
{
    ftRemixConkerNSPSetStatus(fighter_gobj, nFTRemixConkerStatusNSPAirEnd, nFTRemixConkerKirbyStatusNSPAirEnd, FTSTATUS_PRESERVE_NONE);
}

/* ConkerNSP.ground_wait_initial_ */
static void ftRemixConkerNSPGroundWaitInitial(GObj *fighter_gobj)
{
    ftRemixConkerNSPSetStatus(fighter_gobj, nFTRemixConkerStatusNSPGroundWait, nFTRemixConkerKirbyStatusNSPGroundWait, FTSTATUS_PRESERVE_LOOPSFX);
}

/* ConkerNSP.air_wait_initial_ */
static void ftRemixConkerNSPAirWaitInitial(GObj *fighter_gobj)
{
    ftRemixConkerNSPSetStatus(fighter_gobj, nFTRemixConkerStatusNSPAirWait, nFTRemixConkerKirbyStatusNSPAirWait, FTSTATUS_PRESERVE_LOOPSFX);
}

/* ConkerNSP.ground_begin_initial_ */
static void ftRemixConkerNSPGroundBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixConkerNSPSetStatus(fighter_gobj, nFTRemixConkerStatusNSPGroundBegin, nFTRemixConkerKirbyStatusNSPGroundBegin, FTSTATUS_PRESERVE_NONE);
    ftRemixConkerNSPClearTempVars(fp);
}

/* ConkerNSP.air_begin_initial_ */
static void ftRemixConkerNSPAirBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixConkerNSPSetStatus(fighter_gobj, nFTRemixConkerStatusNSPAirBegin, nFTRemixConkerKirbyStatusNSPAirBegin, FTSTATUS_PRESERVE_NONE);
    ftRemixConkerNSPClearTempVars(fp);
}

/* Shared body of ConkerNSP.ground_begin_main_ and air_begin_main_: once temp variable 2 is set and B is released,
 * the end routine runs; otherwise the wait routine runs when the animation ends. */
static void ftRemixConkerNSPBeginProcUpdate(GObj *fighter_gobj, void (*proc_end)(GObj *), void (*proc_wait)(GObj *))
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

/* ConkerNSP.ground_begin_main_ */
static void ftRemixConkerNSPGroundBeginProcUpdate(GObj *fighter_gobj)
{
    ftRemixConkerNSPBeginProcUpdate(fighter_gobj, ftRemixConkerNSPGroundEndInitial, ftRemixConkerNSPGroundWaitInitial);
}

/* ConkerNSP.air_begin_main_ */
static void ftRemixConkerNSPAirBeginProcUpdate(GObj *fighter_gobj)
{
    ftRemixConkerNSPBeginProcUpdate(fighter_gobj, ftRemixConkerNSPAirEndInitial, ftRemixConkerNSPAirWaitInitial);
}

/* ConkerNSP.ground_wait_main_ */
static void ftRemixConkerNSPGroundWaitProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.button_hold & B_BUTTON)
    {
        return;
    }
    ftRemixConkerNSPGroundEndInitial(fighter_gobj);
}

/* ConkerNSP.air_wait_main_ */
static void ftRemixConkerNSPAirWaitProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.button_hold & B_BUTTON)
    {
        return;
    }
    ftRemixConkerNSPAirEndInitial(fighter_gobj);
}

/* Fills the nut's weapon descriptor (asm nut_projectile_struct) from the fighter's special file. */
static void ftRemixConkerNSPFillNutDesc(WPDesc *nut_desc, FTStruct *fp)
{
    nut_desc->flags = 0;
    nut_desc->kind = CONKER_NUT_PROJECTILE_ID;
    nut_desc->p_weapon = fp->data->p_file_special1;
    nut_desc->o_attributes = 0;
    nut_desc->transform_types.tk1 = nGCMatrixKindTra;
    nut_desc->transform_types.tk2 = CONKER_NUT_TRANSFORM_TK2;
    nut_desc->transform_types.tk3 = 0;
    nut_desc->proc_update = ftRemixConkerNSPNutMain;
    nut_desc->proc_map = itLGunWeaponAmmoProcMap;
    nut_desc->proc_hit = ftRemixConkerNSPNutDestruction;
    nut_desc->proc_shield = ftRemixConkerNSPNutDestruction;
    nut_desc->proc_hop = wpMarioFireballProcHop;
    nut_desc->proc_setoff = ftRemixConkerNSPNutDestruction;
    nut_desc->proc_reflector = wpSamusBombProcReflector;
    nut_desc->proc_absorb = ftRemixConkerNSPNutDestruction;
}

/* Sound for the nut's power level: none at power 0, a medium one up to max power, and the strong one at max. */
static u16 ftRemixConkerNSPNutFGM(s32 power)
{
    if (power == 0)
    {
        return CONKER_NUT_FGM_POWER_NONE;
    }
    if (power < CONKER_NUT_MAX_POWER)
    {
        return CONKER_NUT_FGM_POWER_LOW;
    }
    return CONKER_NUT_FGM_POWER_MAX;
}

/* Scales the nut's hitbox, speed and angle by temp variable 3 (the power level), then launches it.
 * Remix keeps the power in attack_pos[1].pos_curr.x (N64 WPStruct 0x1B4), so it stays readable by nut_main_. */
static void ftRemixConkerNSPInitNut(GObj *nut_gobj, FTStruct *fp)
{
    WPStruct *wp = wpGetStruct(nut_gobj);
    s32 power = fp->motion_vars.flags.flag2;
    sb32 is_max_power = (power >= CONKER_NUT_MAX_POWER);
    f32 power_f = (f32) power;
    f32 speed = CONKER_NUT_INITIAL_SPEED + (CONKER_NUT_SPEED_PER_POWER * power_f);
    f32 angle = CONKER_NUT_INITIAL_ANGLE - (CONKER_NUT_ANGLE_PER_POWER * power_f);

    /* N64 WPStruct 0x268 is lifetime, as wpMainDecLifeCheckExpire reads it. */
    wp->lifetime = CONKER_NUT_DURATION;
    wp->attack_coll.attack_pos[1].pos_curr.x = power_f;

    func_800269C0_275C0(ftRemixConkerNSPNutFGM(power));

    wp->attack_coll.damage += power * CONKER_NUT_DAMAGE_PER_POWER;
    wp->attack_coll.knockback_scale += power * CONKER_NUT_KBG_PER_POWER;
    wp->attack_coll.knockback_base += power * CONKER_NUT_BKB_PER_POWER;
    if (is_max_power)
    {
        wp->attack_coll.fgm_id = CONKER_NUT_FGM_HIT_MAX;
    }

    wp->physics.vel_air.z = 0.0F;
    wp->physics.vel_air.x = __cosf(angle) * speed * (f32) fp->lr;
    wp->physics.vel_air.y = __sinf(angle) * speed;

    DObjGetStruct(nut_gobj)->mobj->palette_id = 0.0F;
    wpMainVelSetModelPitch(nut_gobj);
}

/* ConkerNSP.nut_stage_setting_: creates the nut weapon at spawn_pos from the fighter's special file. */
static void ftRemixConkerNSPStageNut(GObj *fighter_gobj, Vec3f *spawn_pos)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    WPDesc nut_desc;
    GObj *nut_gobj;

    ftRemixConkerNSPFillNutDesc(&nut_desc, fp);
    nut_gobj = wpManagerMakeWeapon(fighter_gobj, &nut_desc, spawn_pos, WEAPON_FLAG_COLLPROJECT);
    if (nut_gobj == NULL)
    {
        return;
    }
    ftRemixConkerNSPInitNut(nut_gobj, fp);
}

/* ConkerNSP.end_main_ spawn path: the nut starts at the hand joint, raised by a fixed offset. */
static void ftRemixConkerNSPCreateNut(GObj *fighter_gobj, FTStruct *fp)
{
    Vec3f spawn_pos = { 0.0F, 0.0F, 0.0F };

    gmCollisionGetFighterPartsWorldPosition(fp->joints[CONKER_NSP_SPAWN_JOINT], &spawn_pos);
    spawn_pos.y += CONKER_NUT_SPAWN_Y_OFFSET;
    spawn_pos.z = 0.0F;
    ftRemixConkerNSPStageNut(fighter_gobj, &spawn_pos);
}

/* ConkerNSP.end_main_: releases the nut when temp variable 1 is set, then returns to idle at animation end. */
static void ftRemixConkerNSPEndProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 is_firing = fp->motion_vars.flags.flag0;

    fp->motion_vars.flags.flag0 = 0;
    if (is_firing != 0)
    {
        ftRemixConkerNSPCreateNut(fighter_gobj, fp);
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* ConkerNSP.ground_to_air_: moves a grounded special to its aerial status (status id + 3). */
static void ftRemixConkerNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id + 3, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_LOOPSFX);
    ftPhysicsClampAirVelXMax(fp);
}

/* ConkerNSP.air_to_ground_: moves an aerial special to its grounded status (status id - 3). */
static void ftRemixConkerNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id - 3, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_LOOPSFX);
}

/* ConkerNSP.ground_collision_ */
static void ftRemixConkerNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixConkerNSPGroundToAir);
}

/* ConkerNSP.air_collision_ */
static void ftRemixConkerNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixConkerNSPAirToGround);
}

/* ConkerNSP.air_collision_end_: before the nut is released, landing returns to the grounded wait; after, it cancels. */
static void ftRemixConkerNSPAirEndProcMap(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame < CONKER_NSP_FIRED_FRAME)
    {
        mpCommonProcFighterLanding(fighter_gobj, ftRemixConkerNSPAirToGround);
        return;
    }
    mpCommonProcFighterCliffWaitOrLanding(fighter_gobj);
}

/* ConkerNSP.nut_main_: the nut's per-frame update (proc_update). Returns TRUE when the nut is destroyed. */
static sb32 ftRemixConkerNSPNutMain(GObj *nut_gobj)
{
    WPStruct *wp = wpGetStruct(nut_gobj);
    DObj *nut_dobj = DObjGetStruct(nut_gobj);
    f32 spin_multiplier = 1.0F + (wp->attack_coll.attack_pos[1].pos_curr.x * 0.5F);

    if (wpMainDecLifeCheckExpire(wp))
    {
        efManagerDustExpandSmallMakeEffect(&nut_dobj->translate.vec.f, 1.0F);
        return TRUE;
    }
    wpMainApplyGravityClampTVel(wp, CONKER_NUT_GRAVITY, CONKER_NUT_MAX_SPEED);
    nut_dobj->rotate.vec.f.x += CONKER_NUT_ROTATION_SPEED * spin_multiplier;
    return FALSE;
}

/* ConkerNSP.nut_destruction_: removes the nut with a smoke effect (hit, shield, set-off and absorb procs). */
static sb32 ftRemixConkerNSPNutDestruction(GObj *nut_gobj)
{
    DObj *nut_dobj = DObjGetStruct(nut_gobj);

    efManagerDustExpandSmallMakeEffect(&nut_dobj->translate.vec.f, 1.0F);
    return TRUE;
}

/* Remix address -> port routine for every ConkerNSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixConkerNSPProcs[] =
{
    { REMIX_SYM_ConkerNSP__air_begin_initial_, ftRemixConkerNSPAirBeginInitial },
    { REMIX_SYM_ConkerNSP__air_begin_main_, ftRemixConkerNSPAirBeginProcUpdate },
    { REMIX_SYM_ConkerNSP__air_collision_, ftRemixConkerNSPAirProcMap },
    { REMIX_SYM_ConkerNSP__air_collision_end_, ftRemixConkerNSPAirEndProcMap },
    { REMIX_SYM_ConkerNSP__air_wait_main_, ftRemixConkerNSPAirWaitProcUpdate },
    { REMIX_SYM_ConkerNSP__end_main_, ftRemixConkerNSPEndProcUpdate },
    { REMIX_SYM_ConkerNSP__ground_begin_initial_, ftRemixConkerNSPGroundBeginInitial },
    { REMIX_SYM_ConkerNSP__ground_begin_main_, ftRemixConkerNSPGroundBeginProcUpdate },
    { REMIX_SYM_ConkerNSP__ground_collision_, ftRemixConkerNSPGroundProcMap },
    { REMIX_SYM_ConkerNSP__ground_wait_main_, ftRemixConkerNSPGroundWaitProcUpdate }
};

const s32 dFTRemixConkerNSPProcsNum = ARRAY_COUNT(dFTRemixConkerNSPProcs);

#endif
