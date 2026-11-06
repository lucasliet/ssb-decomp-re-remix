#ifndef _MNREMIXCSSPANELMENURULES_H_
#define _MNREMIXCSSPANELMENURULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the per-port panel menu of the Remix character select screens (src/CharacterSelectDebugMenu.asm and the css/X.asm
 * items), so the headless unit tests (tests/remix) can run them: which items each CSS lists, the pages, the value wrap, the layout
 * of the rows and arrows, the blink and the hold-A repeat. Items are numbered in the order of add_menu_item (the asm 2720-2739),
 * which is also the order of RemixPanelOption in port/remix/remix_player_panel.h.
 */

#define MNREMIX_PANELMENU_ITEM_COUNT 20
#define MNREMIX_PANELMENU_ROWS 3

typedef enum MNRemixPanelMenuMode
{
    MNREMIX_PANELMENU_MODE_VS,
    MNREMIX_PANELMENU_MODE_1P,
    MNREMIX_PANELMENU_MODE_TRAINING,
    MNREMIX_PANELMENU_MODE_BONUS,
    MNREMIX_PANELMENU_MODE_ALLSTAR,
    MNREMIX_PANELMENU_MODE_COUNT
} MNRemixPanelMenuMode;

typedef enum MNRemixPanelMenuItem
{
    MNREMIX_PANELMENU_ITEM_SHIELD,
    MNREMIX_PANELMENU_ITEM_PLAYER_TAG,
    MNREMIX_PANELMENU_ITEM_VISIBILITY,
    MNREMIX_PANELMENU_ITEM_SKELETON,
    MNREMIX_PANELMENU_ITEM_MODEL,
    MNREMIX_PANELMENU_ITEM_INPUT_HUD,
    MNREMIX_PANELMENU_ITEM_SIZE,
    MNREMIX_PANELMENU_ITEM_STOCK,
    MNREMIX_PANELMENU_ITEM_KNOCKBACK,
    MNREMIX_PANELMENU_ITEM_DELAY,
    MNREMIX_PANELMENU_ITEM_HANDICAP,
    MNREMIX_PANELMENU_ITEM_START_WITH,
    MNREMIX_PANELMENU_ITEM_TAUNT_ITEM,
    MNREMIX_PANELMENU_ITEM_TAUNT_BUTTON,
    MNREMIX_PANELMENU_ITEM_KIRBY_HAT,
    MNREMIX_PANELMENU_ITEM_DPAD_MAP,
    MNREMIX_PANELMENU_ITEM_DPAD_CONTROL,
    MNREMIX_PANELMENU_ITEM_DAMAGE,
    MNREMIX_PANELMENU_ITEM_POISON,
    MNREMIX_PANELMENU_ITEM_PRACTICE
} MNRemixPanelMenuItem;

/* Layout in screen units, from the panel card (render_menu_page_, create_item_*, render_pagination_, update_debug_menu_). */
#define MNREMIX_PANELMENU_ROW_HEIGHT 24
#define MNREMIX_PANELMENU_LABEL_X 4.0F
#define MNREMIX_PANELMENU_LABEL_Y 18.0F
#define MNREMIX_PANELMENU_VALUE_Y 30.0F
#define MNREMIX_PANELMENU_TEXT_SCALE 0.875F
#define MNREMIX_PANELMENU_LABEL_RGBA 0xD0D0D0FF
#define MNREMIX_PANELMENU_VALUE_RGBA 0xFFFFFFFF
#define MNREMIX_PANELMENU_LABEL_INSET 8
#define MNREMIX_PANELMENU_VALUE_INSET 24
#define MNREMIX_PANELMENU_ITEM_ARROW_LEFT_X 2.0F
#define MNREMIX_PANELMENU_ITEM_ARROW_RIGHT_INSET 5.0F
#define MNREMIX_PANELMENU_ITEM_ARROW_Y 32.0F
#define MNREMIX_PANELMENU_PAGE_NUMBER_INSET 10.5F
#define MNREMIX_PANELMENU_PAGE_NUMBER_Y 4.0F
#define MNREMIX_PANELMENU_PAGE_ARROW_LEFT_INSET 20.0F
#define MNREMIX_PANELMENU_PAGE_ARROW_RIGHT_INSET 5.0F
#define MNREMIX_PANELMENU_PAGE_ARROW_Y 6.0F
#define MNREMIX_PANELMENU_BUTTON_X 4.0F
#define MNREMIX_PANELMENU_RESET_X 20.0F
#define MNREMIX_PANELMENU_BUTTON_Y 130.0F
#define MNREMIX_PANELMENU_BUTTON_SCALE 0.75F
#define MNREMIX_PANELMENU_ARROW_SCALE 0.625F
#define MNREMIX_PANELMENU_ARROW_TINT 0xFF0000FF
#define MNREMIX_PANELMENU_ARROW_PALETTE 0x303030FF

