#ifndef _GMREMIXTUGOFWAR_H_
#define _GMREMIXTUGOFWAR_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fttypes.h>

/*
 * Smash Remix Tug of War (TugOfWar.asm): a Timed Stock match where each KO also gives the KOer one more stock. The last
 * attacker of each fighter is kept per stock, so a KO without a KOer (self destruct, environment) still credits someone.
 * Ports are the controller ports (0..3); a port value of GMREMIX_TUGOFWAR_NO_PORT means nobody.
 */

#define GMREMIX_TUGOFWAR_NO_PORT (-1)

/**
 * TugOfWar.asm setup_: resets the last hits and the teams of the ports, clears the shield player of each fighter and
 * finds the 1v1 state (two fighters: each one's last hit is the other). Does nothing when the mode is not Tug of War.
 * Called at the start of each VS battle.
 */
extern void gmRemixTugOfWarSetup(void);

/**
 * TugOfWar.asm update_last_hit_by_._damage: `attacker` hit `victim`, so the attacker becomes the last hit of the victim
 * when the match is Tug of War, not 1v1 and the attacker is on another team.
 */
extern void gmRemixTugOfWarRecordDamage(FTStruct *victim, FTStruct *attacker);

/**
 * TugOfWar.asm update_last_hit_by_._shield: the shield player of `victim` broke its shield. Recorded as the last hit of the
 * victim when the match is Tug of War, not 1v1, the shield player is a valid port and it is on another team.
 */
extern void gmRemixTugOfWarRecordShieldBreak(FTStruct *victim);

/**
 * Last port that hit `victim_port` in this stock, or GMREMIX_TUGOFWAR_NO_PORT. The index is the port of the victim, never
 * its team (TugOfWar.asm credit_stock_to_koer_ R2).
 */
extern s32 gmRemixTugOfWarGetLastHit(s32 victim_port);

/**
 * Pure choice of the port that gets the stock of a KO. The KOer gets it when it is on another team than the victim;
 * otherwise (same team, or no KOer) the last hit of the victim gets it.
 *
 * victim_team: team of the fighter that was KO'd.
 * koer_team, koer_port: team and port of the KOer, koer_port GMREMIX_TUGOFWAR_NO_PORT when there is no KOer.
 * last_hit_port: last hit of the victim, from gmRemixTugOfWarGetLastHit.
 *
 * Returns the port to credit, or GMREMIX_TUGOFWAR_NO_PORT when nobody can be credited.
 */
extern s32 gmRemixTugOfWarPickCreditPort(s32 victim_team, s32 koer_team, s32 koer_port, s32 last_hit_port);

/**
 * TRUE when a stock credited to a fighter brings it back from elimination. `stock_count_before` is the FTStruct stock
 * count before the credit, which runs out at -1 (fttypes.h), so an eliminated fighter goes from -1 to 0.
 */
extern sb32 gmRemixTugOfWarShouldRevive(s32 stock_count_before);

#endif

#endif
