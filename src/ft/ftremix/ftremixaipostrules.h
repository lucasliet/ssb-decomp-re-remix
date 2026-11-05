#ifndef _FTREMIXAIPOSTRULES_H_
#define _FTREMIXAIPOSTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the per-fighter Remix AI post process (Character.cpu_post_process table, AI.asm 3733, and the
 * <Fighter>/AI/Attacks.asm or <family>shared.asm routines it points to), so the headless unit tests (tests/remix) can run
 * them without game state. Numbers in, numbers out. */

/* Vanilla attack inputs and Remix routine ids (AI.asm ATTACK_TABLE.*.INPUT and the add_cpu_input_routine order). */
#define FTREMIX_AI_POST_INPUT_NSP 0x09
#define FTREMIX_AI_POST_INPUT_USP 0x0D
#define FTREMIX_AI_POST_INPUT_DSP 0x1B
#define FTREMIX_AI_POST_ROUTINE_POINT_STICK_TO_TARGET 0x49
#define FTREMIX_AI_POST_ROUTINE_WARIO_NSP_TOWARDS 0x4D
#define FTREMIX_AI_POST_ROUTINE_WARIO_NSP_TOWARDS_JUMP 0x4E
#define FTREMIX_AI_POST_ROUTINE_MARINA_DOUBLE_DSP 0x52

/* Stick extremes the routines write (0x50 and 0xFFB0 in the asm). */
#define FTREMIX_AI_POST_STICK_MAX 80
#define FTREMIX_AI_POST_STICK_MIN (-80)

/* Distances and offsets of the asm float constants (lui immediates 0x44FA, 0x43C8, 0x4448, 0x447A, 0x4489, 0x442F). */
#define FTREMIX_AI_POST_LANDING_PROBE_OFFSET 2000.0F
#define FTREMIX_AI_POST_DK_PROBE_OFFSET 400.0F
#define FTREMIX_AI_POST_DEDEDE_CHARGE_DISTANCE 800.0F
#define FTREMIX_AI_POST_MARINA_DSP_DISTANCE 1000.0F
#define FTREMIX_AI_POST_WOLF_HOLD_UP_DIFF 1096.0F
#define FTREMIX_AI_POST_WOLF_HOLD_NEUTRAL_DIFF 700.0F

/* Button mask the routines press (Joypad.B). */
#define FTREMIX_AI_POST_BUTTON_B 0x4000

/* Marth and Roy NSP chain inputs and the choices of ftRemixAiPostMarinaSnatchChoice. */
#define FTREMIX_AI_POST_SWORD_NSP_WAIT 0x0F
#define FTREMIX_AI_POST_SWORD_NSP_ROLL_RANGE 3
#define FTREMIX_AI_POST_MARINA_SNATCH_ROLL_RANGE 10
#define FTREMIX_AI_POST_MARINA_SNATCH_NONE 0
#define FTREMIX_AI_POST_MARINA_SNATCH_BACK 1
#define FTREMIX_AI_POST_MARINA_SNATCH_UP 2
#define FTREMIX_AI_POST_MARINA_ITEM_ABSORB_ROLL_RANGE 600

/* TRUE for the Bowser and Giga Bowser grounded USP (Bowser.Action.WhirlingFortress 0xDE). */
extern int ftRemixAiPostIsWhirlingFortress(int status_id);

/* TRUE for the Dragon King USP actions (0xEF grounded, 0xF1 aerial). */
extern int ftRemixAiPostIsDragonKingUsp(int status_id);

/* TRUE for the Ganondorf and Captain Falcon dive actions (WarlockDive and FalconDive 0xEB, DiveEnd2 0xEE). */
extern int ftRemixAiPostIsDiveGrab(int status_id);

/* TRUE for the Fox and Falco USP actions the CPU steers down (0xE3 to 0xE8: start, readying and travel, ground and air). */
extern int ftRemixAiPostIsFoxFamilyUsp(int status_id);

/* TRUE for the Pikachu quick attack actions the CPU steers down (0xE8 to 0xED, ground and air). */
extern int ftRemixAiPostIsQuickAttack(int status_id);

/* TRUE for the Wolf USP start actions (WolfFlashStartAir 0xE3, WolfFlashStart 0xE4). */
extern int ftRemixAiPostIsWolfFlashStart(int status_id);

/* TRUE for the DK hand slap actions (0xE8 to 0xEA). */
extern int ftRemixAiPostIsHandSlap(int status_id);

/* TRUE for the DK Spinning Kong actions (0xE6 grounded, 0xE7 aerial). */
extern int ftRemixAiPostIsSpinningKong(int status_id);

