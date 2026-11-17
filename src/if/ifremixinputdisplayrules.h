#ifndef _IFREMIXINPUTDISPLAYRULES_H_
#define _IFREMIXINPUTDISPLAYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <stdint.h>

/*
 * Pure rules of the Remix Input Display (InputDisplay.asm), so the headless unit tests (tests/remix) can run them: where the
 * block of a port goes (create_input_display_objects_) and how the stick and its gate follow the sample
 * (update_input_display_).
 */

/** Screen class of the match, from Global.current_screen: VS 0x16, Training 0x36, Bonus 1 and 2 0x35. */
typedef enum IFRemixInputDisplayScreen
{
    IFREMIX_INPUTDISPLAY_SCREEN_OTHER,
    IFREMIX_INPUTDISPLAY_SCREEN_VS,
    IFREMIX_INPUTDISPLAY_SCREEN_TRAINING,
    IFREMIX_INPUTDISPLAY_SCREEN_BONUS
} IFRemixInputDisplayScreen;

/** Single player mode class, from SinglePlayerModes.singleplayer_mode_flag. */
typedef enum IFRemixInputDisplayMode
{
    IFREMIX_INPUTDISPLAY_MODE_OTHER,
    IFREMIX_INPUTDISPLAY_MODE_MULTIMAN,
    IFREMIX_INPUTDISPLAY_MODE_ALLSTAR,
    IFREMIX_INPUTDISPLAY_MODE_SINGLE
} IFRemixInputDisplayMode;

/** Stages of the 1P game the asm tests (STAGE_FLAG): Yoshi team, Kirby team and Polygon team. */
#define IFREMIX_INPUTDISPLAY_STAGE_REST_AREA 0
#define IFREMIX_INPUTDISPLAY_STAGE_YOSHI_TEAM 1
#define IFREMIX_INPUTDISPLAY_STAGE_KIRBY_TEAM 8
#define IFREMIX_INPUTDISPLAY_STAGE_POLYGON_TEAM 0x0C

typedef struct IFRemixInputDisplayLayoutInput
{
    int is_bottom_requested;
    IFRemixInputDisplayScreen screen;
    IFRemixInputDisplayMode mode;
    int stage;
    int is_dragon_king_hud;
} IFRemixInputDisplayLayoutInput;

typedef struct IFRemixInputDisplayLayout
{
    int is_bottom;
    float offset_y;
} IFRemixInputDisplayLayout;

/** Stick range of the sample the display draws (0x50), and the pixels one unit of it moves the stick sprite. */
#define IFREMIX_INPUTDISPLAY_STICK_RANGE 80
#define IFREMIX_INPUTDISPLAY_STICK_PIXELS_PER_UNIT 0.06F

/** Frames the gate stays lit after the stick returns to the centre. */
#define IFREMIX_INPUTDISPLAY_GATE_COOLDOWN (-6)

typedef struct IFRemixInputDisplayStick
{
    float offset_x;
    float offset_y;
    uint8_t stick_alpha;
    uint8_t gate_alpha;
    int cooldown;
} IFRemixInputDisplayStick;

/**
 * Edge and vertical offset of the block of a port (create_input_display_objects_): Training always at the bottom, the 1P and
 * Multi-Man offsets that clear the stage HUD, the Dragon King HUD offsets that undo the Training bottom rule.
 *
 * @param input Panel value, screen, mode, stage and Dragon King HUD state.
 * @param layout Receives the edge and the offset added to every Y of the block.
 */
extern void ifRemixInputDisplayRulesResolveLayout(const IFRemixInputDisplayLayoutInput *input, IFRemixInputDisplayLayout *layout);

/**
 * Stick sprite offset and the alpha of the stick and its gate for one sample (update_input_display_).
 *
 * @param stick_x Raw stick X.
 * @param stick_y Raw stick Y.
 * @param cooldown Value kept from the previous frame (0 at the start).
 * @param stick Receives the offset from the centred position, the alphas and the new cooldown.
 */
extern void ifRemixInputDisplayRulesUpdateStick(int stick_x, int stick_y, int cooldown, IFRemixInputDisplayStick *stick);

#endif

#endif
