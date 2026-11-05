#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaiweightrules.h"

/** TRUE for the Action.asm states of the advantage list of weight_attack_options (_advantage). */
static int ftRemixAiWeightRulesIsAdvantageStatus(int status_id)
{
    switch (status_id)
    {
    case 0x043:
    case 0x044:
    case 0x09E:
    case 0x09F:
    case 0x0A0:
    case 0x0A1:
    case 0x0A2:
    case 0x0A3:
    case 0x0A4:
    case 0x0A5:
    case 0x03A:
    case 0x0B1:
    case 0x0B2:
    case 0x0AB:
        return 1;
    default:
        return 0;
    }
}

/** TRUE for the Action.asm states of the small advantage list of weight_attack_options (_small_advantage). */
static int ftRemixAiWeightRulesIsSmallAdvantageStatus(int status_id)
{
    switch (status_id)
    {
    case 0x09C:
    case 0x09D:
    case 0x051:
    case 0x049:
    case 0x04A:
        return 1;
    default:
        return 0;
    }
}

/** TRUE for the shield states of the opponent that let a grab get the combo multiplier. */
static int ftRemixAiWeightRulesIsShieldStatus(int status_id)
{
    return (status_id == FTREMIX_AI_WEIGHT_ACTION_SHIELD_ON) || (status_id == FTREMIX_AI_WEIGHT_ACTION_SHIELD) ||
           (status_id == FTREMIX_AI_WEIGHT_ACTION_SHIELD_STUN);
}

/** TRUE for the grab inputs: vanilla grab (input 28) and the Remix DASH_GRAB routine. */
static int ftRemixAiWeightRulesIsGrabInput(int input_kind)
{
    return (input_kind == FTREMIX_AI_WEIGHT_INPUT_GRAB) || (input_kind == FTREMIX_AI_WEIGHT_INPUT_DASH_GRAB);
}

int ftRemixAiWeightRulesCanLandOn(int floor_line_id, int is_airborne)
{
    return (floor_line_id >= 0) && is_airborne;
}

float ftRemixAiWeightRulesCapFallSpeed(float vel_y, float max_fall)
{
    if (vel_y <= max_fall)
    {
        return max_fall;
    }
    return vel_y;
}

int ftRemixAiWeightRulesLandsBeforeMove(int hit_frame, float floor_dist, float vel_y, float gravity, float max_fall)
{
    int frame;

    for (frame = 0; frame < hit_frame; frame++)
    {
        floor_dist = floor_dist - vel_y;
        if (floor_dist >= 0.0F)
        {
            return 1;
        }
        vel_y = ftRemixAiWeightRulesCapFallSpeed(vel_y - gravity, max_fall);
    }
    return 0;
}

int ftRemixAiWeightRulesIsTechAction(int status_id)
{
    return (status_id >= FTREMIX_AI_WEIGHT_ACTION_DAMAGE_FLY_HIGH) && (status_id <= FTREMIX_AI_WEIGHT_ACTION_TUMBLE);
}

int ftRemixAiWeightRulesHitstunLeft(int hit_frame, int hitstun_tics, float floor_dist, float vel_y, float gravity,
                                    float max_fall)
{
    int elapsed;

    for (elapsed = 0; elapsed < hit_frame; elapsed++)
    {
        if (elapsed == hitstun_tics)
        {
            return hitstun_tics;
        }
        floor_dist = floor_dist - vel_y;
        if (floor_dist >= 0.0F)
        {
            return elapsed - 1;
        }
        vel_y = ftRemixAiWeightRulesCapFallSpeed(vel_y - gravity, max_fall);
    }
    return hitstun_tics;
}

int ftRemixAiWeightRulesIsCombo(int hit_frame, int is_hitstun, int hitstun_left, int status_id, int shieldstun_frames)
{
    if (!is_hitstun)
    {
        return 0;
    }
    if (hit_frame <= (hitstun_left + FTREMIX_AI_WEIGHT_COMBO_WINDOW))
    {
        return 1;
    }
    if (status_id != FTREMIX_AI_WEIGHT_ACTION_SHIELD_STUN)
    {
        return 0;
    }
    return hit_frame <= shieldstun_frames;
}