/* TRUE for the Marth and Roy grounded NSP actions (NSPG_1 0xE0, NSPG_2_High 0xE1, Mid 0xE2, Low 0xE3). */
extern int ftRemixAiPostIsSwordNsp(int status_id);

/* TRUE for the Dedede down special charge actions (DSPG_CHARGE 0xF9, DSPA_CHARGE 0xFC). */
extern int ftRemixAiPostIsDededeCharge(int status_id);

/* TRUE for the Kirby stone actions (Stone 0x106, StoneFall 0x109, StoneLanding 0x10A, StoneFall2 0x10B). */
extern int ftRemixAiPostIsKirbyStone(int status_id);

/* TRUE for the Marina jet snatch pull actions (NSPGroundPull 0xE2, NSPAirPull 0xE7). */
extern int ftRemixAiPostIsMarinaPull(int status_id);

/* TRUE for the Marina cargo actions (Cargo 0xEB, CargoAir 0xF1). */
extern int ftRemixAiPostIsMarinaCargo(int status_id);

/*
 * Stick X of the turnaround in Ganondorf and Captain Falcon dive control: 0 when the fighter already faces the target
 * (the caller points the stick at it), else the stick extreme toward it. target_x_diff is target X minus own X, lr is the
 * facing (1 right, -1 left).
 */
extern int ftRemixAiPostDiveTurnStickX(float target_x_diff, int lr);

/*
 * X offset of the probe the landing check casts (Falco, Wario, Wolf ground_check): 2000 toward the target, where the
 * target is to the right when own X minus target X is negative.
 */
extern float ftRemixAiPostLandingProbeOffset(float location_x, float target_x);

/* Wolf USP stick Y from target Y minus own Y: up from 1096, neutral from 700, down below. */
extern int ftRemixAiPostWolfStickY(float target_y_diff);

/* DK Spinning Kong ground probe offset: 400 along the sign of the horizontal speed, 0 when it is 0 (no probe). */
extern float ftRemixAiPostDkProbeOffset(float vel_air_x);

/* DK Spinning Kong stick X when moving off stage: the stick extreme against the horizontal speed. */
extern int ftRemixAiPostDkHoldStickX(float vel_air_x);

/* Dedede keeps B held while the target is at least 800 away, else it releases. */
extern int ftRemixAiPostDededeHoldsB(float distance);

/* Mewtwo DSP is skipped (NULL command) when both face the same way, the target is airborne, stunned or asleep. */
extern int ftRemixAiPostMewtwoSkipsDsp(int own_lr, int target_lr, int target_ga, int target_status_id);

/* Jigglypuff DSP is skipped when the target shields (0x98, 0x99, 0x9B) or is below 30 percent. */
extern int ftRemixAiPostPuffSkipsDsp(int target_status_id, int target_percent);

/* Yoshi NSP is skipped when the target is already in an egg (EggLayPulled 0xB1, EggLay 0xB2). */
extern int ftRemixAiPostYoshiSkipsNsp(int target_status_id);

/* Kirby stone is cancelled with no floor below, when grounded, or after the stone hit something. */
extern int ftRemixAiPostKirbyCancelsStone(int has_floor_below, int is_grounded, int has_hit);

/* Kirby stone cancel mash: B is pressed on odd frames and every button is released on even ones. */
extern int ftRemixAiPostKirbyMashButtons(int buttons, int frame_count);

/* TRUE when the Marth/Roy NSP chain draws a 0..2 roll: no fixed follow-up, no shield, target at 30 to 100 percent. */
extern int ftRemixAiPostSwordNspNeedsRoll(int status_id, int target_status_id, int target_percent);

/* Input of the Marth/Roy NSP follow-up (roll is ignored unless ftRemixAiPostSwordNspNeedsRoll). */
extern int ftRemixAiPostSwordNspInput(int status_id, int target_status_id, int target_percent, int roll);

/* Marina jet snatch choice from a 0..9 roll: 0 to 2 nothing, 3 and 4 throw back, 5 to 9 throw up. */
extern int ftRemixAiPostMarinaSnatchChoice(int roll);

/* Marina back throw stick X: away from the target, which is the extreme opposite its side. */
extern int ftRemixAiPostMarinaBackStickX(float player_x, float target_x);

/* Marina shakes the cargo victim loose when the victim's breakout timer is at least 0x10. */
extern int ftRemixAiPostMarinaCargoShakes(int breakout_wait);

/* Marina considers the double DSP only with the target at least 1000 away. */
extern int ftRemixAiPostMarinaDspInDistance(float distance);

/* Range of the 1 in N roll of the Marina DSP: 500 divided by the charge (the asm divides, higher charge, better odds). */
extern int ftRemixAiPostMarinaDspRollRange(int charge);

#endif

#endif