/* Hit padding of the arrows (check_image_footer_press_ left and right padding in handle_item_arrow_press_, handle_page_arrow_press_). */
#define MNREMIX_PANELMENU_ITEM_ARROW_LEFT_PAD 8
#define MNREMIX_PANELMENU_PAGE_ARROW_LEFT_PAD 3
#define MNREMIX_PANELMENU_PAGE_ARROW_RIGHT_PAD 4

/* Arrow blink: the timer counts 0 to 19 and the arrow shows while it is below 11 (arrow_state_routine_). */
#define MNREMIX_PANELMENU_BLINK_PERIOD 20
#define MNREMIX_PANELMENU_BLINK_ON_FRAMES 11

/* Hold A (hold_a_handler_): frames before the repeat starts, before it runs every frame, and frames between two repeats. */
#define MNREMIX_PANELMENU_HOLD_BUFFER 15
#define MNREMIX_PANELMENU_HOLD_FAST_BUFFER 90
#define MNREMIX_PANELMENU_HOLD_TIMER 6
#define MNREMIX_PANELMENU_SOUND_BUFFER 3

typedef struct MNRemixPanelMenuHold
{
    unsigned int held_frames;
    unsigned int repeat_timer;
} MNRemixPanelMenuHold;

/**
 * Items a panel lists, in menu order (add_menu_item and write_menu_items). The VS and Training CSS list one set for a human panel and
 * another for a CPU panel (the human/CPU bitmask); the 1P, Bonus and All-Star CSS list the human set whatever the bitmask says.
 *
 * @param mode MNRemixPanelMenuMode of the screen.
 * @param is_cpu Non-zero for a CPU (or empty) panel of the VS or Training CSS.
 * @param items Receives the item numbers.
 * @param capacity Size of items.
 * @return Number of items written.
 */
extern int mnRemixPanelMenuRulesListItems(int mode, int is_cpu, int *items, int capacity);

/**
 * Items the reset button sets to their default (handle_reset_button_press_ walks the human array, then the CPU one): every item
 * of either list of the mode, once.
 */
extern int mnRemixPanelMenuRulesListResetItems(int mode, int *items, int capacity);

extern int mnRemixPanelMenuRulesItemMin(int item);
extern int mnRemixPanelMenuRulesItemMax(int item);
extern int mnRemixPanelMenuRulesItemDefault(int item);

/** Label of an item (the css/X.asm LABEL). */
extern const char *mnRemixPanelMenuRulesItemLabel(int item);

/** Non-zero when the value is shown as a number instead of a name (only Damage, value_type.NUMERIC). */
extern int mnRemixPanelMenuRulesItemIsNumeric(int item);

/** Non-zero when a value other than the default turns the 1P high scores off (DISABLES_HIGH_SCORES). */
extern int mnRemixPanelMenuRulesItemDisablesHighScores(int item);

/** Highest page number, 0 based (setup_menu_page_: 3 items per page). */
extern int mnRemixPanelMenuRulesMaxPage(int item_count);

/** Page kept in range after the item list changed (setup_menu_page_ _get_current_page). */
extern int mnRemixPanelMenuRulesClampPage(int page, int max_page);

/** Page after a page arrow press: wraps over 0..max_page (handle_page_arrow_press_). */
extern int mnRemixPanelMenuRulesStepPage(int page, int direction, int max_page);

/** Items on a page, 0 to 3 (the rows that exist). */
extern int mnRemixPanelMenuRulesRowsOnPage(int page, int item_count);

