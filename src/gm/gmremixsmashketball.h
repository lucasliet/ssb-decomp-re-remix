#ifndef _GMREMIXSMASHKETBALL_H_
#define _GMREMIXSMASHKETBALL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fttypes.h>

/*
 * Smash Remix Smashketball (Smashketball.asm): two teams, Home and Away, play basketball or soccer with one ball on the
 * Smashketball or Soccer stage. The first team to reach the goal (the CSS score, default 10, 100 means no goal) wins,
 * a basket or goal is worth 1 point and 3 for the golden ball. The Rule button of the VS menu is not used: the type
 * (basketball or soccer) is chosen in the port menu (Remix > Smashketball) or with SSB64_SMASH_TYPE.
 */

#define GMREMIX_SMASHKETBALL_TYPE_BASKETBALL 0
#define GMREMIX_SMASHKETBALL_TYPE_SOCCER 1
#define GMREMIX_SMASHKETBALL_GOAL_NONE 100
#define GMREMIX_SMASHKETBALL_NO_TEAM (-1)
/* Smashketball.asm play_end_buzzer_: FGM queued after the announcer sound when the match ends. */
#define GMREMIX_SMASHKETBALL_FGM_END_BUZZER 0x431

/** TRUE during a Smashketball match. */
extern sb32 gmRemixSmashketballIsActive(void);

/** Smashketball.type: TRUE for soccer (the Soccer stage and goals, the ball ignores the hoop). */
extern sb32 gmRemixSmashketballIsSoccer(void);

/** Smashketball.spawn_golden: TRUE when the next ball is the golden soccer ball (worth 3 points). */
extern sb32 gmRemixSmashketballIsGolden(void);

/** Team of the home side (the left spawn side), read by the CPU ball hooks (Smashketball.asm ai_ball_behavior_). */
extern s32 gmRemixSmashketballGetHomeTeam(void);

/** Stage id the match plays on for the current type (Smashketball 0x5A, Soccer 0xD1). */
extern s32 gmRemixSmashketballGetStage(void);

/** Match start, after the stage exists: scores, teams, baskets height, scoreboard and the update process. */
extern void gmRemixSmashketballSetup(void);

/** Smashketball.winner_scores: score of `port` for the results ordering (TimedStock.asm _smashketball). */
extern s32 gmRemixSmashketballGetWinnerScore(s32 port);

/**
 * Smashketball.asm prevent_sudden_death_ and allow_overtime_: after the time ran out sudden death starts only when the two
 * team scores are tied, with every fighter in it and no time limit. Sets gSCManagerVSBattleState for it.
 *
 * @return TRUE when sudden death must start.
 */
extern sb32 gmRemixSmashketballPrepareSuddenDeath(void);

/** Smashketball.asm no_initial_damage_: fighters of the Smashketball sudden death start at 0% instead of 300%. */
extern s32 gmRemixSmashketballGetSuddenDeathDamage(s32 default_damage);

/** Smashketball.player_spawns: neutral spawn slot of `player` (0..3) following the Home and Away fill order, or -1. */
extern s32 gmRemixSmashketballGetSpawnSlot(s32 player);

/** Spawn position of neutral slot `slot` (x and y, the same on both stages). */
extern void gmRemixSmashketballGetSpawnPos(s32 slot, f32 *x, f32 *y);

/** Frame update of the Smashketball scoreboard model (scoreboard_main_). Called by the update process. */
extern void gmRemixSmashketballBoardUpdate(s32 home_score, s32 away_score);

/** Finds the digits of the scoreboard model in the stage (setup_ _init_scoreboard). */
extern void gmRemixSmashketballBoardSetup(void);

/** setup_ _teams: paints the score digits with the colour of the Home and the Away team (Color.tag of the team index). */
extern void gmRemixSmashketballBoardSetTeamColors(s32 home_team, s32 away_team);

/** setup_ time only mode: the four score digits show the dash, only the time runs (Smashketball stage in a normal match). */
extern void gmRemixSmashketballBoardShowDashScores(void);

/** scoreboard_main_ time only mode: updates the time digits and leaves the scores alone. */
extern void gmRemixSmashketballBoardUpdateTimeOnly(void);

/** Moves the baskets joint down to its Smashketball height and returns its y (Hazards.asm _nba_jam), or 0 when missing. */
extern f32 gmRemixSmashketballMoveBaskets(void);

/** Y of the baskets joint, valid after gmRemixSmashketballMoveBaskets. */
extern f32 gmRemixSmashketballGetBasketsY(void);

#endif

#endif
