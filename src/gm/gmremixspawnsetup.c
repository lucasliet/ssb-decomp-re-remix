/**
 * gmremixspawnsetup.c - Smash Remix VS setup: neutral spawn positions (Spawn.asm, load_spawn_ hook at
 * 0x800FAF64) and the default CPU level applied when the VS battle setup starts (AI.asm, 0x8013AF2C).
 * VS and Training are ported (bonus and 1P spawns stay vanilla); Smashketball always uses its own neutral spawns.
 * The neutral and original rows of every stage id are read from the Remix image.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <string.h>
#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixspawnrules.h"
#include "gm/gmremixspawnsetup.h"
#include "gm/gmremixtoggle.h"
#include "gr/grremix/grremixstage.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define GMREMIX_SPAWN_PLAYERS_NUM 4
#define GMREMIX_SPAWN_TYPE_NOT 2
#define GMREMIX_SPAWN_TYPE_ACTIVE 0
#define GMREMIX_DEFAULT_CPU_LEVEL 3

/* Spawn.asm valid_teams / valid_singles row: key is the team of each player (teams) or its
 * type masked with 0x02 (singles, 2 = not in the match); slot is the neutral spawn of each player, -1 = none. */
typedef struct GMRemixSpawnSetup
{
    u8 key[GMREMIX_SPAWN_PLAYERS_NUM];
    s8 slot[GMREMIX_SPAWN_PLAYERS_NUM];
} GMRemixSpawnSetup;

#define GMREMIX_SPAWN_COORD_SIZE 4

/**
 * Reads a coordinate of a Spawn table (original_table or neutral_table) from the Remix image.
 * @param table Image address of the table.
 * @param offset Byte offset of the slot, from gmRemixSpawnRulesTableOffset.
 * @param coord 0 for x, 1 for y.
 * @return The coordinate.
 */
static f32 gmRemixSpawnReadTableCoord(u32 table, s32 offset, s32 coord)
{
    u32 word = remix_ram_image_read_word(table + offset + (coord * GMREMIX_SPAWN_COORD_SIZE));
    f32 value;

    memcpy(&value, &word, sizeof(value));
    return value;
}

/**
 * Overwrites x and y of a spawn position with a slot of a Spawn table; z is untouched, as in Spawn.asm _set_spawn.
 * @param table Image address of the table.
 * @param stage_id Stage id (row of the table).
 * @param slot Slot of the row, 0 to 3.
 * @param pos Spawn position.
 */
static void gmRemixSpawnApplyTableSlot(u32 table, s32 stage_id, s32 slot, Vec3f *pos)
{
    s32 offset = gmRemixSpawnRulesTableOffset(stage_id, slot);

    if (offset < 0)
    {
        return;
    }
    pos->x = gmRemixSpawnReadTableCoord(table, offset, 0);
    pos->y = gmRemixSpawnReadTableCoord(table, offset, 1);
}

/* Spawn.asm valid_teams: the two teams of every combination, with the neutral slot of each player. */
static const GMRemixSpawnSetup sGMRemixValidTeams[] =
{
    { { 0, 0, 1, 1 }, { 0, 2, 1, 3 } },
    { { 0, 1, 0, 1 }, { 0, 1, 2, 3 } },
    { { 0, 1, 1, 0 }, { 0, 1, 3, 2 } },
    { { 1, 1, 0, 0 }, { 0, 2, 1, 3 } },
    { { 1, 0, 1, 0 }, { 0, 1, 2, 3 } },
    { { 1, 0, 0, 1 }, { 0, 1, 3, 2 } },
    { { 0, 0, 2, 2 }, { 0, 2, 1, 3 } },
    { { 0, 2, 0, 2 }, { 0, 1, 2, 3 } },
    { { 0, 2, 2, 0 }, { 0, 1, 3, 2 } },
    { { 2, 2, 0, 0 }, { 0, 2, 1, 3 } },
    { { 2, 0, 2, 0 }, { 0, 1, 2, 3 } },
    { { 2, 0, 0, 2 }, { 0, 1, 3, 2 } },
    { { 1, 1, 2, 2 }, { 0, 2, 1, 3 } },
    { { 1, 2, 1, 2 }, { 0, 1, 2, 3 } },
    { { 1, 2, 2, 1 }, { 0, 1, 3, 2 } },
    { { 2, 2, 1, 1 }, { 0, 2, 1, 3 } },
    { { 2, 1, 2, 1 }, { 0, 1, 2, 3 } },
    { { 2, 1, 1, 2 }, { 0, 1, 3, 2 } },
    { { 0, 0, 3, 3 }, { 0, 2, 1, 3 } },
    { { 0, 3, 0, 3 }, { 0, 1, 2, 3 } },
    { { 0, 3, 3, 0 }, { 0, 1, 3, 2 } },
    { { 3, 3, 0, 0 }, { 0, 2, 1, 3 } },
    { { 3, 0, 3, 0 }, { 0, 1, 2, 3 } },
    { { 3, 0, 0, 3 }, { 0, 1, 3, 2 } },
    { { 1, 1, 3, 3 }, { 0, 2, 1, 3 } },
    { { 1, 3, 1, 3 }, { 0, 1, 2, 3 } },
    { { 1, 3, 3, 1 }, { 0, 1, 3, 2 } },
    { { 3, 3, 1, 1 }, { 0, 2, 1, 3 } },
    { { 3, 1, 3, 1 }, { 0, 1, 2, 3 } },
    { { 3, 1, 1, 3 }, { 0, 1, 3, 2 } },
    { { 2, 2, 3, 3 }, { 0, 2, 1, 3 } },
    { { 2, 3, 2, 3 }, { 0, 1, 2, 3 } },
    { { 2, 3, 3, 2 }, { 0, 1, 3, 2 } },
    { { 3, 3, 2, 2 }, { 0, 2, 1, 3 } },
    { { 3, 2, 3, 2 }, { 0, 1, 2, 3 } },
    { { 3, 2, 2, 3 }, { 0, 1, 3, 2 } },
};

