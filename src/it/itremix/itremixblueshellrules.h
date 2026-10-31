#ifndef _ITREMIXBLUESHELLRULES_H_
#define _ITREMIXBLUESHELLRULES_H_

#include <stdint.h>

/* Pure rules of the Blue Shell target choice (BlueShell.asm get_player_in_first_). Ints only, so the headless tests can run them. */

#define ITREMIX_BLUESHELL_RULES_PLAYERS 4
#define ITREMIX_BLUESHELL_RULES_GAME_RULES_STAMINA 5

/**
 * TRUE when the match rules byte (SCBattleState.game_rules, asm match_info + 3) is Stamina. Stamina picks the lowest value
 * as first place, the other rules pick the highest.
 */
int32_t itRemixBlueShellRulesIsStamina(int32_t game_rules);

/**
 * Port with the best place in the match, as get_player_in_first_ chooses it. Port 0 is always the first candidate, even
 * without a fighter. A later port needs a fighter, and a value that is better (or equal, for the rules that pick the
 * highest). On a tie of values the lower damage percent wins.
 *
 * @param is_lower_wins TRUE for Stamina, where the lowest value is first.
 * @param values Score of each port: KO count (time), stock count (stock) or the Stamina value.
 * @param percents Damage percent of each port, used for ties.
 * @param is_present TRUE for each port that has a fighter.
 * @return Port index 0..3. 0 when no port has a fighter.
 */
int32_t itRemixBlueShellRulesFirstPort(int32_t is_lower_wins, const int32_t values[], const int32_t percents[], const int32_t is_present[]);

#endif
