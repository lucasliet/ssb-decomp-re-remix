#ifndef _GMREMIXSMASHKETBALLRULES_H_
#define _GMREMIXSMASHKETBALLRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure scoring rules of Smashketball (Smashketball.asm _scored), so the headless unit tests (tests/remix) can run them. */

/* Same value as GMREMIX_SMASHKETBALL_NO_TEAM in gmremixsmashketball.h. */
#define GMREMIX_SMASHKETBALL_RULES_NO_TEAM (-1)
#define GMREMIX_SMASHKETBALL_RULES_PLAYERS 4
#define GMREMIX_SMASHKETBALL_RULES_GOLDEN_POINTS 3
#define GMREMIX_SMASHKETBALL_RULES_ATTRIBUTED_SHARE 0

/**
 * Team credited by a hoop: a negative side is the left hoop, which credits the Away team; any other side credits Home.
 */
extern int gmRemixSmashketballRulesScoringTeam(int side, int home_team, int away_team);

/** Points of a basket: GMREMIX_SMASHKETBALL_RULES_GOLDEN_POINTS for the golden ball, 1 otherwise. */
extern int gmRemixSmashketballRulesBasketPoints(int is_golden);

/** TRUE when the ball has an owner and that owner is on the scoring team. */
extern int gmRemixSmashketballRulesIsOwnerScoring(int has_owner, int owner_team, int scoring_team);

/**
 * Winner score of a basket for one share: the table entry of `teammates` times `points`. Index 0 is an attributed
 * basket, index N the unattributed share of a team of N.
 */
extern int gmRemixSmashketballRulesWinnerShare(int teammates, int points);

/** Points of a team, 0 when the team is GMREMIX_SMASHKETBALL_RULES_NO_TEAM. */
extern int gmRemixSmashketballRulesTeamScore(int team, const int *team_scores);

/**
 * Records the team of one fighter: the first fighter sets home_team; a fighter of another team sets away_team, so the
 * last different team seen wins.
 */
extern void gmRemixSmashketballRulesRecordTeam(int *home_team, int *away_team, int team);

/** Attributed basket: the streak of `port` goes up and every other streak goes back to 0. */
extern void gmRemixSmashketballRulesRecordBasket(int *streaks, int port);

#endif

#endif
