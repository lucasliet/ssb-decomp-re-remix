#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaipostrules.h"

#define FTREMIX_AI_POST_STATUS_SHIELD_ON 0x98
#define FTREMIX_AI_POST_STATUS_SHIELD 0x99
#define FTREMIX_AI_POST_STATUS_SHIELD_STUN 0x9B
#define FTREMIX_AI_POST_STATUS_STUN 0xA4
#define FTREMIX_AI_POST_STATUS_SLEEP 0xA5
#define FTREMIX_AI_POST_STATUS_EGG_LAY_PULLED 0xB1
#define FTREMIX_AI_POST_STATUS_EGG_LAY 0xB2
#define FTREMIX_AI_POST_STATUS_SWORD_NSP_FIRST 0xE0
#define FTREMIX_AI_POST_STATUS_SWORD_NSP_HIGH 0xE1
#define FTREMIX_AI_POST_STATUS_SWORD_NSP_MID 0xE2
#define FTREMIX_AI_POST_STATUS_SWORD_NSP_LOW 0xE3
#define FTREMIX_AI_POST_TARGET_PERCENT_LOW 30
#define FTREMIX_AI_POST_TARGET_PERCENT_HIGH 100
#define FTREMIX_AI_POST_MARINA_SNATCH_BACK_FIRST 3
#define FTREMIX_AI_POST_MARINA_SNATCH_UP_FIRST 5
#define FTREMIX_AI_POST_MARINA_DSP_ROLL_BASE 500
#define FTREMIX_AI_POST_CARGO_SHAKE_TIMER 0x10

/** Bowser.Action.WhirlingFortress (Bowser/AI/Attacks.asm, GBowser/AI/Attacks.asm usp_control). */
int ftRemixAiPostIsWhirlingFortress(int status_id)
{
    return status_id == 0xDE;
}

/** DragonKing/AI/Attacks.asm cpu_post_process: the literal uspg 0xEF and uspa 0xF1. */
int ftRemixAiPostIsDragonKingUsp(int status_id)
{
    return (status_id == 0xEF) || (status_id == 0xF1);
}

/** Ganondorf.Action.WarlockDive/WarlockDiveEnd2 and Action.CAPTAIN.FalconDive/FalconDiveEnd2 share the ids. */
int ftRemixAiPostIsDiveGrab(int status_id)
{
    return (status_id == 0xEB) || (status_id == 0xEE);
}

/** Action.FOX.FireFoxStart to FireFoxAir and Falco.Action.FireBirdStart to FireBirdAir: 0xE3 to 0xE8. */
int ftRemixAiPostIsFoxFamilyUsp(int status_id)
{
    return (status_id >= 0xE3) && (status_id <= 0xE8);
}

/** Action.PIKACHU.QuickAttackStart to QuickAttackEndAir: 0xE8 to 0xED. */
int ftRemixAiPostIsQuickAttack(int status_id)
{
    return (status_id >= 0xE8) && (status_id <= 0xED);
}

/** Wolf.Action.WolfFlashStartAir 0xE3 and WolfFlashStart 0xE4. */
int ftRemixAiPostIsWolfFlashStart(int status_id)
{
    return (status_id == 0xE3) || (status_id == 0xE4);
}

/** Action.DK.HandSlapStart, HandSlapLoop and HandSlapEnd. */
int ftRemixAiPostIsHandSlap(int status_id)
{
    return (status_id >= 0xE8) && (status_id <= 0xEA);
}

/** Action.DK.SpinningKong and SpinningKongAir. */
int ftRemixAiPostIsSpinningKong(int status_id)
{
    return (status_id == 0xE6) || (status_id == 0xE7);
}

/** Marth.Action and Roy.Action NSPG_1, NSPG_2_High, NSPG_2_Mid and NSPG_2_Low. */
int ftRemixAiPostIsSwordNsp(int status_id)
{
    return (status_id >= FTREMIX_AI_POST_STATUS_SWORD_NSP_FIRST) && (status_id <= FTREMIX_AI_POST_STATUS_SWORD_NSP_LOW);
}

/** Dedede.Action.DSPG_CHARGE and DSPA_CHARGE. */
int ftRemixAiPostIsDededeCharge(int status_id)
{
    return (status_id == 0xF9) || (status_id == 0xFC);
}

/** Action.KIRBY.Stone, StoneFall, StoneLanding and StoneFall2. */
int ftRemixAiPostIsKirbyStone(int status_id)
{
    return (status_id == 0x106) || (status_id == 0x109) || (status_id == 0x10A) || (status_id == 0x10B);
}

/** Marina.Action.NSPGroundPull and NSPAirPull. */
int ftRemixAiPostIsMarinaPull(int status_id)
{
    return (status_id == 0xE2) || (status_id == 0xE7);
}