int ftRemixAiWeightRulesComboNeedsRoll(int span)
{
    return span >= FTREMIX_AI_WEIGHT_COMBO_MIN_SPAN;
}

int ftRemixAiWeightRulesComboRollPasses(int roll)
{
    return roll == 0;
}

float ftRemixAiWeightRulesComboChance(float chance)
{
    return FTREMIX_AI_WEIGHT_COMBO_MULTIPLIER * chance;
}

int ftRemixAiWeightRulesNotComboKeeps(int hit_frame, float roll)
{
    float threshold;

    if (hit_frame >= 30)
    {
        threshold = 0.015625F;
    }
    else if (hit_frame >= 20)
    {
        threshold = 0.03125F;
    }
    else if (hit_frame >= 15)
    {
        threshold = 0.125F;
    }
    else
    {
        return 1;
    }
    return roll < threshold;
}

int ftRemixAiWeightRulesSlowMoveKind(int status_id, int floor_line_id, int percent)
{
    if (floor_line_id == -1)
    {
        return FTREMIX_AI_WEIGHT_SLOW_ADVANTAGE;
    }
    if (ftRemixAiWeightRulesIsAdvantageStatus(status_id))
    {
        return FTREMIX_AI_WEIGHT_SLOW_ADVANTAGE;
    }
    if (ftRemixAiWeightRulesIsSmallAdvantageStatus(status_id))
    {
        return FTREMIX_AI_WEIGHT_SLOW_SMALL_ADVANTAGE;
    }
    if (percent > 150)
    {
        return FTREMIX_AI_WEIGHT_SLOW_NORMAL;
    }
    if (percent > 90)
    {
        return FTREMIX_AI_WEIGHT_SLOW_SMALL_ADVANTAGE;
    }
    return FTREMIX_AI_WEIGHT_SLOW_NORMAL;
}

float ftRemixAiWeightRulesSlowMoveChance(float chance, int hit_frame, int kind)
{
    float frame = (float)hit_frame;
    float frame_squared = frame * frame;

    if (kind == FTREMIX_AI_WEIGHT_SLOW_SMALL_ADVANTAGE)
    {
        return chance * frame;
    }
    if (kind == FTREMIX_AI_WEIGHT_SLOW_ADVANTAGE)
    {
        return chance * frame_squared;
    }
    return chance / frame_squared;
}

float ftRemixAiWeightRulesGrabVsShieldChance(float chance, int input_kind, int status_id)
{
    if (!ftRemixAiWeightRulesIsGrabInput(input_kind))
    {
        return chance;
    }
    if (!ftRemixAiWeightRulesIsShieldStatus(status_id))
    {
        return chance;
    }
    return FTREMIX_AI_WEIGHT_COMBO_MULTIPLIER * chance;
}

int ftRemixAiWeightRulesTruncFrames(float frames)
{
    if (!((frames > -2147483648.0F) && (frames < 2147483648.0F)))
    {
        return 0x7FFFFFFF;
    }
    return (int)frames;
}

/** The three states in which an opponent shields (ShieldOn, Shield, ShieldStun). */
static int ftRemixAiWeightRulesTargetShields(const FTRemixAiWeightCustomContext *context)
{
    return ftRemixAiWeightRulesIsShieldStatus(context->target_status_id);
}

/** Shield check shared by the NSP and DSP routines: 100 against a shield, the chance otherwise. */
static float ftRemixAiWeightRulesShieldChance(const FTRemixAiWeightCustomContext *context, float chance)
{
    return ftRemixAiWeightRulesTargetShields(context) ? FTREMIX_AI_WEIGHT_CUSTOM_SHIELD_CHANCE : chance;
}

