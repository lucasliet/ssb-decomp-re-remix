/*
 * ftremixaiweight.c - lv10 attack option weighting of the Remix AI (AI.asm weight_attack_options, 5485-5938). The pure
 * rules live in ftremixaiweightrules.c. The asm reads the fighter fields by N64 offset, named here where the port has a
 * field.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sys/utils.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixaiweight.h"
#include "ft/ftremix/ftremixaiweightrules.h"
#include "ft/ftremix/ftremixfields.h"

#define FTREMIX_AI_WEIGHT_LEVEL 10
#define FTREMIX_AI_WEIGHT_TABLE_ENTRY_BYTES 4

/* AI.asm weight_attack_options reads the shieldstun frames (float) of the opponent at N64 0x0B34. */
#define FTREMIX_AI_WEIGHT_N64_SHIELDSTUN 0x0B34

sb32 ftRemixAiWeightAttackOptionsApplies(FTStruct *fp)
{
    return fp->level == FTREMIX_AI_WEIGHT_LEVEL;
}

/**
 * Own landing rule (AI.asm _check_landing): the candidate is discarded when this fighter is airborne over a floor line
 * and would reach the ground before the move comes out.
 */
static f32 ftRemixAiWeightOwnLandingChance(FTStruct *this_fp, s32 hit_frame, f32 chance)
{
    sb32 is_airborne = this_fp->ga != nMPKineticsGround;

    if (!ftRemixAiWeightRulesCanLandOn(this_fp->coll_data.floor_line_id, is_airborne))
    {
        return chance;
    }
    if (ftRemixAiWeightRulesLandsBeforeMove(hit_frame, this_fp->coll_data.floor_dist, this_fp->physics.vel_air.y,
                                            this_fp->attr->gravity, -this_fp->attr->tvel_base))
    {
        return 0.0F;
    }
    return chance;
}

/**
 * Hitstun frames left for the target (AI.asm _check_opponent_landing). A target in a tech-able damage action that lands
 * inside its hitstun has the frames until landing as its hitstun.
 */
static s32 ftRemixAiWeightTargetHitstunLeft(FTStruct *target_fp, s32 hit_frame)
{
    s32 hitstun_tics = target_fp->status_vars.common.damage.hitstun_tics;
    sb32 is_airborne = target_fp->ga != nMPKineticsGround;

    if (!ftRemixAiWeightRulesCanLandOn(target_fp->coll_data.floor_line_id, is_airborne))
    {
        return hitstun_tics;
    }
    if (!ftRemixAiWeightRulesIsTechAction(target_fp->status_id))
    {
        return hitstun_tics;
    }
    return ftRemixAiWeightRulesHitstunLeft(hit_frame, hitstun_tics, target_fp->coll_data.floor_dist,
                                           target_fp->physics.vel_air.y, target_fp->attr->gravity,
                                           -target_fp->attr->tvel_base);
}

/**
 * Not-combo stage (AI.asm _not_combo): one random float is always drawn, then a slow move from an early frame is kept
 * only when the roll is below its threshold.
 */
static f32 ftRemixAiWeightNotComboChance(s32 hit_frame, f32 chance)
{
    f32 roll = syUtilsRandFloat();

    if (ftRemixAiWeightRulesNotComboKeeps(hit_frame, roll))
    {
        return chance;
    }
    return 0.0F;
}

/**
 * Combo commit (AI.asm _combo). The span is the time left in the target's hit value (N64 0x40, see the note in
 * ftRemixAiWeightComboChanceFor) minus the move start frame. A span of 2 or more rolls once, and the move is kept only
 * when the roll is 0. Kept combos get the combo multiplier.
 */
static f32 ftRemixAiWeightComboCommitChance(FTStruct *target_fp, s32 hit_frame, f32 chance)
{
    s32 span = (s32)target_fp->hitlag_tics - hit_frame;

    if (ftRemixAiWeightRulesComboNeedsRoll(span))
    {
        if (!ftRemixAiWeightRulesComboRollPasses(syUtilsRandIntRange(span)))
        {
            return 0.0F;
        }
    }
    return ftRemixAiWeightRulesComboChance(chance);
}

/**
 * Combo stage (AI.asm _check_combo). Only a target in hitstun can be combo'd. Otherwise the not-combo rule applies.
 *
 * REMIX-TODO(AI): the asm comment calls the 0x40 read the opponent hitstun timer, but by FTStruct layout (word after
 * unk_ft_0x3C) 0x40 is hitlag_tics. This port reads hitlag_tics, as the asm does. Confirm in a match.
 */
