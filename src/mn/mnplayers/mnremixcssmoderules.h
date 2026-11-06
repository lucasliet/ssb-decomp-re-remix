#ifndef _MNREMIXCSSMODERULES_H_
#define _MNREMIXCSSMODERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix VS modes on the VS character select, so the headless unit tests (tests/remix) can run them: the
 * header image and the announcer of each mode (TwelveCharBattle.asm update_css_header_ and update_announcer_on_entry_) and
 * the two team rule of Smashketball (Smashketball.asm prevent_start_unless_two_teams_).
 *
 * Modes are numbered like MNREMIX_VSMODE_* (0 default, 1 12-Character Battle, 2 Tag Team, 3 King of the Hill,
 * 4 Smashketball, 5 Tug of War).
 */

#define MNREMIX_CSS_MODE_COUNT 6
#define MNREMIX_CSS_MODE_TEAM_SLOTS 4

/**
 * Offset of the header image of a mode in the game modes file (update_css_header_). Smashketball has two images, one per type.
 *
 * @param mode Mode, 0..5.
 * @param is_soccer Smashketball type: nonzero for soccer ("Smashketball 2").
 * @return Offset of the image, or 0 when the mode keeps the vanilla Free-for-all or Team Battle header.
 */
extern int mnRemixCssModeRulesTitleOffset(int mode, int is_soccer);

/**
 * FGM id the announcer plays when the character select opens (update_announcer_on_entry_).
 *
 * @param mode Mode, 0..5.
 * @return FGM id, or 0 when the mode keeps the vanilla Free-for-all or Team Battle voice.
 */
extern int mnRemixCssModeRulesAnnouncerFgm(int mode);

/** Frame of the character select the finished 12-Character Battle says "Wins" on (update_announcer_on_entry_._wins). */
#define MNREMIX_CSS_MODE_WINS_FRAME 0x34

/**
 * Announcer voice of the winner of a finished 12-Character Battle (update_announcer_on_entry_): "Player 1" unless the pool of player 1
 * is empty, then "Player 2".
 *
 * @param player_one_stocks_remaining Stocks left in the pool of player 1.
 * @return FGM id.
 */
extern int mnRemixCssModeRulesTwelveCBWinnerFgm(int player_one_stocks_remaining);

/**
 * "Wins" voice of a finished 12-Character Battle, which follows the winner's name at a fixed frame of the character select.
 *
 * @param is_complete Nonzero when the battle is finished.
 * @param frame Frames the character select has run.
 * @return FGM id on that frame, otherwise 0.
 */
extern int mnRemixCssModeRulesTwelveCBWinsFgm(int is_complete, unsigned int frame);

/**
 * Smashketball start rule: exactly two teams must be present among the panels that are in play with a fighter on them.
 *
 * @param teams Team of each panel, 0..3 (an out of range team never counts).
 * @param is_present Nonzero when the panel is in play and has a fighter picked or hovered.
 * @param count Number of panels.
 * @return Nonzero when exactly two different teams are present.
 */
extern int mnRemixCssModeRulesHasTwoTeams(const int *teams, const int *is_present, int count);

#endif

#endif
