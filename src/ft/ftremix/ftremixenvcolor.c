/**
 * ftremixenvcolor.c - Smash Remix CharEnvColor tables (src/CharEnvColor.asm override_table, moveset_table, state_table).
 *
 * The fighter draw (ftdisplaymain.c) asks ftRemixEnvColorGetOverride before it picks the stage light colour.
 */

#ifdef REGION_REMIX

#include "ftremixenvcolor.h"

#define FTREMIX_ENVCOLOR_VALUE_NONE 0xFFFFFF01   /* asm: -0x00FF */
#define FTREMIX_ENVCOLOR_VALUE_DARK 0x000000FF   /* asm: 0x00FF */
#define FTREMIX_ENVCOLOR_VALUE_CLOAKED 0xFFFFFF10 /* asm: -0x00F0 */

static u32 sFTRemixEnvColorOverride[FTREMIX_ENVCOLOR_PORTS];
static u32 sFTRemixEnvColorMoveset[FTREMIX_ENVCOLOR_PORTS];
static FTRemixEnvColorState sFTRemixEnvColorState[FTREMIX_ENVCOLOR_PORTS];

static sb32 ftRemixEnvColorIsPort(s32 port)
{
    return (port >= 0) && (port < FTREMIX_ENVCOLOR_PORTS);
}

u32 ftRemixEnvColorGetOverride(s32 port)
{
    if (!ftRemixEnvColorIsPort(port))
    {
        return 0;
    }
    return (sFTRemixEnvColorOverride[port] != 0) ? sFTRemixEnvColorOverride[port] : sFTRemixEnvColorMoveset[port];
}

u32 ftRemixEnvColorGetOverrideValue(s32 port)
{
    return ftRemixEnvColorIsPort(port) ? sFTRemixEnvColorOverride[port] : 0;
}

void ftRemixEnvColorSetOverride(s32 port, u32 rgba)
{
    if (ftRemixEnvColorIsPort(port))
    {
        sFTRemixEnvColorOverride[port] = rgba;
    }
}

void ftRemixEnvColorSetMoveset(s32 port, u32 rgba)
{
    if (ftRemixEnvColorIsPort(port))
    {
        sFTRemixEnvColorMoveset[port] = rgba;
    }
}

void ftRemixEnvColorSetState(s32 port, FTRemixEnvColorState state)
{
    if (ftRemixEnvColorIsPort(port))
    {
        sFTRemixEnvColorState[port] = state;
    }
}

FTRemixEnvColorState ftRemixEnvColorGetState(s32 port)
{
    return ftRemixEnvColorIsPort(port) ? sFTRemixEnvColorState[port] : nFTRemixEnvColorStateNormal;
}

u32 ftRemixEnvColorGetStateValue(s32 port)
{
    switch (ftRemixEnvColorGetState(port))
    {
    case nFTRemixEnvColorStateNone:
        return FTREMIX_ENVCOLOR_VALUE_NONE;

    case nFTRemixEnvColorStateDark:
        return FTREMIX_ENVCOLOR_VALUE_DARK;

    case nFTRemixEnvColorStateCloaked:
        return FTREMIX_ENVCOLOR_VALUE_CLOAKED;

    default:
        return 0;
    }
}

void ftRemixEnvColorRestoreState(s32 port)
{
    ftRemixEnvColorSetOverride(port, ftRemixEnvColorGetStateValue(port));
}

void ftRemixEnvColorClearAll(void)
{
    s32 port;

    for (port = 0; port < FTREMIX_ENVCOLOR_PORTS; port++)
    {
        sFTRemixEnvColorOverride[port] = 0;
        sFTRemixEnvColorMoveset[port] = 0;
    }
}

#endif