/** Marina.Action.Cargo and the commented-out CargoAir literal 0xF1. */
int ftRemixAiPostIsMarinaCargo(int status_id)
{
    return (status_id == 0xEB) || (status_id == 0xF1);
}

/** Ganondorf/AI/Attacks.asm _opponent_left, _opponent_right and captainshared.asm cpu_post_process_falcon. */
int ftRemixAiPostDiveTurnStickX(float target_x_diff, int lr)
{
    if (target_x_diff < 0.0F)
    {
        return (lr < 0) ? 0 : FTREMIX_AI_POST_STICK_MIN;
    }
    return (lr > 0) ? 0 : FTREMIX_AI_POST_STICK_MAX;
}

/** Falco, Wario and Wolf ground_check: the asm negates the 2000 only when own X minus target X is not negative. */
float ftRemixAiPostLandingProbeOffset(float location_x, float target_x)
{
    return ((location_x - target_x) < 0.0F) ? FTREMIX_AI_POST_LANDING_PROBE_OFFSET : -FTREMIX_AI_POST_LANDING_PROBE_OFFSET;
}

/** Wolf/AI/Attacks.asm usp_control: _hold_up, _hold_neutral and _hold_down. */
int ftRemixAiPostWolfStickY(float target_y_diff)
{
    if (target_y_diff >= FTREMIX_AI_POST_WOLF_HOLD_UP_DIFF)
    {
        return FTREMIX_AI_POST_STICK_MAX;
    }
    if (target_y_diff >= FTREMIX_AI_POST_WOLF_HOLD_NEUTRAL_DIFF)
    {
        return 0;
    }
    return FTREMIX_AI_POST_STICK_MIN;
}

/** dkshared.asm ground_check: a zero speed skips the probe, the offset follows the sign of the speed. */
float ftRemixAiPostDkProbeOffset(float vel_air_x)
{
    if (vel_air_x == 0.0F)
    {
        return 0.0F;
    }
    return (vel_air_x > 0.0F) ? FTREMIX_AI_POST_DK_PROBE_OFFSET : -FTREMIX_AI_POST_DK_PROBE_OFFSET;
}

/** dkshared.asm _cancel_velocity: a positive speed (sign above 0) holds left, otherwise right. */
int ftRemixAiPostDkHoldStickX(float vel_air_x)
{
    return (vel_air_x > 0.0F) ? FTREMIX_AI_POST_STICK_MIN : FTREMIX_AI_POST_STICK_MAX;
}

/** Dedede/AI/Attacks.asm charge_check: c.le.s 800, distance keeps B held, else it is released. */
int ftRemixAiPostDededeHoldsB(float distance)
{
    return distance >= FTREMIX_AI_POST_DEDEDE_CHARGE_DISTANCE;
}

/** Mewtwo/AI/Attacks.asm dsp_check: same facing, airborne target, Action.Stun and Action.Sleep. */
int ftRemixAiPostMewtwoSkipsDsp(int own_lr, int target_lr, int target_ga, int target_status_id)
{
    if (own_lr == target_lr)
    {
        return 1;
    }
    if (target_ga != 0)
    {
        return 1;
    }
    return (target_status_id == FTREMIX_AI_POST_STATUS_STUN) || (target_status_id == FTREMIX_AI_POST_STATUS_SLEEP);
}

/** jigglypuffkirbyshared.asm cpu_post_process dsp_check: ShieldOn, Shield, ShieldStun and below 30 percent. */
int ftRemixAiPostPuffSkipsDsp(int target_status_id, int target_percent)
{
    if ((target_status_id == FTREMIX_AI_POST_STATUS_SHIELD_ON) || (target_status_id == FTREMIX_AI_POST_STATUS_SHIELD) ||
        (target_status_id == FTREMIX_AI_POST_STATUS_SHIELD_STUN))
    {
        return 1;
    }
    return target_percent < FTREMIX_AI_POST_TARGET_PERCENT_LOW;
}

/** yoshishared.asm nsp_check: Action.EggLay and Action.EggLayPulled. */
int ftRemixAiPostYoshiSkipsNsp(int target_status_id)
{
    return (target_status_id == FTREMIX_AI_POST_STATUS_EGG_LAY) || (target_status_id == FTREMIX_AI_POST_STATUS_EGG_LAY_PULLED);
}

/** jigglypuffkirbyshared.asm cpu_post_process_kirby _dsp_logic. */
int ftRemixAiPostKirbyCancelsStone(int has_floor_below, int is_grounded, int has_hit)
{
    return (has_floor_below == 0) || (is_grounded != 0) || (has_hit != 0);
}

