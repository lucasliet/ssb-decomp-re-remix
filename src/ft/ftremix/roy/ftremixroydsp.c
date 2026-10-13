/**
 * ftremixroydsp.c - C ports of Smash Remix Roy down special routines
 * (.toolchain/pkgs/smashremix-src/src/Roy/RoySpecial.asm, scope RoyDSP).
 *
 * The charge is driven by temp variable 1 (motion_vars.flags.flag0, set by
 * moveset command 5C0000XX), the release by temp variable 2 (flag1) and the
 * bonus power by temp variable 3 (flag2). The charge level is the N64 word
 * at 0x0B18, read through ftRemixFighterWord.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

#define ROY_DSP_MAX_CHARGE_LEVEL 21
#define ROY_DSP_BONUS_DAMAGE 10
#define ROY_DSP_CHARGE_DAMAGE_PER_LEVEL 2
#define ROY_DSP_SHIELD_DAMAGE_BASE 8
#define ROY_DSP_HITBOX_NUM 4
#define ROY_DSP_ATTACK_JUST_ACTIVE 1
#define ROY_DSP_TRANSITION_FLAGS 0x2803 /* keeps sword trails, 3C command FGM, gfx routines, hitboxes */

#define ROY_DSP_OFF_CHARGE_LEVEL 0x0B18 /* s32 charge level; shares the word with status_vars.common */

enum
{
    nFTRemixRoyStatusDSPG_Begin = 0xEF,
    nFTRemixRoyStatusDSPG_Wait = 0xF0,
    nFTRemixRoyStatusDSPG_End = 0xF1,
    nFTRemixRoyStatusDSPG_Strong_End = 0xF2,
    nFTRemixRoyStatusDSPA_Begin = 0xF3,
    nFTRemixRoyStatusDSPA_Wait = 0xF4,
    nFTRemixRoyStatusDSPA_End = 0xF5,
    nFTRemixRoyStatusDSPA_Strong_End = 0xF6
};

/* Charge level counter, N64 0x0B18. */
static s32 *ftRemixRoyDSPChargeLevel(FTStruct *fp)
{
    return ftRemixFighterWord(fp, ROY_DSP_OFF_CHARGE_LEVEL);
}

/* Starts `status_id` from its first frame and plays its animation events. */
static void ftRemixRoyDSPStartStatus(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

static void ftRemixRoyDSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Shared start of RoyDSP.ground_begin_initial_ and air_begin_initial_. */
static void ftRemixRoyDSPBeginInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixRoyDSPStartStatus(fighter_gobj, status_id);
    ftRemixRoyDSPClearTempVars(fp);
    *ftRemixRoyDSPChargeLevel(fp) = 0;
}

/* RoyDSP.ground_begin_initial_ */
static void ftRemixRoyDSPGroundBeginInitial(GObj *fighter_gobj)
{
    ftRemixRoyDSPBeginInitial(fighter_gobj, nFTRemixRoyStatusDSPG_Begin);
}

/* RoyDSP.air_begin_initial_ */
static void ftRemixRoyDSPAirBeginInitial(GObj *fighter_gobj)
{
    ftRemixRoyDSPBeginInitial(fighter_gobj, nFTRemixRoyStatusDSPA_Begin);
}

/* RoyDSP.ground_wait_initial_ (transition target, not in the worklist). */
static void ftRemixRoyDSPGroundWaitInitial(GObj *fighter_gobj)
{
    ftRemixRoyDSPStartStatus(fighter_gobj, nFTRemixRoyStatusDSPG_Wait);
}

/* RoyDSP.air_wait_initial_ (transition target, not in the worklist). */
static void ftRemixRoyDSPAirWaitInitial(GObj *fighter_gobj)
{
    ftRemixRoyDSPStartStatus(fighter_gobj, nFTRemixRoyStatusDSPA_Wait);
}

/* Shared body of ground_begin_main_ and air_begin_main_. Releasing B while
 * temp variable 2 is set ends the move; otherwise the begin status plays out
 * and then hands over to its wait status. */
static void ftRemixRoyDSPBeginMain(GObj *fighter_gobj, s32 end_status, void (*wait_initial)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_released = (fp->motion_vars.flags.flag1 != 0) && !(fp->input.pl.button_hold & B_BUTTON);

    if (is_released)
    {
        ftRemixRoyDSPStartStatus(fighter_gobj, end_status);
        return;
    }
    ftAnimEndCheckSetStatus(fighter_gobj, wait_initial);
}

/* RoyDSP.ground_begin_main_ */
static void ftRemixRoyDSPGroundBeginMain(GObj *fighter_gobj)
{
    /* The asm reaches ground_end_initial_ without setting a1, so its action id is
     * whatever the caller left there. The asm comment says it transitions to
     * DSP_Ground_End, which is used here. See the notes file. */
    ftRemixRoyDSPBeginMain(fighter_gobj, nFTRemixRoyStatusDSPG_End, ftRemixRoyDSPGroundWaitInitial);
}

/* RoyDSP.air_begin_main_ (same release caveat as ftRemixRoyDSPGroundBeginMain). */
static void ftRemixRoyDSPAirBeginMain(GObj *fighter_gobj)
{
    ftRemixRoyDSPBeginMain(fighter_gobj, nFTRemixRoyStatusDSPA_End, ftRemixRoyDSPAirWaitInitial);
}

/* Consumes temp variable 1, which the moveset sets to advance the charge, and
 * adds one charge level for it. Returns TRUE when the charge advanced. */
