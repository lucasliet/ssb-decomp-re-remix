#ifndef _MNREMIXBONUSMODESRULES_H_
#define _MNREMIXBONUSMODESRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of what the Bonus 2 character select shows for the Remix 1P modes (SinglePlayerMenus.asm header_graphic, counter_graphic,
 * _ko_amount_or_feet, _timedisplay_skip, total_kos_or_feet_, draw_hrc_high_score_decimal_ and load_bonus3_time), kept apart so the
 * headless unit tests (tests/remix) run them. Modes are the SC1PREMIX_MODE_* ids.
 */

/* header_graphic: offsets of the mode title in the game modes file (0 keeps the Board the Platforms title). */
#define MNREMIX_BONUSMODES_HEADER_BONUS3 0x1DD8
#define MNREMIX_BONUSMODES_HEADER_MULTIMAN 0x14D8
#define MNREMIX_BONUSMODES_HEADER_CRUEL 0x1958
#define MNREMIX_BONUSMODES_HEADER_HRC 0x24C8

/* counter_graphic: offsets of the label beside the record number in the 1P mode file. */
#define MNREMIX_BONUSMODES_COUNTER_KOS 0x34E8
#define MNREMIX_BONUSMODES_COUNTER_FEET 0x3C08

/* _ko_amount_or_feet: digits of the record number; total_kos_or_feet_: digits of the total. */
#define MNREMIX_BONUSMODES_VANILLA_DIGITS 2
#define MNREMIX_BONUSMODES_KO_DIGITS 5
#define MNREMIX_BONUSMODES_FEET_DIGITS 7
#define MNREMIX_BONUSMODES_KO_TOTAL_DIGITS 6
#define MNREMIX_BONUSMODES_FEET_TOTAL_DIGITS 8

/* total_kos_or_feet_._total_texture: the "TOTAL" label is cut to 24 wide at x 235 and the sum is drawn right aligned at x 225, y 205. */
#define MNREMIX_BONUSMODES_TOTAL_LABEL_WIDTH 24
#define MNREMIX_BONUSMODES_TOTAL_LABEL_X 235.0F
#define MNREMIX_BONUSMODES_TOTAL_NUMBER_X 225.0F
#define MNREMIX_BONUSMODES_TOTAL_NUMBER_Y 205.0F

/* total_kos_or_feet_._total_texture: a record slot holding this filler is a console memory fix and counts as zero. */
#define MNREMIX_BONUSMODES_RECORD_FILLER 0xAAAAAAAAu

/* draw_hrc_high_score_decimal_: the tenths digit moves 4 to the right, a period (the colon image cut to 5 rows) sits at x 216, 5 below the digits. */
#define MNREMIX_BONUSMODES_DECIMAL_SHIFT 4.0F
#define MNREMIX_BONUSMODES_DECIMAL_X 216.0F
#define MNREMIX_BONUSMODES_DECIMAL_Y_OFFSET 5.0F
#define MNREMIX_BONUSMODES_DECIMAL_ROWS 5

