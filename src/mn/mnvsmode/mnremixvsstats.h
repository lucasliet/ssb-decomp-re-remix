#ifndef _MNREMIXVSSTATS_H_
#define _MNREMIXVSSTATS_H_

#include <ssb_types.h>

#define MN_REMIX_VS_STATS_PORT_COUNT 4

typedef enum MNRemixVSStatsSpecial
{
    nMNRemixVSStatsSpecialUp,
    nMNRemixVSStatsSpecialNeutral,
    nMNRemixVSStatsSpecialDown,
    nMNRemixVSStatsSpecialCount
} MNRemixVSStatsSpecial;

/**
 * Clears the special move counters at the start of a VS match (VsStats.asm setup_, scVSBattleStartBattle).
 */
void mnRemixVSStatsReset(void);

/**
 * Counts one accepted special move of the given kind for a controller port (VsStats.asm increment_special_tracker).
 *
 * @param port Controller port of the fighter, 0 to MN_REMIX_VS_STATS_PORT_COUNT - 1.
 * @param special Up (USP), neutral (NSP) or down (DSP) special.
 */
void mnRemixVSStatsCountSpecial(s32 port, MNRemixVSStatsSpecial special);

/**
 * @return How many times the port used the given special move in the current match.
 */
s32 mnRemixVSStatsGetSpecialCount(s32 port, MNRemixVSStatsSpecial special);

/**
 * Share of the port's special moves that were of the given kind, as a whole percentage (same formula as VsStats.asm
 * calculate_tracker_percentage_: part * 100 / total).
 *
 * @return 0 to 100, or 0 when the port used no special move.
 */
s32 mnRemixVSStatsGetSpecialPercent(s32 port, MNRemixVSStatsSpecial special);

typedef enum MNRemixVSStatsTracker
{
    nMNRemixVSStatsTrackerZCancelSuccess,
    nMNRemixVSStatsTrackerZCancelMiss,
    nMNRemixVSStatsTrackerTechSuccess,
    nMNRemixVSStatsTrackerTechMiss,
    nMNRemixVSStatsTrackerLedgeGrab,
    nMNRemixVSStatsTrackerCount
} MNRemixVSStatsTracker;

/**
 * Adds one event to a tracker of the port (VsStats.asm stat_trackers: Z-cancel, tech, ledge grab).
 *
 * @param port Controller port of the fighter, 0 to MN_REMIX_VS_STATS_PORT_COUNT - 1.
 * @param tracker Event kind.
 */
void mnRemixVSStatsCountTracker(s32 port, MNRemixVSStatsTracker tracker);

/**
 * @return How many events the tracker recorded for the port in the current match.
 */
s32 mnRemixVSStatsGetTracker(s32 port, MNRemixVSStatsTracker tracker);

/**
 * Success share of a success and miss tracker pair, as a whole percentage (VsStats.asm calculate_tracker_percentage_).
 *
 * @param port Controller port, 0 to MN_REMIX_VS_STATS_PORT_COUNT - 1.
 * @param success Success tracker of the pair.
 * @param miss Miss tracker of the pair.
 * @return 0 to 100, or 0 when neither tracker recorded an event.
 */
s32 mnRemixVSStatsGetTrackerPercent(s32 port, MNRemixVSStatsTracker success, MNRemixVSStatsTracker miss);

/**
 * Keeps the highest damage percent the port reached in the current match (VsStats.asm collect_stats_midmatch).
 *
 * @param port Controller port, 0 to MN_REMIX_VS_STATS_PORT_COUNT - 1.
 * @param percent Current damage percent of the port.
 */
void mnRemixVSStatsRecordPercent(s32 port, s32 percent);

/**
 * @return Highest damage percent the port reached in the current match, 0 when it took no damage.
 */
s32 mnRemixVSStatsGetHighestPercent(s32 port);

#endif
