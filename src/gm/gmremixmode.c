/**
 * gmremixmode.c - the mode of the current Remix VS match: the menu choice or the SSB64_REMIX_MODE override.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixmode.h"
#include "if/ifremixsprite.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_debug.h"
#include "remix/remix_hud_text.h"

extern int strcmp(const char *a, const char *b);

typedef struct GMRemixModeName
{
    const char *name;
    s32 mode;
} GMRemixModeName;

static const GMRemixModeName dGMRemixModeNames[] =
{
    { "default", MNREMIX_VSMODE_DEFAULT },
    { "12cb", MNREMIX_VSMODE_TWELVE_CB },
    { "tagteam", MNREMIX_VSMODE_TAG_TEAM },
    { "koth", MNREMIX_VSMODE_KOTH },
    { "smashketball", MNREMIX_VSMODE_SMASHKETBALL },
    { "tugofwar", MNREMIX_VSMODE_TUG_OF_WAR }
};

static sb32 sGMRemixModeEnvIsRead = FALSE;
static s32 sGMRemixModeEnvMode = -1;

static s32 gmRemixModeReadEnv(void)
{
    const char *value;
    s32 i;

    if (sGMRemixModeEnvIsRead != FALSE)
    {
        return sGMRemixModeEnvMode;
    }
    sGMRemixModeEnvIsRead = TRUE;
    value = remix_debug_env_text("SSB64_REMIX_MODE");

    if (value == NULL)
    {
        return sGMRemixModeEnvMode;
    }
    for (i = 0; i < ARRAY_COUNT(dGMRemixModeNames); i++)
    {
        if (strcmp(value, dGMRemixModeNames[i].name) == 0)
        {
            sGMRemixModeEnvMode = dGMRemixModeNames[i].mode;
            return sGMRemixModeEnvMode;
        }
    }
    port_log("SSB64_REMIX_MODE=%s is not a mode (default, 12cb, tagteam, koth, smashketball, tugofwar)\n", value);
    return sGMRemixModeEnvMode;
}

s32 gmRemixModeGet(void)
{
    s32 env_mode = gmRemixModeReadEnv();

    return (env_mode >= 0) ? env_mode : gMNVSModeRemixMode;
}

sb32 gmRemixModeIs(s32 mode)
{
    return (gmRemixModeGet() == mode) ? TRUE : FALSE;
}

void gmRemixModeSceneClear(void)
{
    remix_hud_text_clear();
    ifRemixSpriteSceneReset();
}

void gmRemixModeLogMatch(void)
{
    port_log("remix mode: mode=%d rules=%d time_limit=%d stocks=%d team=%d\n", (int)gmRemixModeGet(),
             (int)gSCManagerBattleState->game_rules, (int)gSCManagerBattleState->time_limit,
             (int)gSCManagerBattleState->stocks, (int)gSCManagerBattleState->is_team_battle);
}

#endif
