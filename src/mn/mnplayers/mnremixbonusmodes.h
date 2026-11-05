#ifndef _MNREMIXBONUSMODES_H_
#define _MNREMIXBONUSMODES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objtypes.h>

/*
 * What the Bonus 2 character select does for the Remix 1P modes (SinglePlayerMenus.asm, SinglePlayerModes.asm bonus2_css_2_1p,
 * CharacterSelect.asm save_random_char_._bonus and Bonus.asm): the mode title, the records of Multi-Man, Cruel Multi-Man, the
 * Home-Run Contest and Bonus 3, the total line, the Random token with its roll, and the Normal/Remix course choice of Bonus 3.
 * Files are the loaded files of the screen, in the order of dMNPlayers1PBonusFileIDs.
 */

#define MNREMIX_BONUSMODES_FILE_COMMON 0
#define MNREMIX_BONUSMODES_FILE_GAME_MODES 3
#define MNREMIX_BONUSMODES_FILE_1P_MODE 5
#define MNREMIX_BONUSMODES_FILE_DIGITS 9

/** Offset of the title sprite of the mode in the game modes file (header_graphic), 0 to keep the vanilla title. */
extern intptr_t mnRemixBonusModesGetHeaderOffset(void);

/**
 * Makes the record of a Multi-Man, Cruel Multi-Man or Home-Run Contest fighter: the KO or feet label and the number, with the
 * decimal point of the feet (counter_graphic, _ko_amount_or_feet, draw_hrc_high_score_decimal_).
 *
 * @param gobj Record GObj, which already holds nothing.
 * @param files Loaded files of the screen.
 * @param fkind Fighter whose record is shown.
 * @return TRUE when the mode has its own record and it was made, FALSE to make the standard one.
 */
extern sb32 mnRemixBonusModesMakeRecord(GObj *gobj, void **files, s32 fkind);

/** TRUE when the screen belongs to Bonus 3, whose records are finish times. */
extern sb32 mnRemixBonusModesIsBonus3(void);

/**
 * Best finish time of a fighter on the course in use (load_bonus3_time), in ticks.
 *
 * @param fkind Fighter whose record is shown.
 */
extern u32 mnRemixBonusModesGetBonus3Time(s32 fkind);

/**
 * Whether the record under the token is a time (_timedisplay_skip).
 *
 * @param fkind Fighter under the token, REMIX_ID_RANDOM for the Random bookend.
 * @param is_complete TRUE when the fighter has finished the standard bonus.
 */
extern sb32 mnRemixBonusModesShowsTime(s32 fkind, sb32 is_complete);

/** Whether the total line is drawn (total_kos_or_feet_._check). @param is_all_complete TRUE when every fighter finished the bonus. */
extern sb32 mnRemixBonusModesShowsTotal(sb32 is_all_complete);

/**
 * Turns the total line of the standard bonus into the KO or feet total of the mode (total_kos_or_feet_._total_texture).
 *
 * @param gobj Total line GObj.
 * @param label The "TOTAL BEST TIME" sprite already placed by the standard code.
 * @param files Loaded files of the screen.
 * @return TRUE when the line was made here and the standard time digits must be skipped.
 */
extern sb32 mnRemixBonusModesMakeTotal(GObj *gobj, SObj *label, void **files);

/**
 * Replaces the Random token of the player by a fighter and a costume (CharacterSelect.asm save_random_char_._bonus); does nothing
 * for another fighter. Locked fighters and the last six rolls are avoided.
 *
 * @param player Port of the player.
 * @param fkind In: the fighter of the token. Out: the rolled fighter.
 * @param costume In: costume of the token. Out: the rolled costume.
 */
extern void mnRemixBonusModesResolveRandom(s32 player, s32 *fkind, s32 *costume);

/**
 * Cell of the token fighter on the grid, which for the Random token is the Random bookend (portrait_id_table override
 * BOOKEND_RANDOM_PORTRAIT): left and top edge, and size.
 *
 * @param fkind Fighter of the token.
 * @param portrait Grid cell of the fighter (mnRemixCssGetPortrait).
 */
extern f32 mnRemixBonusModesGetCellX(s32 fkind, s32 portrait);
extern f32 mnRemixBonusModesGetCellY(s32 fkind, s32 portrait);
extern f32 mnRemixBonusModesGetCellWidth(s32 fkind);
extern f32 mnRemixBonusModesGetCellHeight(s32 fkind);

/** TRUE when the fighter is one of the bonus bookend list (the ten bonus fighters), whose cell is the bonus bookend. */
extern sb32 mnRemixBonusModesIsBonusBookendFighter(s32 fkind);

/**
 * Z and R held scroll the bonus fighters while the token is over the bonus bookend (CharacterSelect.asm check_variant_input_).
 * Call it every frame for the cursor.
 *
 * @param player Player whose cursor it is.
 * @param held_player Token the cursor holds, negative when it holds none.
 * @param fkind Fighter of the held token.
 * @param held_buttons Held buttons of the cursor controller.
 * @return TRUE on a frame that changed the fighter.
 */
extern sb32 mnRemixBonusModesUpdateScroll(s32 player, s32 held_player, s32 fkind, u16 held_buttons);

/**
 * Shows or hides the scroll hint of the bonus bookend (draw_bonus_scroll_indicators_).
 *
 * @param token Player of the token.
 * @param is_token_held TRUE while the token is held and not yet selected.
 * @param fkind Fighter of the token.
 * @param files Loaded files of the screen.
 */
extern void mnRemixBonusModesUpdateHint(s32 token, sb32 is_token_held, s32 fkind, void **files);

/** TRUE when a hot spot lies on the Random bookend. */
extern sb32 mnRemixBonusModesRandomBookendContains(s32 pos_x, s32 pos_y);

/**
 * Makes the Option panel of Bonus 3: the title, the MODE row and its two arrows (Bonus.asm setup_). Does nothing in the other modes.
 *
 * @param files Loaded files of the screen.
 */
extern void mnRemixBonusModesToggleMake(void **files);

/**
 * Blinks the arrows and draws the MODE row, the STAGE row and "L: Set Stage" for this frame (Bonus.asm arrow_routine_).
 *
 * @param is_token_held TRUE while a token is held.
 * @param fkind Fighter of the token, nFTKindNull when none.
 */
extern void mnRemixBonusModesToggleUpdate(sb32 is_token_held, s32 fkind);

/** TRUE on the Bonus 1 and 2 screens with the Remix choice, where the fighter is not started by the countdown (manual_remix_bonus_start_). */
extern sb32 mnRemixBonusModesIsManualStart(void);

/**
 * L released while a token is held on a fighter sets the STAGE row to the stage of that fighter, or to a random one for the Random
 * token (Bonus.asm check_input_).
 *
 * @param released_buttons Buttons released this frame by the cursor controller.
 * @param is_token_held TRUE while a token is held.
 * @param fkind Fighter of the token.
 * @return TRUE when the stage changed.
 */
extern sb32 mnRemixBonusModesOnButtonRelease(u16 released_buttons, sb32 is_token_held, s32 fkind);

/**
 * A press of the cursor: moves the course choice when it lands on an arrow (Bonus.asm handle_arrow_press_).
 *
 * @param pos_x Cursor hot spot X.
 * @param pos_y Cursor hot spot Y.
 * @return TRUE when an arrow took the press.
 */
extern sb32 mnRemixBonusModesTogglePress(s32 pos_x, s32 pos_y);

#endif

#endif