/** jigglypuffkirbyshared.asm _cancel_dsp: andi 0x0001 on the frame count picks press B or release all. */
int ftRemixAiPostKirbyMashButtons(int buttons, int frame_count)
{
    return ((frame_count & 1) != 0) ? (buttons | FTREMIX_AI_POST_BUTTON_B) : 0;
}

/** True when the asm falls to the random block: no fixed step, no shield, and 30 to 100 percent. */
int ftRemixAiPostSwordNspNeedsRoll(int status_id, int target_status_id, int target_percent)
{
    if ((status_id == FTREMIX_AI_POST_STATUS_SWORD_NSP_HIGH) || (status_id == FTREMIX_AI_POST_STATUS_SWORD_NSP_MID) ||
        (status_id == FTREMIX_AI_POST_STATUS_SWORD_NSP_LOW))
    {
        return 0;
    }
    if ((target_status_id == FTREMIX_AI_POST_STATUS_SHIELD) || (target_status_id == FTREMIX_AI_POST_STATUS_SHIELD_STUN))
    {
        return 0;
    }
    return (target_percent >= FTREMIX_AI_POST_TARGET_PERCENT_LOW) && (target_percent <= FTREMIX_AI_POST_TARGET_PERCENT_HIGH);
}

/** Marth/AI/Attacks.asm and Roy/AI/Attacks.asm nsp_logic. Step 2 repeats its height: High USP, Mid NSP, Low DSP. */
int ftRemixAiPostSwordNspInput(int status_id, int target_status_id, int target_percent, int roll)
{
    if (status_id == FTREMIX_AI_POST_STATUS_SWORD_NSP_HIGH)
    {
        return FTREMIX_AI_POST_INPUT_USP;
    }
    if (status_id == FTREMIX_AI_POST_STATUS_SWORD_NSP_MID)
    {
        return FTREMIX_AI_POST_INPUT_NSP;
    }
    if (status_id == FTREMIX_AI_POST_STATUS_SWORD_NSP_LOW)
    {
        return FTREMIX_AI_POST_INPUT_DSP;
    }
    if ((target_status_id == FTREMIX_AI_POST_STATUS_SHIELD) || (target_status_id == FTREMIX_AI_POST_STATUS_SHIELD_STUN))
    {
        return FTREMIX_AI_POST_INPUT_DSP;
    }
    if (target_percent < FTREMIX_AI_POST_TARGET_PERCENT_LOW)
    {
        return FTREMIX_AI_POST_INPUT_DSP;
    }
    if (target_percent > FTREMIX_AI_POST_TARGET_PERCENT_HIGH)
    {
        return FTREMIX_AI_POST_INPUT_NSP;
    }
    if (roll == 0)
    {
        return FTREMIX_AI_POST_INPUT_NSP;
    }
    return (roll == 1) ? FTREMIX_AI_POST_INPUT_USP : FTREMIX_AI_POST_INPUT_DSP;
}

/** Marina/AI/Attacks.asm jet_snatch _randomize. */
int ftRemixAiPostMarinaSnatchChoice(int roll)
{
    if (roll < FTREMIX_AI_POST_MARINA_SNATCH_BACK_FIRST)
    {
        return FTREMIX_AI_POST_MARINA_SNATCH_NONE;
    }
    return (roll < FTREMIX_AI_POST_MARINA_SNATCH_UP_FIRST) ? FTREMIX_AI_POST_MARINA_SNATCH_BACK
                                                          : FTREMIX_AI_POST_MARINA_SNATCH_UP;
}

/** Marina/AI/Attacks.asm jet_snatch _back: player X below target X gives the left extreme, else the right one. */
int ftRemixAiPostMarinaBackStickX(float player_x, float target_x)
{
    return (player_x < target_x) ? FTREMIX_AI_POST_STICK_MIN : FTREMIX_AI_POST_STICK_MAX;
}

/** Marina/AI/Attacks.asm cargo: blt skips below 0x10, so 0x10 and above shakes. */
int ftRemixAiPostMarinaCargoShakes(int breakout_wait)
{
    return breakout_wait >= FTREMIX_AI_POST_CARGO_SHAKE_TIMER;
}

/** Marina/AI/Attacks.asm dsp_check _check_distance: c.le.s 1000, distance. */
int ftRemixAiPostMarinaDspInDistance(float distance)
{
    return distance >= FTREMIX_AI_POST_MARINA_DSP_DISTANCE;
}

/** Marina/AI/Attacks.asm dsp_check _randomize: div 500 by the charge; the caller guarantees a nonzero charge. */
int ftRemixAiPostMarinaDspRollRange(int charge)
{
    return FTREMIX_AI_POST_MARINA_DSP_ROLL_BASE / charge;
}

#endif