static f32 ftRemixAiWeightComboChanceFor(FTStruct *target_fp, s32 hit_frame, f32 chance)
{
    s32 hitstun_left;
    s32 shieldstun_frames;

    if (!target_fp->is_hitstun)
    {
        return ftRemixAiWeightNotComboChance(hit_frame, chance);
    }
    hitstun_left = ftRemixAiWeightTargetHitstunLeft(target_fp, hit_frame);
    shieldstun_frames = ftRemixAiWeightRulesTruncFrames(*ftRemixFighterFloat(target_fp, FTREMIX_AI_WEIGHT_N64_SHIELDSTUN));
    if (ftRemixAiWeightRulesIsCombo(hit_frame, TRUE, hitstun_left, target_fp->status_id, shieldstun_frames))
    {
        return ftRemixAiWeightComboCommitChance(target_fp, hit_frame, chance);
    }
    return ftRemixAiWeightNotComboChance(hit_frame, chance);
}

/** Slow move stage (AI.asm _less_odds_slow_moves): applies to every candidate, combo or not. */
static f32 ftRemixAiWeightSlowMoveChanceFor(FTStruct *target_fp, s32 hit_frame, f32 chance)
{
    s32 kind = ftRemixAiWeightRulesSlowMoveKind(target_fp->status_id, target_fp->coll_data.floor_line_id,
                                                target_fp->percent_damage);

    return ftRemixAiWeightRulesSlowMoveChance(chance, hit_frame, kind);
}

/** Grab against shield stage (AI.asm _check_shield). */
static f32 ftRemixAiWeightShieldChanceFor(FTStruct *target_fp, s32 input_kind, f32 chance)
{
    return ftRemixAiWeightRulesGrabVsShieldChance(chance, input_kind, target_fp->status_id);
}

/**
 * Kind of the Character.cpu_attack_weight routine of a fighter kind, FTREMIX_AI_WEIGHT_CUSTOM_NONE when its table entry is
 * empty. The routines are Remix code, so the port runs the C equivalent that the fighter kind selects (the shared files
 * register one routine for a family).
 */
static s32 ftRemixAiWeightGetCustomKind(s32 fkind)
{
    if (remix_ram_image_read_word(REMIX_SYM_Character__cpu_attack_weight__table +
                                  (fkind * FTREMIX_AI_WEIGHT_TABLE_ENTRY_BYTES)) == 0)
    {
        return FTREMIX_AI_WEIGHT_CUSTOM_NONE;
    }
    switch (fkind)
    {
    case REMIX_ID_CRASH:
        return FTREMIX_AI_WEIGHT_CUSTOM_CRASH;
    case REMIX_ID_DEDEDE:
    case REMIX_ID_MARINA:
    case REMIX_ID_KIRBY:
    case REMIX_ID_JKIRBY:
    case REMIX_ID_YOSHI:
    case REMIX_ID_JYOSHI:
        return FTREMIX_AI_WEIGHT_CUSTOM_NSP_VS_SHIELD;
    case REMIX_ID_PIANO:
        return FTREMIX_AI_WEIGHT_CUSTOM_PIANO;
    case REMIX_ID_GND:
    case REMIX_ID_DRAGONKING:
        return FTREMIX_AI_WEIGHT_CUSTOM_USP_VS_BELOW;
    case REMIX_ID_CAPTAIN:
    case REMIX_ID_JFALCON:
        return FTREMIX_AI_WEIGHT_CUSTOM_CAPTAIN;
    case REMIX_ID_JIGGLY:
    case REMIX_ID_JPUFF:
    case REMIX_ID_EPUFF:
        return FTREMIX_AI_WEIGHT_CUSTOM_PUFF;
    default:
        return FTREMIX_AI_WEIGHT_CUSTOM_NONE;
    }
}

/**
 * Per-fighter stage (AI.asm _custom_weight_table, last stage of weight_attack_options): the cpu_attack_weight routine of
 * this fighter kind rescales the chance of the candidate.
 */
static f32 ftRemixAiWeightCustomChanceFor(FTStruct *this_fp, FTStruct *target_fp, s32 input_kind, f32 chance)
{
    FTRemixAiWeightCustomContext context;
    s32 custom_kind = ftRemixAiWeightGetCustomKind(this_fp->fkind);

    if (custom_kind == FTREMIX_AI_WEIGHT_CUSTOM_NONE)
    {
        return chance;
    }
    context.input_kind = input_kind;
    context.cpu_level = this_fp->level;
    context.target_status_id = target_fp->status_id;
    context.target_height_diff = target_fp->coll_data.p_translate->y - this_fp->coll_data.p_translate->y;

    return ftRemixAiWeightRulesCustomChance(custom_kind, &context, chance);
}

f32 ftRemixAiWeightAttackOption(FTStruct *this_fp, FTStruct *target_fp, FTComputerAttack *comattack)
{
    s32 hit_frame = comattack->hit_start_frame;
    f32 chance = 1.0F;

    chance = ftRemixAiWeightOwnLandingChance(this_fp, hit_frame, chance);
    chance = ftRemixAiWeightComboChanceFor(target_fp, hit_frame, chance);
    chance = ftRemixAiWeightSlowMoveChanceFor(target_fp, hit_frame, chance);
    chance = ftRemixAiWeightShieldChanceFor(target_fp, comattack->input_kind, chance);
    chance = ftRemixAiWeightCustomChanceFor(this_fp, target_fp, comattack->input_kind, chance);
    return chance;
}

#endif
