#ifndef _FTREMIXAIWEIGHTRULES_H_
#define _FTREMIXAIWEIGHTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix lv10 attack option weighting (AI.asm weight_attack_options, 5485-5938), so the headless unit
 * tests (tests/remix) can run them without game state. Numbers in, numbers out. Action ids are the Action.asm values.
 */

#define FTREMIX_AI_WEIGHT_ACTION_DAMAGE_FLY_HIGH 0x033
#define FTREMIX_AI_WEIGHT_ACTION_TUMBLE 0x039
#define FTREMIX_AI_WEIGHT_ACTION_SHIELD_ON 0x098
#define FTREMIX_AI_WEIGHT_ACTION_SHIELD 0x099
#define FTREMIX_AI_WEIGHT_ACTION_SHIELD_STUN 0x09B

#define FTREMIX_AI_WEIGHT_INPUT_GRAB 28
#define FTREMIX_AI_WEIGHT_INPUT_DASH_GRAB 0x045

#define FTREMIX_AI_WEIGHT_SLOW_NORMAL 0
#define FTREMIX_AI_WEIGHT_SLOW_SMALL_ADVANTAGE 1
#define FTREMIX_AI_WEIGHT_SLOW_ADVANTAGE 2

#define FTREMIX_AI_WEIGHT_COMBO_MULTIPLIER 1000.0F
#define FTREMIX_AI_WEIGHT_COMBO_MIN_SPAN 2
#define FTREMIX_AI_WEIGHT_COMBO_WINDOW 2

/** TRUE when the fighter has a floor line below it and is airborne: the only case where the landing check runs. */
int ftRemixAiWeightRulesCanLandOn(int floor_line_id, int is_airborne);

/** Moves the fall speed toward max_fall once it is at or past max_fall (both negative). */
float ftRemixAiWeightRulesCapFallSpeed(float vel_y, float max_fall);

/** TRUE when the fighter reaches the ground within hit_frame frames, integrating gravity like the asm loop. */
int ftRemixAiWeightRulesLandsBeforeMove(int hit_frame, float floor_dist, float vel_y, float gravity, float max_fall);

/** TRUE for the damage actions that can tech on landing (DamageFlyHigh to Tumble). */
int ftRemixAiWeightRulesIsTechAction(int status_id);

/**
 * Hitstun frames left for the opponent. Returns hitstun_tics unless the opponent lands within the hitstun, and then the
 * frames until landing minus one. The loop stops early when the elapsed frames reach hitstun_tics.
 */
int ftRemixAiWeightRulesHitstunLeft(int hit_frame, int hitstun_tics, float floor_dist, float vel_y, float gravity,
                                    float max_fall);

/** TRUE when the move hits inside the opponent hitstun (or, in ShieldStun, before the shield stun ends). */
int ftRemixAiWeightRulesIsCombo(int hit_frame, int is_hitstun, int hitstun_left, int status_id, int shieldstun_frames);

/** TRUE when the combo span is long enough that the asm rolls for it. */
int ftRemixAiWeightRulesComboNeedsRoll(int span);

/** TRUE when the combo roll result keeps the move (the asm discards it when the roll is not 0). */
int ftRemixAiWeightRulesComboRollPasses(int roll);

/** Chance multiplier of a combo move. */
float ftRemixAiWeightRulesComboChance(float chance);

/**
 * TRUE when a non-combo move keeps its chance. Moves starting before frame 15 always keep it. Later moves keep it when
 * roll is below 1/8 (15), 1/32 (20) or 1/64 (30).
 */
int ftRemixAiWeightRulesNotComboKeeps(int hit_frame, float roll);

/** FTREMIX_AI_WEIGHT_SLOW_* kind of the opponent state: advantage, small advantage or normal. */
int ftRemixAiWeightRulesSlowMoveKind(int status_id, int floor_line_id, int percent);

/** Slow move chance: scaled by the start frame (advantage or small advantage) or divided by its square (normal). */
float ftRemixAiWeightRulesSlowMoveChance(float chance, int hit_frame, int kind);

/** Grab input against a shielding opponent gets the combo multiplier, any other input keeps the chance. */
float ftRemixAiWeightRulesGrabVsShieldChance(float chance, int input_kind, int status_id);

/** Truncates a frame count like the MIPS trunc.w.s: values out of the s32 range become 0x7FFFFFFF. */
int ftRemixAiWeightRulesTruncFrames(float frames);

/* Character.cpu_attack_weight routines (<Fighter>/AI/Attacks.asm, <family>shared.asm), one kind per distinct routine. */
typedef enum FTRemixAiWeightCustomKind
{
    FTREMIX_AI_WEIGHT_CUSTOM_NONE,
    FTREMIX_AI_WEIGHT_CUSTOM_CRASH,
    FTREMIX_AI_WEIGHT_CUSTOM_NSP_VS_SHIELD,
    FTREMIX_AI_WEIGHT_CUSTOM_PIANO,
    FTREMIX_AI_WEIGHT_CUSTOM_USP_VS_BELOW,
    FTREMIX_AI_WEIGHT_CUSTOM_CAPTAIN,
    FTREMIX_AI_WEIGHT_CUSTOM_PUFF
} FTRemixAiWeightCustomKind;

#define FTREMIX_AI_WEIGHT_INPUT_NSP 0x09
#define FTREMIX_AI_WEIGHT_INPUT_USP 0x0D
#define FTREMIX_AI_WEIGHT_INPUT_USMASH 0x19
#define FTREMIX_AI_WEIGHT_INPUT_DSP 0x1B
#define FTREMIX_AI_WEIGHT_INPUT_DASH_USMASH 0x46
#define FTREMIX_AI_WEIGHT_INPUT_CRASH_DASH_NSP 0x55
#define FTREMIX_AI_WEIGHT_CUSTOM_LEVEL 10
#define FTREMIX_AI_WEIGHT_CUSTOM_SHIELD_CHANCE 100.0F
#define FTREMIX_AI_WEIGHT_CUSTOM_USMASH_FACTOR 2.0F
#define FTREMIX_AI_WEIGHT_CUSTOM_PUFF_NSP_FACTOR 0.0625F
#define FTREMIX_AI_WEIGHT_CUSTOM_BELOW_LIMIT (-100.0F)

/* State a cpu_attack_weight routine reads: the candidate input, the CPU level, the opponent action and the opponent height over the CPU. */
typedef struct FTRemixAiWeightCustomContext
{
    int input_kind;
    int cpu_level;
    int target_status_id;
    float target_height_diff;
} FTRemixAiWeightCustomContext;

/**
 * New chance of a candidate attack after a fighter's cpu_attack_weight routine (AI.asm _custom_weight_table). Every kind
 * but Crash applies at level 10 only. Crash drops NSP and its dash against a shielding opponent, Dedede, Marina, Kirby and
 * Yoshi raise NSP to 100 against a shield, Piano does the same with DSP, Ganondorf and Dragon King drop USP when the opponent
 * is 100 or more below and raise it against a shield, Captain Falcon doubles up smash and dash grab and dash up smash and
 * treats USP like Ganondorf, Jigglypuff scales NSP by 1/16.
 */
extern float ftRemixAiWeightRulesCustomChance(int custom_kind, const FTRemixAiWeightCustomContext *context, float chance);

#endif

#endif