/** Up special of Ganondorf, Dragon King and Captain Falcon: 0 with the opponent 100 or more below, 100 against a shield. */
static float ftRemixAiWeightRulesUspChance(const FTRemixAiWeightCustomContext *context, float chance)
{
    if (!(FTREMIX_AI_WEIGHT_CUSTOM_BELOW_LIMIT <= context->target_height_diff))
    {
        return 0.0F;
    }
    return ftRemixAiWeightRulesShieldChance(context, chance);
}

/** Crash/AI/Attacks.asm: no level gate, NSP and the dash NSP drop to 0 against a shield. */
static float ftRemixAiWeightRulesCrashChance(const FTRemixAiWeightCustomContext *context, float chance)
{
    int is_nsp = (context->input_kind == FTREMIX_AI_WEIGHT_INPUT_NSP) ||
                 (context->input_kind == FTREMIX_AI_WEIGHT_INPUT_CRASH_DASH_NSP);

    if (is_nsp && ftRemixAiWeightRulesTargetShields(context))
    {
        return 0.0F;
    }
    return chance;
}

/** captainshared.asm: up smash inputs doubled, USP like Ganondorf. */
static float ftRemixAiWeightRulesCaptainChance(const FTRemixAiWeightCustomContext *context, float chance)
{
    int input_kind = context->input_kind;

    if ((input_kind == FTREMIX_AI_WEIGHT_INPUT_USMASH) || (input_kind == FTREMIX_AI_WEIGHT_INPUT_DASH_GRAB) ||
        (input_kind == FTREMIX_AI_WEIGHT_INPUT_DASH_USMASH))
    {
        return chance * FTREMIX_AI_WEIGHT_CUSTOM_USMASH_FACTOR;
    }
    if (input_kind == FTREMIX_AI_WEIGHT_INPUT_USP)
    {
        return ftRemixAiWeightRulesUspChance(context, chance);
    }
    return chance;
}

/** Level 10 routine of the kind, for the candidate input of the context. */
static float ftRemixAiWeightRulesLevelTenChance(int custom_kind, const FTRemixAiWeightCustomContext *context, float chance)
{
    int input_kind = context->input_kind;

    switch (custom_kind)
    {
    case FTREMIX_AI_WEIGHT_CUSTOM_NSP_VS_SHIELD:
        return (input_kind == FTREMIX_AI_WEIGHT_INPUT_NSP) ? ftRemixAiWeightRulesShieldChance(context, chance) : chance;
    case FTREMIX_AI_WEIGHT_CUSTOM_PIANO:
        return (input_kind == FTREMIX_AI_WEIGHT_INPUT_DSP) ? ftRemixAiWeightRulesShieldChance(context, chance) : chance;
    case FTREMIX_AI_WEIGHT_CUSTOM_USP_VS_BELOW:
        return (input_kind == FTREMIX_AI_WEIGHT_INPUT_USP) ? ftRemixAiWeightRulesUspChance(context, chance) : chance;
    case FTREMIX_AI_WEIGHT_CUSTOM_CAPTAIN:
        return ftRemixAiWeightRulesCaptainChance(context, chance);
    case FTREMIX_AI_WEIGHT_CUSTOM_PUFF:
        return (input_kind == FTREMIX_AI_WEIGHT_INPUT_NSP) ? chance * FTREMIX_AI_WEIGHT_CUSTOM_PUFF_NSP_FACTOR : chance;
    default:
        return chance;
    }
}

float ftRemixAiWeightRulesCustomChance(int custom_kind, const FTRemixAiWeightCustomContext *context, float chance)
{
    if (custom_kind == FTREMIX_AI_WEIGHT_CUSTOM_CRASH)
    {
        return ftRemixAiWeightRulesCrashChance(context, chance);
    }
    if (context->cpu_level < FTREMIX_AI_WEIGHT_CUSTOM_LEVEL)
    {
        return chance;
    }
    return ftRemixAiWeightRulesLevelTenChance(custom_kind, context, chance);
}

#endif
