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

#endif
