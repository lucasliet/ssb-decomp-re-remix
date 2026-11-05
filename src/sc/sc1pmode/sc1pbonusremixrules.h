#ifndef _SC1PBONUSREMIXRULES_H_
#define _SC1PBONUSREMIXRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Remix Break the Targets and Board the Platforms stage tables (Bonus.asm, SinglePlayer.asm extend_high_score_btt/btp_*,
 * set_btt_stage_id_, set_btp_stage_id_, Stages.asm bonus_fix_*): the 33 stages the Bonus screens can pick, the record of a fighter on
 * each of them and which stage a fighter plays. Kept apart so the headless unit tests (tests/remix) run them.
 */

#define SC1PBONUS_REMIX_STAGE_COUNT 33

/* Targets or platforms of a completed bonus; a record below it is a count, at it the time is valid. */
#define SC1PBONUS_REMIX_TASK_MAX 10

/* extend_high_score_btt_time_write_: the time a record holds before its first completion (0x034BC0). */
#define SC1PBONUS_REMIX_TIME_UNSET 0x34BC0u

/* Stages.class: the class table values the bonus stages use. */
#define SC1PBONUS_REMIX_CLASS_BTP 2
#define SC1PBONUS_REMIX_CLASS_BTT 3

/* Character.BTT_TABLE entry of a fighter without a bonus stage. */
#define SC1PBONUS_REMIX_NO_STAGE 0xFF

/* Where the stage of a bonus comes from (set_btt_stage_id_, set_btp_stage_id_). */
#define SC1PBONUS_REMIX_SOURCE_VANILLA 0
#define SC1PBONUS_REMIX_SOURCE_DEFAULT_TABLE 1
#define SC1PBONUS_REMIX_SOURCE_REMIX_1P_TABLE 2
#define SC1PBONUS_REMIX_SOURCE_SELECTED 3

/* Stock icons beside the STAGE row (Bonus.asm setup_, update_stage_and_icons_): X of a lone icon and of a pair, Y, and Y/X of the row text. */
#define SC1PBONUS_REMIX_ICON_X_SINGLE 245.0F
#define SC1PBONUS_REMIX_ICON_X_FIRST 240.0F
#define SC1PBONUS_REMIX_ICON_X_SECOND 250.0F
#define SC1PBONUS_REMIX_ICON_Y 176.0F
#define SC1PBONUS_REMIX_STAGE_LABEL_X 208.0F
#define SC1PBONUS_REMIX_STAGE_LABEL_Y 175.0F
#define SC1PBONUS_REMIX_ARROW_LEFT_X 212.0F
#define SC1PBONUS_REMIX_ARROW_RIGHT_X 280.0F
#define SC1PBONUS_REMIX_ARROW_Y 176.0F
#define SC1PBONUS_REMIX_SET_STAGE_TEXT_X 78.5F
#define SC1PBONUS_REMIX_SET_STAGE_TEXT_Y 217.0F
#define SC1PBONUS_REMIX_SET_STAGE_BUTTON_X 61.75F
#define SC1PBONUS_REMIX_SET_STAGE_BUTTON_Y 216.0F
#define SC1PBONUS_REMIX_SET_STAGE_TEXT_SCALE 0.8125F

/* File.STOCK_ICONS: the stock icons beside the STAGE row (the same file the All-Star Rest Area uses) */
#define SC1PBONUS_REMIX_ICON_FILE_ID 0xD8B

/**
 * Index of a stage in a stage table (get_bonus_stage_index_).
 *
 * @param stage_table The 33 stage ids of the table.
 * @param stage_id Stage id looked for.
 * @return The index, or -1 when the stage is not in the table.
 */
extern int sc1PBonusRemixStageIndexOf(const unsigned char *stage_table, int count, int stage_id);

/**
 * Whether a stage is the one the fighter normally plays: the records of that stage live in the standard tables.
 *
 * @param stage_id Stage played or selected.
 * @param default_stage_id Entry of Character.BTT_TABLE or BTP_TABLE of the fighter.
 */
extern int sc1PBonusRemixIsDefaultStage(int stage_id, int default_stage_id);

/**
 * Stage index after an arrow press of the STAGE row: wraps over the table (handle_arrow_press_ _stage_pressed).
 *
 * @param index Current stage index.
 * @param direction -1 or 1.
 * @param count Stages in the table.
 */
extern int sc1PBonusRemixStageStep(int index, int direction, int count);

/**
 * Task count a record keeps after a failed run: the highest of the stored and the new count; a finished record keeps its 10.
 *
 * @param stored_count Count in the record (10 when finished).
 * @param new_count Targets or platforms done in the run.
 */
extern int sc1PBonusRemixCountAfterRun(int stored_count, int new_count);

/**
 * Time a record compares a finish with: the stored time once finished, the unset time before.
 *
 * @param stored_count Count in the record.
 * @param stored_time Time in the record.
 */
extern unsigned int sc1PBonusRemixRecordTime(int stored_count, unsigned int stored_time);

/** Whether a finish time beats the record (strictly faster, or any time on a record never finished). */
extern int sc1PBonusRemixIsTimeBeaten(int stored_count, unsigned int stored_time, unsigned int new_time);

/**
 * Where the stage of a Break the Targets or Board the Platforms run comes from.
 *
 * @param is_remix_1p Non-zero in the Remix 1P mode (REMIX_BTT_TABLE / REMIX_BTP_TABLE).
 * @param is_remix_choice Non-zero when the Bonus screen choice is Remix (Bonus.mode).
 * @param fkind Fighter kind.
 * @param first_added_fkind First fighter kind past the vanilla playable and polygon fighters (Character.id.BOSS).
 * @return SC1PBONUS_REMIX_SOURCE_*.
 */
extern int sc1PBonusRemixStageSource(int is_remix_1p, int is_remix_choice, int fkind, int first_added_fkind);

/**
 * Whether the stage is a Board the Platforms stage, by its class when it is a Remix stage (bonus_fix_3_) and by the vanilla range
 * otherwise.
 *
 * @param is_vanilla_platforms Non-zero for the stages of the vanilla platform range.
 * @param is_vanilla_stage Non-zero for the stages of the vanilla bonus ranges.
 * @param stage_class Entry of the class table.
 */
extern int sc1PBonusRemixRulesIsPlatformStage(int is_vanilla_platforms, int is_vanilla_stage, int stage_class);

/**
 * Whether "L: Set Stage" shows (Bonus.asm check_input_): on the Bonus 1 and 2 screens, with the Remix choice, while a token is held
 * over a fighter.
 *
 * @param is_bonus_12 Non-zero outside the Remix 1P modes.
 * @param is_remix_choice Non-zero when the choice is Remix.
 * @param is_token_held Non-zero while the token is held.
 * @param has_fighter Non-zero when the token is over a fighter.
 */
extern int sc1PBonusRemixRulesSetStageIsVisible(int is_bonus_12, int is_remix_choice, int is_token_held, int has_fighter);

/**
 * X of the stock icons of a stage: a lone icon is centred, a pair is spread.
 *
 * @param icon_count 1 or 2.
 * @param index 0 or 1.
 */
extern float sc1PBonusRemixIconX(int icon_count, int index);

#endif

#endif