/* Spawn.asm valid_singles: 1v1 combinations only. */
static const GMRemixSpawnSetup sGMRemixValidSingles[] =
{
    { { 0, 0, 2, 2 }, { 0, 1, -1, -1 } },
    { { 0, 2, 0, 2 }, { 0, -1, 1, -1 } },
    { { 0, 2, 2, 0 }, { 0, -1, -1, 1 } },
    { { 2, 2, 0, 0 }, { -1, -1, 0, 1 } },
    { { 2, 0, 2, 0 }, { -1, 0, -1, 1 } },
    { { 2, 0, 0, 2 }, { -1, 0, 1, -1 } },
};

/**
 * Checks whether a setup row matches the key of every player.
 * @param setup Setup row from the Spawn.asm tables.
 * @param keys Key of each player in the current match.
 * @return TRUE when every player key equals the row key.
 */
static sb32 gmRemixSpawnSetupMatches(const GMRemixSpawnSetup *setup, const u8 keys[GMREMIX_SPAWN_PLAYERS_NUM])
{
    s32 player;

    for (player = 0; player < GMREMIX_SPAWN_PLAYERS_NUM; player++)
    {
        if (setup->key[player] != keys[player])
        {
            return FALSE;
        }
    }
    return TRUE;
}

/**
 * Finds the setup row of the current match and returns the neutral slot of a player.
 * @param setups Setup table to search (teams or singles).
 * @param count Number of rows in setups.
 * @param keys Key of each player in the current match.
 * @param player Player index.
 * @return Neutral slot of the player, or -1 when no row matches or the player is not in the match.
 */
static s32 gmRemixSpawnSlotFromSetups(const GMRemixSpawnSetup *setups, s32 count, const u8 keys[GMREMIX_SPAWN_PLAYERS_NUM], s32 player)
{
    s32 i;

    for (i = 0; i < count; i++)
    {
        if (gmRemixSpawnSetupMatches(&setups[i], keys))
        {
            return setups[i].slot[player];
        }
    }
    return -1;
}

/**
 * Builds the team key of each player for a team battle.
 * @param keys Output array, one team byte per player.
 */
static void gmRemixSpawnBuildTeamKeys(u8 keys[GMREMIX_SPAWN_PLAYERS_NUM])
{
    s32 player;

    for (player = 0; player < GMREMIX_SPAWN_PLAYERS_NUM; player++)
    {
        keys[player] = gSCManagerBattleState->players[player].team;
    }
}

/**
 * Builds the singles key of each player: 0 when the player is in the match, 2 when the slot is empty.
 * @param keys Output array, one masked type byte per player.
 */
static void gmRemixSpawnBuildSinglesKeys(u8 keys[GMREMIX_SPAWN_PLAYERS_NUM])
{
    s32 player;

    for (player = 0; player < GMREMIX_SPAWN_PLAYERS_NUM; player++)
    {
        if (gSCManagerBattleState->players[player].pkind == nFTPlayerKindNot)
        {
            keys[player] = GMREMIX_SPAWN_TYPE_NOT;
        }
        else
        {
            keys[player] = GMREMIX_SPAWN_TYPE_ACTIVE;
        }
    }
}