/* Bonus.asm setup_: layout of the Option panel of Bonus 3 (Option title, MODE row, arrows) in screen units. */
#define MNREMIX_BONUSMODES_OPTION_X 198.0F
#define MNREMIX_BONUSMODES_OPTION_Y 129.0F
#define MNREMIX_BONUSMODES_OPTION_TINT 0xAFB1CCFFu
#define MNREMIX_BONUSMODES_OPTION_PALETTE 0x000000FFu
/* Bonus.asm setup_: filled bar behind the Option title (slot 46 of the HUD rectangles) and the outline sprite, which is the one of the 1P game Option box (cms wrap, cmt mirror, maskt 5, lrs 184, lrt 64) */
#define MNREMIX_BONUSMODES_BAR_SLOT 46
#define MNREMIX_BONUSMODES_BAR_X 175.0F
#define MNREMIX_BONUSMODES_BAR_Y 136.0F
#define MNREMIX_BONUSMODES_BAR_WIDTH 144.0F
#define MNREMIX_BONUSMODES_BAR_HEIGHT 5.0F
#define MNREMIX_BONUSMODES_BAR_RGBA 0x576088FFu
#define MNREMIX_BONUSMODES_OUTLINE_X 146.0F
#define MNREMIX_BONUSMODES_OUTLINE_Y 139.0F
#define MNREMIX_BONUSMODES_OUTLINE_LRS 184
#define MNREMIX_BONUSMODES_OUTLINE_LRT 64
#define MNREMIX_BONUSMODES_OUTLINE_MASKT 5
#define MNREMIX_BONUSMODES_MODE_LABEL_X 208.0F
#define MNREMIX_BONUSMODES_MODE_NAME_X 249.0F
#define MNREMIX_BONUSMODES_MODE_Y 156.0F
#define MNREMIX_BONUSMODES_MODE_SCALE 0.875F
#define MNREMIX_BONUSMODES_MODE_LABEL_RGBA 0xC5B6A7FFu
#define MNREMIX_BONUSMODES_MODE_NORMAL_RGBA 0xE4BE41FFu
#define MNREMIX_BONUSMODES_MODE_REMIX_RGBA 0xE44141FFu
#define MNREMIX_BONUSMODES_ARROW_LEFT_X 212.0F
#define MNREMIX_BONUSMODES_ARROW_RIGHT_X 280.0F
#define MNREMIX_BONUSMODES_ARROW_Y 157.0F
/* check_image_footer_press_ paddings: none on the side facing away from the other arrow, 8 on the side facing it */
#define MNREMIX_BONUSMODES_ARROW_OUTER_PAD 0
#define MNREMIX_BONUSMODES_ARROW_INNER_PAD 8
#define MNREMIX_BONUSMODES_MODE_COUNT 2
#define MNREMIX_BONUSMODES_BLINK_PERIOD 20
#define MNREMIX_BONUSMODES_BLINK_ON_FRAMES 11

/**
 * Course after a press of an arrow: the choice moves one step and stops at the first and last (handle_arrow_press_ only checks the
 * arrow that exists for the current choice).
 *
 * @param mode Current choice, 0 for Normal and 1 for Remix.
 * @param direction -1 for the left arrow, 1 for the right arrow.
 */
extern int mnRemixBonusModesToggleStep(int mode, int direction);

/**
 * Whether an arrow of the MODE row exists: the left one when the choice is not the first, the right one when it is not the last.
 *
 * @param mode Current choice.
 * @param is_right Non-zero for the right arrow.
 */
extern int mnRemixBonusModesToggleArrowExists(int mode, int is_right);

/** Blink timer after a frame (arrow_routine_): counts 0 to 19. */
extern int mnRemixBonusModesToggleBlinkNext(int timer);

/** Whether the arrows are drawn for a blink timer: the first 11 frames of 20. */
extern int mnRemixBonusModesToggleBlinkIsOn(int timer);

/**
 * Whether a point is on an arrow, with the paddings of check_image_footer_press_.
 *
 * @param pos_x Hot spot X.
 * @param pos_y Hot spot Y.
 * @param arrow_x Left edge of the arrow.
 * @param arrow_y Top edge of the arrow.
 * @param width Width of the arrow sprite.
 * @param height Height of the arrow sprite.
 * @param is_right Non-zero for the right arrow (padding on its left side only).
 */
extern int mnRemixBonusModesToggleArrowHit(int pos_x, int pos_y, int arrow_x, int arrow_y, int width, int height, int is_right);

/**
 * Offset of the mode title in the game modes file.
 *
 * @param mode SC1PREMIX_MODE_*.
 * @return The offset, or 0 for the standard bonus stages and for modes that never reach this screen.
 */
extern int mnRemixBonusModesHeaderOffset(int mode);

/* header_graphic_remix_1p: offsets of the title of the 1P game character select in the 1P mode file (0 keeps the 1P Game title). */
#define MNREMIX_BONUSMODES_GAME_HEADER_REMIX_1P 0x3758
#define MNREMIX_BONUSMODES_GAME_HEADER_ALLSTAR 0x39C8

/* _1P_FGM_alter and _BTP_FGM_alter: announcer voice ids of the character selects (0 keeps the vanilla voice). */
#define MNREMIX_BONUSMODES_FGM_ALLSTAR 1014
#define MNREMIX_BONUSMODES_FGM_BONUS3 0x1EF
#define MNREMIX_BONUSMODES_FGM_MULTIMAN 0x3C7
#define MNREMIX_BONUSMODES_FGM_CRUEL 0x3C8
#define MNREMIX_BONUSMODES_FGM_HRC 1015