static sb32 ftRemixRoyDSPConsumeChargePulse(FTStruct *fp)
{
    sb32 is_pulse = (fp->motion_vars.flags.flag0 != 0);

    fp->motion_vars.flags.flag0 = 0;
    if (is_pulse)
    {
        (*ftRemixRoyDSPChargeLevel(fp))++;
    }
    return is_pulse;
}

/* Shared body of ground_wait_main_ and air_wait_main_. A full charge ends the
 * move at once; otherwise the move ends when B is released. */
static void ftRemixRoyDSPWaitMain(GObj *fighter_gobj, s32 end_status, s32 strong_end_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixRoyDSPConsumeChargePulse(fp) && (*ftRemixRoyDSPChargeLevel(fp) == ROY_DSP_MAX_CHARGE_LEVEL))
    {
        ftRemixRoyDSPStartStatus(fighter_gobj, strong_end_status);
        return;
    }
    if (fp->input.pl.button_hold & B_BUTTON)
    {
        return;
    }
    ftRemixRoyDSPStartStatus(fighter_gobj, end_status);
}

/* RoyDSP.ground_wait_main_ */
static void ftRemixRoyDSPGroundWaitMain(GObj *fighter_gobj)
{
    ftRemixRoyDSPWaitMain(fighter_gobj, nFTRemixRoyStatusDSPG_End, nFTRemixRoyStatusDSPG_Strong_End);
}

/* RoyDSP.air_wait_main_ */
static void ftRemixRoyDSPAirWaitMain(GObj *fighter_gobj)
{
    ftRemixRoyDSPWaitMain(fighter_gobj, nFTRemixRoyStatusDSPA_End, nFTRemixRoyStatusDSPA_Strong_End);
}

/* Adds charge damage to each hitbox that has just become active. The shield
 * damage of the k-th active hitbox is (8 + charge) >> k, because the asm halves
 * the shield value after every active hitbox. */
static void ftRemixRoyDSPUpdateHitboxes(FTStruct *fp)
{
    s32 charge_level = *ftRemixRoyDSPChargeLevel(fp);
    s32 shield_damage = ROY_DSP_SHIELD_DAMAGE_BASE + charge_level;
    s32 hitbox_index;

    for (hitbox_index = 0; hitbox_index < ROY_DSP_HITBOX_NUM; hitbox_index++)
    {
        FTAttackColl *attack_coll = &fp->attack_colls[hitbox_index];

        if (attack_coll->attack_state != ROY_DSP_ATTACK_JUST_ACTIVE)
        {
            continue;
        }
        attack_coll->damage += charge_level * ROY_DSP_CHARGE_DAMAGE_PER_LEVEL;
        attack_coll->shield_damage = shield_damage;
        shield_damage >>= 1;
    }
}

/* Sets the animation to idle or fall once the animation has ended. */
static void ftRemixRoyDSPAnimEndToIdle(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* Temp variable 3 (flag2) adds ROY_DSP_BONUS_DAMAGE to Roy once and is then cleared. */
static void ftRemixRoyDSPApplyBonusDamage(FTStruct *fp)
{
    if (fp->motion_vars.flags.flag2 == 0)
    {
        return;
    }
    ftParamUpdateDamage(fp, ROY_DSP_BONUS_DAMAGE);
    fp->motion_vars.flags.flag2 = 0;
}

/* RoyDSP.end_main_ (shared by the ground and air end statuses) */
static void ftRemixRoyDSPEndMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixRoyDSPApplyBonusDamage(fp);
    ftRemixRoyDSPUpdateHitboxes(fp);
    ftRemixRoyDSPAnimEndToIdle(fighter_gobj);
}

/* Ground to air transition of the down special: the air status is the ground status + 4. */
static void ftRemixRoyDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id + 4, fighter_gobj->anim_frame, 1.0F, ROY_DSP_TRANSITION_FLAGS);
    ftPhysicsClampAirVelXMax(fp);
}

/* Air to ground transition of the down special: the ground status is the air status - 4. */
static void ftRemixRoyDSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id - 4, fighter_gobj->anim_frame, 1.0F, ROY_DSP_TRANSITION_FLAGS);
}

/* RoyDSP.ground_collision_ */
static void ftRemixRoyDSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixRoyDSPGroundToAir);
}

/* RoyDSP.air_collision_ */
static void ftRemixRoyDSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixRoyDSPAirToGround);
}

/* Remix address -> port routine for every RoyDSP routine in the Roy action
 * array and the down special tables. */
const FTRemixProc dFTRemixRoyDSPProcs[] =
{
    { REMIX_SYM_RoyDSP__ground_begin_initial_, ftRemixRoyDSPGroundBeginInitial },
    { REMIX_SYM_RoyDSP__air_begin_initial_, ftRemixRoyDSPAirBeginInitial },
    { REMIX_SYM_RoyDSP__ground_begin_main_, ftRemixRoyDSPGroundBeginMain },
    { REMIX_SYM_RoyDSP__air_begin_main_, ftRemixRoyDSPAirBeginMain },
    { REMIX_SYM_RoyDSP__ground_wait_main_, ftRemixRoyDSPGroundWaitMain },
    { REMIX_SYM_RoyDSP__air_wait_main_, ftRemixRoyDSPAirWaitMain },
    { REMIX_SYM_RoyDSP__end_main_, ftRemixRoyDSPEndMain },
    { REMIX_SYM_RoyDSP__ground_collision_, ftRemixRoyDSPGroundProcMap },
    { REMIX_SYM_RoyDSP__air_collision_, ftRemixRoyDSPAirProcMap }
};

const s32 dFTRemixRoyDSPProcsNum = ARRAY_COUNT(dFTRemixRoyDSPProcs);

#endif
