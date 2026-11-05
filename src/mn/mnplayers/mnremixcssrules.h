#ifndef _MNREMIXCSSRULES_H_
#define _MNREMIXCSSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix VS character select (src/CharacterSelect.asm), so the headless unit tests (tests/remix) can run
 * them: the C buttons of the team battle, the bonus bookend, the pool of the random pick and the D-pad indicator of the
 * panels (variant icons, flags and the direction mark).
 */

#define MNREMIX_CSS_BONUS_COUNT 10
#define MNREMIX_CSS_BONUS_REPEAT_DELAY 30
#define MNREMIX_CSS_BONUS_REPEAT_STEP 8
#define MNREMIX_CSS_BONUS_REPEAT_IDLE (-1)

/** Index of a bookend in the portrait numbering of the asm (BOOKEND_BONUS_PORTRAIT, one past the 30 grid slots). */
#define MNREMIX_CSS_BONUS_BOOKEND_SLOT 30

/** Left edge, top edge, width and height in screen units of the bonus bookend (portrait_x_position, draw_portraits_). */
#define MNREMIX_CSS_BOOKEND_X 24
#define MNREMIX_CSS_BOOKEND_Y 44
#define MNREMIX_CSS_BOOKEND_WIDTH 15
#define MNREMIX_CSS_BOOKEND_HEIGHT 72

/** Scale of the bookend image, 16x96 in the file (draw_portraits_: x 0x3F70, y 0x3F42). */
#define MNREMIX_CSS_BONUS_BOOKEND_SCALE_X 0.9375F
#define MNREMIX_CSS_BONUS_BOOKEND_SCALE_Y 0.7578125F

/** X of the D-pad of panel 0 and the distance between two panels (css_settings, VS row: X padding 34, panel offset 69). */
#define MNREMIX_CSS_DPAD_X 34
#define MNREMIX_CSS_DPAD_PANEL_STRIDE 69
#define MNREMIX_CSS_DPAD_Y 177

typedef enum MNRemixCssDpadDirection
{
    MNREMIX_CSS_DPAD_UP,
    MNREMIX_CSS_DPAD_DOWN,
    MNREMIX_CSS_DPAD_LEFT,
    MNREMIX_CSS_DPAD_RIGHT,
    MNREMIX_CSS_DPAD_DIRECTION_COUNT
} MNRemixCssDpadDirection;

/** The image a variant of a hovered fighter shows around the D-pad (draw_variant_icon_). */
typedef enum MNRemixCssVariantIcon
{
    MNREMIX_CSS_ICON_NONE,
    MNREMIX_CSS_ICON_FLAG_J,
    MNREMIX_CSS_ICON_FLAG_E,
    MNREMIX_CSS_ICON_POLYGON,
    MNREMIX_CSS_ICON_METAL,
    MNREMIX_CSS_ICON_PIANO,
    MNREMIX_CSS_ICON_GBOWSER,
    MNREMIX_CSS_ICON_SSONIC,
    MNREMIX_CSS_ICON_MASTER_HAND,
    MNREMIX_CSS_ICON_PEPPY,
    MNREMIX_CSS_ICON_SLIPPY,
    MNREMIX_CSS_ICON_MLUIGI,
    MNREMIX_CSS_ICON_EBI,
    MNREMIX_CSS_ICON_DRAGONKING,
    MNREMIX_CSS_ICON_DSAMUS,
    MNREMIX_CSS_ICON_LUCAS,
    MNREMIX_CSS_ICON_ROY,
    MNREMIX_CSS_ICON_DONKEY_STOCK
} MNRemixCssVariantIcon;

/** Number of handicap values drawn as an icon: 10 to 40 (AI.asm handicap.table). */
#define MNREMIX_CSS_HANDICAP_ICON_COUNT 31

/** Source of a handicap icon that is not the stock icon of a fighter (AI.asm handicap.type). */
#define MNREMIX_CSS_HANDICAP_SOURCE_POLYGON (-1)
#define MNREMIX_CSS_HANDICAP_SOURCE_METAL (-2)
#define MNREMIX_CSS_HANDICAP_SOURCE_MASTER_HAND (-3)
#define MNREMIX_CSS_HANDICAP_SOURCE_NONE (-4)

