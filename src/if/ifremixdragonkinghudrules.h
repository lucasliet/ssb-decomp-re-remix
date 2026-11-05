#ifndef _IFREMIXDRAGONKINGHUDRULES_H_
#define _IFREMIXDRAGONKINGHUDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Dragon King HUD rules (DragonKingHUD.asm and InputDisplay.asm adjust_timer_position_), pure so the headless unit tests
 * (tests/remix) can run them. The toggle values are Toggles.entry_dragon_king_hud: 0 on the Dragon King stages only, 1 on
 * every stage, 2 never.
 */

#define IFREMIX_DKHUD_TOGGLE_STAGES 0
#define IFREMIX_DKHUD_TOGGLE_ALWAYS 1
#define IFREMIX_DKHUD_TOGGLE_OFF 2

/* Y of the damage numbers (DRAGON_KING_Y_POS 0x26); the stock icons follow it. */
#define IFREMIX_DKHUD_DAMAGE_Y 38

/* DragonKingHUD.asm dragon_king_numbers: 0 to 9, %, H.P. in file 0x00A4 (the damage file of the HUD). */
#define IFREMIX_DKHUD_DIGIT_COUNT 12

/* Teams and ports with a Dragon King colour (table_team: red, blue, green, yellow; table_ffa: p1 red, p2 blue, p3 yellow, p4 green). */
#define IFREMIX_DKHUD_COLOR_COUNT 4

/** What decides the colour of the damage numbers of one port. */
typedef struct IFRemixDragonKingHudColorInput
{
    int port;
    int is_vs_teams_battle; /* VS match with teams on (Global.vs.teams) */
    int team;
    int is_cpu;

} IFRemixDragonKingHudColorInput;

/**
 * Tells whether the Dragon King HUD is on for the stage (every scope of DragonKingHUD.asm tests the toggle first, then the stage).
 *
 * @param toggle Toggles.entry_dragon_king_hud.
 * @param gkind Stage id of the match.
 * @return 1 when the Dragon King layout is used.
 */
extern int ifRemixDragonKingHudRulesIsOn(int toggle, int gkind);

/**
 * Y of the player damage numbers (set_vertical_position_). The One Player game path of the asm sets the wrong register on the
 * Dragon King Remix stage, so only the toggle and the Dragon King stage lower the numbers there.
 *
 * @param toggle Toggles.entry_dragon_king_hud.
 * @param gkind Stage id of the match.
 * @param vanilla_y Y the scene sets without the Dragon King HUD.
 * @param is_one_player_game 1 for the One Player game path.
 * @return IFREMIX_DKHUD_DAMAGE_Y or vanilla_y.
 */
extern int ifRemixDragonKingHudRulesDamageY(int toggle, int gkind, int vanilla_y, int is_one_player_game);

/**
 * Sprite offset of a damage digit in file 0x00A4 (dragon_king_numbers).
 *
 * @param image_id 0 to 9 digits, 10 the % sign, 11 H.P.
 * @return The offset, 0 for an id out of range.
 */
extern unsigned int ifRemixDragonKingHudRulesDigitOffset(int image_id);

/**
 * Width of a damage digit used for the spacing (dragon_king_widths).
 *
 * @param image_id 0 to 9 digits, 10 the % sign, 11 H.P.
 * @return The width, 0 for an id out of range.
 */
extern int ifRemixDragonKingHudRulesDigitWidth(int image_id);

/**
 * Colour of the damage numbers (set_damage_color_): the team colour in a VS match with teams, white for a CPU, else the colour
 * of the port.
 *
 * @param input Port, team and kind of fighter.
 * @param rgb Receives red, green and blue.
 */
extern void ifRemixDragonKingHudRulesColor(const IFRemixDragonKingHudColorInput *input, unsigned char *rgb);

/**
 * Y of the match timer digits: the Dragon King HUD lowers them to 212 (adjust_timer_position_ of DragonKingHUD.asm), else
 * port 4's Input Display at the top of the screen lowers them to 52 (InputDisplay.asm), else 30. The Dragon King patch runs after
 * the Input Display one, so it wins.
 *
 * @param is_dragon_king_hud_on Result of ifRemixDragonKingHudRulesIsOn.
 * @param input_display_state_port4 Input Display panel value of port 4: 0 disabled, 1 top, 2 bottom.
 * @return The Y of the timer.
 */
extern float ifRemixDragonKingHudRulesTimerY(int is_dragon_king_hud_on, int input_display_state_port4);

#endif
#endif
