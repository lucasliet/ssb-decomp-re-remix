/*
 * ftremixstalemoves.c - Move Staling of Smash Remix (.toolchain/pkgs/smashremix-src/src/StaleMoves.asm).
 * ftParamGetStale calls ftRemixStaleMovesGetMultiplier for the custom modes (asm check_toggle, 0x800EA4DC) and
 * ftRemixStaleMovesTable for the vanilla loop, which uses another table in the strict and wait-for-it modes
 * (asm 0x800EA4F8 and 0x800EA504).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <gm/gmdef.h>
#include <gm/gmremixtoggle.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include "ft/ftremix/ftremixhooks.h"

/* asm: Toggles.asm string_table_move_staling order, the value stored by entry_move_staling */
#define FTREMIX_STALE_DISABLED    1
#define FTREMIX_STALE_LENIENT     2
#define FTREMIX_STALE_STRICT      3
#define FTREMIX_STALE_WAIT_FOR_IT 4
#define FTREMIX_STALE_REVERSE     5
#define FTREMIX_STALE_CHEAP_SHOT  6

/* asm: StaleMoves.asm strict_stale_table, the 0x3C000000 word is 2^-7 (0.0078125), labelled 0.01F in the asm */
static const f32 sFTRemixStaleStrictTable[] = { 0.0078125F, 0.82F, 0.89F, 0.96F };
/* asm: StaleMoves.asm waitforit_stale_table */
static const f32 sFTRemixStaleWaitForItTable[] = { 0.75F, 0.82F, 0.89F, 1.75F };
/* asm: StaleMoves.asm reverse_stale_table */
static const f32 sFTRemixStaleReverseTable[] = { 0.82F, 0.89F, 0.96F, 1.00F };
/* asm: StaleMoves.asm cheap_shot_stale_table */
static const f32 sFTRemixStaleCheapShotTable[] = { 1.25F, 1.36F, 1.50F, 1.75F };

/* asm: StaleMoves.asm preserved_stale_value, the run count kept per port for multi-hit motions (-1 = never set) */
static s32 sFTRemixStalePreserved[GMCOMMON_PLAYERS_MAX] = { -1, -1, -1, -1 };

/* asm: StaleMoves.asm check_toggle, DEFAULT, STRICT and WAIT FOR IT keep the vanilla loop */
sb32 ftRemixStaleMovesIsCustom(void)
{
    s32 mode = gmRemixToggle(move_staling);

    return (mode == FTREMIX_STALE_DISABLED) || (mode == FTREMIX_STALE_LENIENT) || (mode == FTREMIX_STALE_REVERSE) || (mode == FTREMIX_STALE_CHEAP_SHOT);
}

/* asm: StaleMoves.asm _original_strict and _original_wfi, the vanilla loop reads the table of the mode */
const f32 *ftRemixStaleMovesTable(const f32 *vanilla_table)
{
    switch (gmRemixToggle(move_staling))
    {
        case FTREMIX_STALE_STRICT:
            return sFTRemixStaleStrictTable;
        case FTREMIX_STALE_WAIT_FOR_IT:
            return sFTRemixStaleWaitForItTable;
        default:
            return vanilla_table;
    }
}

/* asm: StaleMoves.asm wrap_index, the queue has five slots and the walk goes backwards */
static s32 ftRemixStaleMovesPrevIndex(s32 index)
{
    if (index == 0)
    {
        return ARRAY_COUNT(gSCManagerBattleState->players[0].stale_info) - 1;
    }
    return index - 1;
}

/* asm: StaleMoves.asm loop_stale_queue, counts the newest queue entries in a row that hold attack_id. The newest entry
 * with the same motion_count returns the count kept for the previous hit of that multi-hit motion. */
static s32 ftRemixStaleMovesCountRun(s32 player, s32 attack_id, u16 motion_count)
{
    s32 stale_id = gSCManagerBattleState->players[player].stale_id;
    s32 index = (stale_id != 0) ? stale_id - 1 : ARRAY_COUNT(gSCManagerBattleState->players[player].stale_info) - 1;
    s32 count = 0;

    while (count < ARRAY_COUNT(sFTRemixStaleReverseTable))
    {
        struct SCPlayerStale *entry = &gSCManagerBattleState->players[player].stale_info[index];

        if (attack_id != entry->attack_id)
        {
            return count;
        }
        if ((count == 0) && (motion_count == entry->motion_count))
        {
            return sFTRemixStalePreserved[player];
        }
        count++;
        index = ftRemixStaleMovesPrevIndex(index);
    }
    return count;
}

/* asm: StaleMoves.asm fresh_multiplier, REVERSE starts stale at 0.75x */
static f32 ftRemixStaleMovesFreshMultiplier(s32 mode)
{
    if (mode == FTREMIX_STALE_REVERSE)
    {
        return 0.75F;
    }
    return 1.0F;
}

/* asm: StaleMoves.asm check_toggle to _end, returns the stale multiplier of attack_id for the custom modes and stores the
 * run count of the player. A DISABLED run is always 0, so the multiplier stays 1.00x. */
f32 ftRemixStaleMovesGetMultiplier(s32 player, s32 attack_id, u16 motion_count)
{
    s32 mode = gmRemixToggle(move_staling);
    s32 count = 0;
    s32 index;

    if (mode != FTREMIX_STALE_DISABLED)
    {
        count = ftRemixStaleMovesCountRun(player, attack_id, motion_count);
    }
    sFTRemixStalePreserved[player] = count;

    index = count - 1;
    if (index < 0)
    {
        return ftRemixStaleMovesFreshMultiplier(mode);
    }
    switch (mode)
    {
        case FTREMIX_STALE_LENIENT:
            return 0.75F;
        case FTREMIX_STALE_REVERSE:
            return sFTRemixStaleReverseTable[index];
        case FTREMIX_STALE_CHEAP_SHOT:
            return sFTRemixStaleCheapShotTable[index];
        default:
            return 1.0F;
    }
}

#endif