/**
 * TRUE when the four C buttons pick the costume of a panel (Costumes.asm disable_update). Without teams they always do;
 * with teams they do only for a fighter that was already selected and whose token nobody holds, otherwise the buttons
 * keep the team-battle behaviour of the vanilla CSS (pick the fighter under the token, no costume change).
 *
 * @param is_team_battle TRUE in a team battle.
 * @param is_fighter_selected TRUE when the panel has a selected fighter (slot is_fighter_selected).
 * @param held_player Port of the hand that holds the token of the panel, -1 when nobody holds it.
 */
extern int mnRemixCssCostumeButtonsActive(int is_team_battle, int is_fighter_selected, int held_player);

/** Offset from the cursor sprite position to the point that presses an image (CharacterSelect.asm check_press_: x + 20, y + 3). */
#define MNREMIX_CSS_PRESS_OFFSET_X 20.0F
#define MNREMIX_CSS_PRESS_OFFSET_Y 3.0F

/**
 * TRUE when a cursor presses an image (CharacterSelect.asm check_press_, every edge inclusive).
 *
 * @param cursor_x Position x of the cursor sprite.
 * @param cursor_y Position y of the cursor sprite.
 * @param image_x Position x of the image sprite.
 * @param image_y Position y of the image sprite.
 * @param image_width Width of the image sprite.
 * @param image_height Height of the image sprite.
 */
extern int mnRemixCssCursorPressesImage(float cursor_x, float cursor_y, float image_x, float image_y, int image_width, int image_height);

/**
 * Panel whose costume the C buttons change when a cursor presses with them (Costumes.asm cpu_costumes_): the first CPU
 * panel, in port order and other than the panel of the cursor, that the cursor is over; the own panel when there is none.
 *
 * @param own_panel Panel of the cursor.
 * @param is_cpu_panel Per panel, non-zero when the panel is a CPU panel.
 * @param is_cursor_over Per panel, non-zero when the cursor is over the panel image.
 * @param panel_count Number of panels.
 */
extern int mnRemixCssCpuCostumePanel(int own_panel, const int *is_cpu_panel, const int *is_cursor_over, int panel_count);

/** Index of the Random bookend in the portrait numbering of the asm (BOOKEND_RANDOM_PORTRAIT, one past the bonus bookend). */
#define MNREMIX_CSS_RANDOM_BOOKEND_SLOT 31

/** Left edge of the Random bookend: right of the grid (START_VISUAL + START_X + PORTRAIT_WIDTH * NUM_COLUMNS), same top and size as the bonus one. */
#define MNREMIX_CSS_RANDOM_BOOKEND_X 279

/** Position of a token dropped on the Random bookend (token_autoposition_ _random_bookend: RANDOM_TOKEN_X 278, BOOKEND_TOKEN_Y 70). */
#define MNREMIX_CSS_RANDOM_TOKEN_X 278.0F
#define MNREMIX_CSS_RANDOM_TOKEN_Y 70.0F

/** Entries of the recent randoms list of a player (CharacterSelect.asm RRL_NUM_SLOTS). */
#define MNREMIX_CSS_RECENT_RANDOMS_SLOTS 6

/** Fighters a player's last Random rolls came out as, and the slot the next roll writes to. */
typedef struct MNRemixCssRecentRandoms
{
    signed char entries[MNREMIX_CSS_RECENT_RANDOMS_SLOTS];
    int index;
} MNRemixCssRecentRandoms;

/** TRUE when the point (pos_x, pos_y), the centre of a token, is over the Random bookend right of the grid. */
extern int mnRemixCssRandomBookendContains(int pos_x, int pos_y);

/** Empties a recent randoms list (the asm table starts as -1 in every entry and index 0). */
extern void mnRemixCssRecentRandomsReset(MNRemixCssRecentRandoms *recent);

/**
 * Writes a pending roll into the slot at the index and counts it (_recent_randoms_lookup).
 *
 * @param recent List of the player.
 * @param fkind Fighter just rolled.
 * @return Entries equal to fkind, the pending one included, so 1 when the fighter is new in the list.
 */
extern int mnRemixCssRecentRandomsStore(MNRemixCssRecentRandoms *recent, int fkind);

/**
 * Size of the dice that decides whether a repeated roll stays: 0 when the fighter is new in the list (no dice), otherwise the match
 * count, so the roll is kept with probability 1 / matches (50% for one earlier match, 33% for two).
 */
extern int mnRemixCssRecentRandomsDiceRange(int matches);

/** Moves the index to the next slot, wrapping after the last, once a roll was kept. */
extern void mnRemixCssRecentRandomsCommit(MNRemixCssRecentRandoms *recent);