/**
 * Title offset of the 1P game character select (header_graphic_remix_1p).
 *
 * @param mode SC1PREMIX_MODE_* id.
 * @return Offset in the 1P mode file, 0 for a mode that keeps the 1P Game title.
 */
extern int mnRemixBonusModesGameHeaderOffset(int mode);

/**
 * Announcer voice of the 1P game character select (_1P_FGM_alter).
 *
 * @param mode SC1PREMIX_MODE_* id.
 * @return FGM id, 0 for a mode that keeps the "choose your character" voice.
 */
extern int mnRemixBonusModesGameAnnounceFgm(int mode);

/**
 * Announcer voice of the Bonus 2 character select (_BTP_FGM_alter).
 *
 * @param mode SC1PREMIX_MODE_* id.
 * @return FGM id, 0 for a mode that keeps the "board the platforms" voice.
 */
extern int mnRemixBonusModesBonusAnnounceFgm(int mode);

/**
 * Screen and mode after a press of the header of the Bonus 2 character select (set_original_flag_swap): Bonus 1 goes to Bonus 2,
 * Bonus 2 to Bonus 3, Bonus 3 back to Bonus 1, Multi-Man to Cruel Man and back, and the Home-Run Contest stays.
 *
 * @param mode SC1PREMIX_MODE_* id now.
 * @param kind 0 on the Break the Targets screen, 1 on the Board the Platforms screen.
 * @param next_mode Receives the mode after the press.
 * @param next_kind Receives the screen after the press.
 * @return Non-zero when the press changes the screen, zero when it does nothing.
 */
extern int mnRemixBonusModesCycleHeader(int mode, int kind, int *next_mode, int *next_kind);

/**
 * Offset of the label beside the record number in the 1P mode file.
 *
 * @param mode SC1PREMIX_MODE_*.
 * @return The offset, or 0 to keep the Platforms label.
 */
extern int mnRemixBonusModesCounterOffset(int mode);

/**
 * Digits of the record number: 5 for the KO counts, 7 for the Home-Run Contest feet, 2 for the platform count.
 *
 * @param mode SC1PREMIX_MODE_*.
 */
extern int mnRemixBonusModesRecordDigits(int mode);

/**
 * Whether the record shown is a time (the race and a finished bonus) or a count (KOs, feet, platforms), the _timedisplay_skip rule:
 * Bonus 3 always shows its time, the other Remix modes their count, and a standard bonus shows the count for the Random token and for
 * a fighter that has not finished it.
 *
 * @param mode SC1PREMIX_MODE_*.
 * @param is_random Non-zero when the token is on the Random bookend.
 * @param is_complete Non-zero when the fighter has finished the bonus.
 */
extern int mnRemixBonusModesRulesShowsTime(int mode, int is_random, int is_complete);

/**
 * Whether the total line is drawn: the Multi-Man, Cruel Multi-Man and Home-Run Contest screens always show it, the others when every
 * fighter has finished the bonus.
 *
 * @param mode SC1PREMIX_MODE_*.
 * @param is_all_complete Non-zero when every fighter has finished the bonus.
 */
extern int mnRemixBonusModesRulesShowsTotal(int mode, int is_all_complete);

/**
 * Digits of the total number (6 for KOs, 8 for feet); 0 for the modes without a total of their own.
 *
 * @param mode SC1PREMIX_MODE_*.
 */
extern int mnRemixBonusModesTotalDigits(int mode);

/**
 * Sum of a record table over every fighter (total_kos_or_feet_._loop): slots holding the console filler count as zero.
 *
 * @param records Record of each fighter.
 * @param count Number of fighters.
 */
extern unsigned int mnRemixBonusModesSumRecords(const unsigned int *records, int count);

/**
 * Whether the record is drawn with a decimal point: only the Home-Run Contest, whose feet are kept in tenths.
 *
 * @param mode SC1PREMIX_MODE_*.
 */
extern int mnRemixBonusModesHasDecimal(int mode);

#endif

#endif
