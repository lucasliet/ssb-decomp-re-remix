#ifndef _MNREMIXSTAGEPANELRULES_H_
#define _MNREMIXSTAGEPANELRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the hazard, movement and layout lines of the Remix stage select (Stages.asm update_text_, right_check_
 * _hazard_toggle, update_cursor_color_), so the headless unit tests (tests/remix) can run them. */

/* Hazards.type: what the stage offers to switch off. */
#define MNREMIX_HAZARD_TYPE_NONE 0
#define MNREMIX_HAZARD_TYPE_HAZARDS 1
#define MNREMIX_HAZARD_TYPE_MOVEMENT 2
#define MNREMIX_HAZARD_TYPE_BOTH 3

/* Hazards.mode: bit 0 switches the hazards off, bit 1 switches the movement off. */
#define MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON 0
#define MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON 1
#define MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF 2
#define MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF 3

/* Toggles whispy_mode (J, Super, Hyper) and saffron_poke_rate (Super, Hyper, Quick Attack) values that change the "On" text. */
#define MNREMIX_WHISPY_MODE_JAPANESE 1
#define MNREMIX_WHISPY_MODE_SUPER 2
#define MNREMIX_WHISPY_MODE_HYPER 3
#define MNREMIX_SAFFRON_RATE_SUPER 1
#define MNREMIX_SAFFRON_RATE_HYPER 2
#define MNREMIX_SAFFRON_RATE_QUICK_ATTACK 3

typedef struct MNRemixStagePanelInput
{
    int stage_id;
    int is_random;
    int hazard_type;
    int hazard_mode;
    int whispy_mode;
    int saffron_poke_rate;
    int is_tournament;

} MNRemixStagePanelInput;

typedef struct MNRemixStagePanelView
{
    int show_hazards;
    int show_movement;
    const char *hazards_value;
    const char *movement_value;
    int cursor_color_index;

} MNRemixStagePanelView;

/**
 * Works out the Hazards and Movement lines and the cursor colour of the hovered stage (update_text_). The lines show when the
 * stage has that type or the RANDOM square is hovered; RANDOM on the tournament layout always reads On/On. The hazards value
 * carries the (J), (S), (H) or (Q) mark of the Whispy Mode and Saffron Pokemon Rate toggles.
 *
 * @param input Hovered stage (variant already resolved, or RANDOM) and the toggle values.
 * @param view Receives what to draw.
 */
extern void mnRemixStagePanelRulesCompute(const MNRemixStagePanelInput *input, MNRemixStagePanelView *view);

/**
 * Hazard mode after the L press (right_check_ _hazard_toggle): RANDOM and BOTH stages step through the four modes, a HAZARDS
 * stage flips bit 0 and a MOVEMENT stage flips bit 1.
 *
 * @param mode Current Hazard Mode toggle.
 * @param hazard_type Hazards.type of the hovered stage.
 * @param is_random TRUE when the RANDOM square is hovered.
 * @param next_mode Receives the new mode when the press changes it.
 * @return 1 when the press changes the mode, 0 when the stage has nothing to switch (hazard type NONE).
 */
extern int mnRemixStagePanelRulesNextHazardMode(int mode, int hazard_type, int is_random, int *next_mode);

/**
 * TRUE when the Layout line shows: a stage with variants is hovered on the normal layout (the line is replaced by the Roulette
 * line over RANDOM and hidden on the tournament layout).
 */
extern int mnRemixStagePanelRulesShowLayout(int is_random, int has_variants, int is_tournament);

/**
 * Text of the Layout line for a variant type (Stages.asm layout_text_table): Def., DL, Omega, then Remix for the three Remix
 * types. The asm table stops at type 5; no stage of Remix 2.0.1 uses the types 6 to 8, which read past that table, so they
 * read Remix here.
 *
 * @param variant_type Stages.variant_type value, 0 for the default stage.
 * @return Text in the Remix font charset (the Omega glyph is 0x7F).
 */
extern const char *mnRemixStagePanelRulesLayoutLabel(int variant_type);

#endif

#endif