/**
 * Shade of a team fighter that Random picked (save_random_char_._vs, _teams_loop): the number of earlier ports that hold the same
 * fighter on the same team, so two teammates with the same fighter do not share a colour.
 *
 * @param port Port being resolved.
 * @param fkinds Fighter of every port; entries before `port` are already resolved.
 * @param teams Team of every port.
 */
extern int mnRemixCssRandomTeamShade(int port, const int *fkinds, const int *teams);

/** TRUE when the point (pos_x, pos_y), the centre of a token, is over the bonus bookend left of the grid. */
extern int mnRemixCssBonusBookendContains(int pos_x, int pos_y);

/**
 * Index in the bonus fighter list after one Z (step -1) or R (step +1) change; it wraps at both ends
 * (CharacterSelect.asm check_variant_input_ _set_bonus_char).
 */
extern int mnRemixCssBonusStep(int index, int step);

/** Z held goes one fighter back, R held one forward and Z wins when both are held; 0 when neither is held. */
extern int mnRemixCssBonusHeldStep(int is_z_held, int is_r_held);

/**
 * Repeat of the bonus scroll while Z or R stay held: the first frame changes the fighter, then nothing until 30 frames
 * passed, then one change every 8 frames. Returns 1 on a frame that changes the fighter.
 *
 * @param timer Per-token timer, MNREMIX_CSS_BONUS_REPEAT_IDLE while neither button was held.
 * @param is_held TRUE while Z or R is held.
 */
extern int mnRemixCssBonusRepeat(int *timer, int is_held);

/**
 * Number of entries of the random pool (get_random_char_id_): the grid slots, plus the bonus fighters when the Random Select
 * toggle is not the default (value 0).
 */
extern int mnRemixCssRandomPoolSize(int random_select, int slot_count);

/** TRUE when entry `index` of the random pool is a bonus fighter, that is an entry past the grid slots. */
extern int mnRemixCssRandomPoolIsBonus(int index, int slot_count);

/** X of the D-pad of a panel. */
extern int mnRemixCssDpadPanelX(int panel);

/** Offset from the D-pad corner to the variant image of a direction (draw_variant_icon_). */
extern void mnRemixCssDpadIconOffset(int direction, int *offset_x, int *offset_y);

/** Offset from the D-pad corner to the yellow mark of a direction (setup_). */
extern void mnRemixCssDpadMarkOffset(int direction, int *offset_x, int *offset_y);

/**
 * Image of a variant (draw_variant_icon_): the J or E flag by variant type, the polygon icon, the stock icon of the
 * variant fighter, or the stock icon of Donkey Kong for a variant with no icon of its own.
 *
 * @param variant_type RemixVariantType of the variant fighter.
 * @param variant_id Remix id of the variant fighter.
 */
extern MNRemixCssVariantIcon mnRemixCssVariantIconKind(int variant_type, int variant_id);

/**
 * Offset of the image of a variant icon in the Remix CSS images file (CharacterSelect.asm VARIANT_ICON_OFFSET, the `+ 0x10`
 * entries pointing at the Sprite header), or 0 for MNREMIX_CSS_ICON_NONE and for the icons taken from a fighter file.
 */
extern unsigned int mnRemixCssVariantIconOffset(MNRemixCssVariantIcon icon);

/** Scale of a variant image: flags 0.625, the Master Hand icon 0.5625, every other icon 1. */
extern float mnRemixCssVariantIconScale(MNRemixCssVariantIcon icon);

/**
 * What a handicap of 10 to 40 shows instead of a digit (AI.asm render_10_._handicap, handicap.table): the Remix id of the fighter
 * whose stock icon is drawn, or one of the MNREMIX_CSS_HANDICAP_SOURCE_* codes. Five consecutive values share a source, one per
 * level. Returns MNREMIX_CSS_HANDICAP_SOURCE_NONE for an index outside the table.
 *
 * @param index Handicap value minus 10.
 */
extern int mnRemixCssHandicapIconSource(int index);

/** Level (0 none, 1 very easy to 5 very hard) of a handicap icon index; 0 for the last entry and for an index outside the table. */
extern int mnRemixCssHandicapIconLevel(int index);

/** Colour 0xRRGGBBAA of the bar under a handicap icon for a level from 1 to 5; 0 for level 0, which draws no bar. */
extern unsigned int mnRemixCssHandicapLevelColor(int level);

/** TRUE when a D-pad is shown for a hovered fighter: when at least one of its four variants exists. */
extern int mnRemixCssHasVariants(const int *variant_ids, int none_id);

#endif

#endif
