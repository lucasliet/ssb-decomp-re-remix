/**
 * gmremixspawnsetup.c - Smash Remix VS setup: neutral spawn positions (Spawn.asm, load_spawn_ hook at
 * 0x800FAF64) and the default CPU level applied when the VS battle setup starts (AI.asm, 0x8013AF2C).
 * Only the VS path is ported. Training and bonus spawns stay vanilla; Smashketball always uses its own neutral spawns.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixspawnsetup.h"
#include "gm/gmremixtoggle.h"

#define GMREMIX_SPAWN_PLAYERS_NUM 4
#define GMREMIX_SPAWN_STAGES_NUM 0x11
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

/* Spawn.asm neutral_table, one row per VS stage id (the stage id is the gkind, 0x00 = Peach's Castle
 * to 0x10 = Final Destination). Each row holds the x, y of the four neutral slots. */
static const f32 sGMRemixNeutralSpawns[GMREMIX_SPAWN_STAGES_NUM][GMREMIX_SPAWN_PLAYERS_NUM][2] =
{
    { { -0x1.9340000000000p+10f, 0x1.8480000000000p+10f }, { 0x1.9340000000000p+10f, 0x1.6100000000000p+10f }, { -0x1.4c80000000000p+9f, 0x1.4b00000000000p+9f }, { 0x1.4c80000000000p+9f, 0x1.4b00000000000p+9f } }, /* 0x00 */
    { { -0x1.9ca0000000000p+11f, 0x1.d340000000000p+10f }, { 0x1.2800000000000p+8f, 0x1.b2c0000000000p+10f }, { -0x1.05c0000000000p+11f, 0x1.ab00000000000p+10f }, { -0x1.c100000000000p+9f, 0x1.8e40000000000p+10f } }, /* 0x01 */
    { { -0x1.b2c0000000000p+10f, 0x1.0000000000000p+1f }, { 0x1.b2c0000000000p+10f, 0x1.0000000000000p+1f }, { -0x1.3b00000000000p+9f, -0x1.a400000000000p+7f }, { 0x1.3b00000000000p+9f, -0x1.a400000000000p+7f } }, /* 0x02 */
    { { -0x1.3f80000000000p+11f, 0x1.1e00000000000p+9f }, { 0x1.b440000000000p+10f, -0x1.0600000000000p+8f }, { -0x1.1c40000000000p+10f, 0x1.6000000000000p+3f }, { 0x0.0p+0f, 0x1.3a00000000000p+8f } }, /* 0x03 */
    { { -0x1.2c00000000000p+11f, 0x1.0480000000000p+10f }, { 0x1.7700000000000p+10f, 0x1.0480000000000p+10f }, { -0x1.1580000000000p+10f, 0x1.0480000000000p+10f }, { 0x1.e000000000000p+7f, 0x1.0480000000000p+10f } }, /* 0x04 */
    { { 0x1.3a80000000000p+9f, -0x1.8000000000000p+6f }, { 0x1.6800000000000p+6f, 0x1.2d20000000000p+11f }, { 0x1.7a00000000000p+9f, 0x1.7a40000000000p+10f }, { -0x1.ef00000000000p+9f, 0x1.0200000000000p+10f } }, /* 0x05 */
    { { -0x1.5e00000000000p+10f, 0x1.c700000000000p+9f }, { 0x1.5e00000000000p+10f, 0x1.c700000000000p+9f }, { -0x1.5e00000000000p+10f, 0x1.4000000000000p+2f }, { 0x1.5e00000000000p+10f, 0x1.4000000000000p+2f } }, /* 0x06 */
    { { 0x1.2c00000000000p+10f, 0x1.9500000000000p+9f }, { -0x1.4a00000000000p+9f, -0x1.0e00000000000p+8f }, { 0x1.fe00000000000p+8f, 0x1.6800000000000p+6f }, { -0x1.2c00000000000p+10f, -0x1.0e00000000000p+8f } }, /* 0x07 */
    { { -0x1.c200000000000p+10f, 0x1.4980000000000p+10f }, { 0x1.7700000000000p+10f, 0x1.e100000000000p+9f }, { -0x1.7700000000000p+10f, 0x1.3000000000000p+7f }, { 0x1.c200000000000p+10f, 0x1.c3c0000000000p+10f } }, /* 0x08 */
    { { 0x1.dd00000000000p+9f, 0x1.2c00000000000p+7f }, { 0x1.f700000000000p+9f, 0x1.d100000000000p+9f }, { -0x1.be00000000000p+9f, 0x1.cf80000000000p+9f }, { -0x1.c800000000000p+9f, 0x1.2c00000000000p+7f } }, /* 0x09 */
    { { -0x1.c200000000000p+8f, 0x1.2c00000000000p+7f }, { 0x1.c200000000000p+8f, 0x1.2c00000000000p+7f }, { 0x0.0p+0f, 0x1.a940000000000p+10f }, { 0x1.7c80000000000p+10f, 0x1.d100000000000p+9f } }, /* 0x0A */
    { { 0x1.4a00000000000p+9f, 0x0.0p+0f }, { 0x1.6800000000000p+10f, 0x0.0p+0f }, { -0x1.4a00000000000p+9f, 0x0.0p+0f }, { -0x1.6800000000000p+10f, 0x0.0p+0f } }, /* 0x0B */
    { { -0x1.1440000000000p+10f, -0x1.4400000000000p+7f }, { 0x1.ed00000000000p+9f, -0x1.3800000000000p+7f }, { -0x1.3a40000000000p+10f, 0x1.6b00000000000p+9f }, { 0x1.2980000000000p+10f, 0x1.de00000000000p+9f } }, /* 0x0C */
    { { -0x1.19c0000000000p+10f, 0x1.f000000000000p+4f }, { 0x1.ba40000000000p+10f, 0x1.0700000000000p+8f }, { -0x1.af00000000000p+8f, 0x1.2000000000000p+5f }, { 0x1.8900000000000p+9f, 0x1.e000000000000p+4f } }, /* 0x0D */
    { { -0x1.2c00000000000p+10f, 0x1.0040000000000p+10f }, { 0x1.2c00000000000p+10f, 0x1.0040000000000p+10f }, { -0x1.2c00000000000p+10f, 0x1.4000000000000p+2f }, { 0x1.2c00000000000p+10f, 0x1.4000000000000p+2f } }, /* 0x0E */
    { { 0x0.0p+0f, 0x0.0p+0f }, { 0x0.0p+0f, 0x0.0p+0f }, { 0x0.0p+0f, 0x0.0p+0f }, { 0x0.0p+0f, 0x0.0p+0f } }, /* 0x0F */
    { { -0x1.c200000000000p+10f, 0x1.4000000000000p+2f }, { 0x1.c200000000000p+10f, 0x1.4000000000000p+2f }, { -0x1.c200000000000p+9f, 0x1.4000000000000p+2f }, { 0x1.c200000000000p+9f, 0x1.4000000000000p+2f } }, /* 0x10 */
};

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
 * Overwrites the vanilla VS spawn of a player with the Remix neutral spawn when the Neutral Spawns toggle is on and
 * the match has a valid setup. Only x and y change, as in Spawn.asm. Called after the vanilla map lookup.
 * @param player Player index.
 * @param pos Spawn position, already set to the vanilla spawn.
 */
void gmRemixNeutralSpawnApply(s32 player, Vec3f *pos)
{
    s32 slot;
    s32 stage_id;

    if (gmRemixSmashketballIsActive() != FALSE)
    {
        f32 x, y;

        gmRemixSmashketballGetSpawnPos(gmRemixSmashketballGetSpawnSlot(player), &x, &y);
        pos->x = x;
        pos->y = y;
        return;
    }
    if (!gmRemixToggle(neutral_spawns))
    {
        return;
    }
    stage_id = gSCManagerBattleState->gkind;
    if (stage_id >= GMREMIX_SPAWN_STAGES_NUM)
    {
        return;
    }
    slot = gmRemixSpawnNeutralSlot(player);
    if (slot < 0)
    {
        return;
    }
    pos->x = sGMRemixNeutralSpawns[stage_id][slot][0];
    pos->y = sGMRemixNeutralSpawns[stage_id][slot][1];
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