/** Value after an item arrow press: wraps over min..max (handle_item_arrow_press_ _update_value). */
extern int mnRemixPanelMenuRulesStepValue(int value, int direction, int min, int max);

/** Blink timer after a frame (arrow_state_routine_, page_arrow_routine_). */
extern int mnRemixPanelMenuRulesBlinkNext(int timer);

/** Non-zero while an arrow is drawn for the blink timer. */
extern int mnRemixPanelMenuRulesBlinkIsVisible(int timer);

/**
 * One frame of the hold A repeat (hold_a_handler_): non-zero when the held press repeats this frame. The first frames count as
 * a plain press (the caller handles the tap itself), then every 6th frame repeats, and from 90 frames on every frame does.
 *
 * @param hold Counters of the port.
 * @param is_held Non-zero while A is held.
 */
extern int mnRemixPanelMenuRulesHoldAdvance(MNRemixPanelMenuHold *hold, int is_held);

/** Restarts the wait of the hold repeat; a page arrow press does it so pages do not scroll fast (handle_page_arrow_press_ _update_page). */
extern void mnRemixPanelMenuRulesHoldRestart(MNRemixPanelMenuHold *hold);

/** Row top Y offset from the panel top, for label (is_value zero) or value (is_value non-zero). */
extern float mnRemixPanelMenuRulesRowTextY(int row, int is_value);

/** Y offset of the arrows of a row from the panel top. */
extern float mnRemixPanelMenuRulesRowArrowY(int row);

/** X of the left or right arrow of an item row (create_item_arrows_: 2 from the left edge, 5 in from the right edge). */
extern float mnRemixPanelMenuRulesItemArrowX(float panel_x, int panel_width, int is_right);

/** X of the left or right page arrow (render_pagination_: 20 and 5 in from the right edge). */
extern float mnRemixPanelMenuRulesPageArrowX(float panel_x, int panel_width, int is_right);

/**
 * Value of the damage item once it is over the Stamina HP (Damage.asm onchange_handler): a right press goes to 0, a left press to
 * the HP.
 *
 * @param value Value after the press.
 * @param direction -1 or 1.
 * @param total_hp Stamina.TOTAL_HP.
 */
extern int mnRemixPanelMenuRulesStaminaDamage(int value, int direction, int total_hp);

/**
 * TRUE when the Stock item may leave Default (StockMode.asm onchange_handler): only the plain, 12-Character Battle, Tug of War and
 * Tag Team modes keep the value.
 *
 * @param keeps_value Non-zero for those modes.
 * @param value Value after the press.
 */
extern int mnRemixPanelMenuRulesStockValue(int keeps_value, int value);

/*
 * Toggles that turn the 1P high scores off (check_high_scores_enabled, _check_toggles). The Gameplay and Pokemon toggles are
 * packed into SRAM words (Toggles.asm update_block_size, 1016-1038), lowest bit first, and the asm compares the words with their defaults
 * (CharacterSelectDebugMenu.asm 2381-2406). Gameplay word 1 mask 0xFFEFFF7F clears bit 20 (Improved AI, Toggles.asm 2428, after
 * Hitstun 1 bit, Hitlag 3, DI 2, Japanese Sounds 2 on bits 6-7, J Stun 1, Momentum 1, Shield Stun 3, Z-Cancel 3, Punish 4) and bit 7
 * (Japanese Sounds value 2). Pokemon: the Pokemon SFX toggle takes bits 0-1 (Toggles.asm 2623, mask 0xFFFC drops them) and the 13
 * Pokemon booleans (2625-2637) bits 2-14, all on by default (0x7FFC).
 */
#define MNREMIX_PANELMENU_GAMEPLAY_WORDS 2
#define MNREMIX_PANELMENU_GAMEPLAY_WORD_1_MASK 0xFFEFFF7FU
#define MNREMIX_PANELMENU_GAMEPLAY_WORD_1_DEFAULT 0x00000000U
#define MNREMIX_PANELMENU_GAMEPLAY_WORD_2_DEFAULT 0x00000000U
#define MNREMIX_PANELMENU_POKEMON_WORD_MASK 0xFFFCU
#define MNREMIX_PANELMENU_POKEMON_WORD_DEFAULT 0x7FFCU