/**
 * Returns the neutral slot of a player for the current match, following the Spawn.asm order: teams first, then
 * singles. A match without a valid setup returns -1 and keeps the vanilla spawn.
 * @param player Player index.
 * @return Neutral slot of the player, or -1.
 */
static s32 gmRemixSpawnNeutralSlot(s32 player)
{
    u8 keys[GMREMIX_SPAWN_PLAYERS_NUM];

    if (gSCManagerBattleState->is_team_battle)
    {
        gmRemixSpawnBuildTeamKeys(keys);
        return gmRemixSpawnSlotFromSetups(sGMRemixValidTeams, ARRAY_COUNT(sGMRemixValidTeams), keys, player);
    }
    gmRemixSpawnBuildSinglesKeys(keys);
    return gmRemixSpawnSlotFromSetups(sGMRemixValidSingles, ARRAY_COUNT(sGMRemixValidSingles), keys, player);
}

/**
 * Overwrites the vanilla VS spawn of a player with a Remix spawn, as Spawn.asm load_spawn_ does: the neutral table when
 * the Neutral Spawns toggle is on and the match has a valid setup, the original table otherwise. Only x and y change.
 * Called after the vanilla map lookup.
 * @param player Player index.
 * @param pos Spawn position, already set to the vanilla spawn.
 */
void gmRemixNeutralSpawnApply(s32 player, Vec3f *pos)
{
    s32 neutral_slot;
    s32 uses_neutral;
    s32 stage_id;
    sb32 is_neutral_on = gmRemixToggle(neutral_spawns) ? TRUE : FALSE;

    if (gmRemixSmashketballIsActive() != FALSE)
    {
        f32 x, y;

        gmRemixSmashketballGetSpawnPos(gmRemixSmashketballGetSpawnSlot(player), &x, &y);
        pos->x = x;
        pos->y = y;
        return;
    }
    stage_id = gSCManagerBattleState->gkind;
    if ((stage_id < 0) || (stage_id > GRREMIX_KIND_MAX))
    {
        return;
    }
    neutral_slot = is_neutral_on ? gmRemixSpawnNeutralSlot(player) : -1;
    uses_neutral = gmRemixSpawnRulesVsUsesNeutral(is_neutral_on, neutral_slot);

    gmRemixSpawnApplyTableSlot(uses_neutral ? REMIX_SYM_Spawn__neutral_table : REMIX_SYM_Spawn__original_table, stage_id,
                               gmRemixSpawnRulesVsSlot(player, uses_neutral, neutral_slot), pos);
}

/**
 * Overwrites the vanilla Training spawn of a port with the slot of the original table, as Spawn.asm load_spawn_ does for
 * spawn ids 0 to 3 (Port 1 to 4). The custom spawn id (4) is applied by sc1PTrainingRemixSpawnApplyDesc.
 * @param spawn_id Spawn id of the port (0 to 3 for a table slot).
 * @param pos Spawn position, already set to the vanilla spawn.
 */
void gmRemixTrainingSpawnApply(s32 spawn_id, Vec3f *pos)
{
    s32 slot = gmRemixSpawnRulesTrainingSlot(spawn_id);
    s32 stage_id = gSCManagerBattleState->gkind;

    if (slot < 0)
    {
        return;
    }
    gmRemixSpawnApplyTableSlot(REMIX_SYM_Spawn__original_table, stage_id, slot, pos);
}

/**
 * Applies the Default CPU LVL (V.S.) toggle to the four player slots, once per boot, as AI.asm does on the first VS
 * setup. A toggle of 0 (default) uses level 3. The A-press refresh of the Remix menu is not ported.
 */
void gmRemixDefaultCPULevelSetup(void)
{
    static sb32 sIsDefaultCPULevelSet = FALSE;   /* AI.is_default_cpu_lvl_set */
    s32 level;
    s32 player;

    if (sIsDefaultCPULevelSet)
    {
        return;
    }
    sIsDefaultCPULevelSet = TRUE;

    level = gmRemixToggle(default_cpu_level);
    if (level == 0)
    {
        level = GMREMIX_DEFAULT_CPU_LEVEL;
    }
    for (player = 0; player < GMREMIX_SPAWN_PLAYERS_NUM; player++)
    {
        gSCManagerTransferBattleState.players[player].level = level;
    }
}

#endif
