#ifndef _GMREMIXTUGOFWARRULES_H_
#define _GMREMIXTUGOFWARRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure credit rules of Tug of War (TugOfWar.asm credit_stock_to_koer_), so the headless unit tests (tests/remix) can run them. */

#define GMREMIX_TUGOFWARRULES_NO_PORT (-1)
#define GMREMIX_TUGOFWARRULES_PORTS 4

/** TRUE for a controller port, 0 to 3. */
extern int gmRemixTugOfWarRulesIsValidPort(int port);

/**
 * Pure choice of the port that gets the stock of a KO. The KOer gets it when it is on another team than the victim;
 * otherwise (same team, or no KOer) the last hit of the victim gets it.
 *
 * victim_team: team of the fighter that was KO'd.
 * koer_team, koer_port: team and port of the KOer, koer_port GMREMIX_TUGOFWARRULES_NO_PORT when there is no KOer.
 * last_hit_port: last hit of the victim, from gmRemixTugOfWarGetLastHit.
 *
 * Returns the port to credit, or GMREMIX_TUGOFWARRULES_NO_PORT when nobody can be credited.
 */
extern int gmRemixTugOfWarPickCreditPort(int victim_team, int koer_team, int koer_port, int last_hit_port);

/**
 * TRUE when a stock credited to a fighter brings it back from elimination. `stock_count_before` is the FTStruct stock
 * count before the credit, which runs out at -1 (fttypes.h), so an eliminated fighter goes from -1 to 0.
 */
extern int gmRemixTugOfWarShouldRevive(int stock_count_before);

#endif

#endif