/** Values of the globals check_high_scores_enabled reads besides the panel items. */
typedef struct MNRemixPanelMenuHighScoreGlobals
{
    unsigned int gameplay_words[MNREMIX_PANELMENU_GAMEPLAY_WORDS];
    unsigned int pokemon_word;
    int enemy_port;
} MNRemixPanelMenuHighScoreGlobals;

/**
 * Packs the toggles of a group into SRAM words the way Toggles.asm lays them out: each toggle takes the bits its highest value needs
 * (update_block_size_based_on_max), lowest bit first, and a toggle that would end past or on a word boundary starts the next word
 * (update_block_size compares block_size / 32 before and after the add). Toggles that do not fit in the given words are dropped.
 *
 * @param values Current value of each toggle in menu order.
 * @param maxima Highest value of each toggle.
 * @param count Number of toggles.
 * @param words Receives the packed words (cleared first).
 * @param word_count Number of words.
 */
extern void mnRemixPanelMenuRulesPackToggles(const int *values, const int *maxima, int count, unsigned int *words, int word_count);

/** Bits a toggle takes in its SRAM block (update_block_size_based_on_max). */
extern int mnRemixPanelMenuRulesToggleBitWidth(int maximum);

/**
 * Non-zero when the Gameplay and Pokemon toggles leave the high scores on (_check_toggles): the first Gameplay word equals its default
 * once the Improved AI and Japanese Sounds "never" bits are ignored, the second Gameplay word is zero, and the Pokemon word equals its
 * default once the sound effect bits are ignored.
 */
extern int mnRemixPanelMenuRulesTogglesKeepHighScores(const MNRemixPanelMenuHighScoreGlobals *globals);

/**
 * Non-zero when the 1P high scores stay on (check_high_scores_enabled): every item that disables them is at its default for the
 * human port, the Gameplay and Pokemon toggles are at their defaults, and the Single Player Enemy option is off.
 *
 * @param mode MNRemixPanelMenuMode of the screen.
 * @param values Value of each item (MNREMIX_PANELMENU_ITEM_COUNT entries) for the human port.
 * @param globals Toggle words and enemy port.
 */
extern int mnRemixPanelMenuRulesHighScoresEnabled(int mode, const int *values, const MNRemixPanelMenuHighScoreGlobals *globals);

/* What the asm onchange_handler of an item does to the fighter shown on the CSS (css/ModelDisplay.asm:13, css/KirbyHat.asm:76,
 * css/Visibility.asm:37). Model and Kirby Hat reload the model of the fighter, Visibility sets the env colour override of the port. */
typedef enum MNRemixPanelMenuChangeAction
{
    MNREMIX_PANELMENU_CHANGE_NONE,
    MNREMIX_PANELMENU_CHANGE_MODEL,
    MNREMIX_PANELMENU_CHANGE_KIRBY_HAT,
    MNREMIX_PANELMENU_CHANGE_VISIBILITY
} MNRemixPanelMenuChangeAction;

#define MNREMIX_PANELMENU_DETAIL_HIGH 1
#define MNREMIX_PANELMENU_DETAIL_LOW 2

/** MNRemixPanelMenuChangeAction of an item; MNREMIX_PANELMENU_CHANGE_NONE for an item with no CSS fighter change handler. */
extern int mnRemixPanelMenuRulesChangeAction(int item);

/**
 * Poly detail the Model handler asks for (ModelDisplay.asm onchange_handler): Default asks for high poly, 1 and 2 are high and low.
 *
 * @param panel_value Model panel value, 0 (Default) to 2 (Lo Poly).
 */
extern int mnRemixPanelMenuRulesModelDetail(int panel_value);

/**
 * Remix fighter id whose hat Kirby wears on the CSS for a Kirby Hat panel value (KirbyHat.asm onchange_handler reads
 * KirbyHats.spawn_with_table_): None and Random give plain Kirby on the CSS, '???' gives the magic hat.
 *
 * @param panel_value Kirby Hat panel value, 0 (None) to 42 (Random).
 */
extern int mnRemixPanelMenuRulesCssHatFighter(int panel_value);

#endif

#endif
