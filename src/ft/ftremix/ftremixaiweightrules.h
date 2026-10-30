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

#endif

#endif
