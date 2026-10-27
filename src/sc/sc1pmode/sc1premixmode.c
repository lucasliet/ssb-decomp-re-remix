/**
 * sc1premixmode.c - the Remix 1P mode of the current run, see sc1premixmode.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <port_log.h>
#include "sc/sc1pmode/sc1premixmode.h"
#include "remix/remix_debug.h"

s32 gSC1PRemixMode = SC1PREMIX_MODE_STANDARD;

static sb32 sSC1PRemixModeEnvIsRead = FALSE;
static s32 sSC1PRemixModeEnvMode = -1;

static s32 sc1PRemixModeReadEnv(void)
{
    const char *value;

    if (sSC1PRemixModeEnvIsRead != FALSE)
    {
        return sSC1PRemixModeEnvMode;
    }
    sSC1PRemixModeEnvIsRead = TRUE;
    value = remix_debug_env_text("SSB64_REMIX_1P_MODE");

    if (value == NULL)
    {
        return sSC1PRemixModeEnvMode;
    }
    sSC1PRemixModeEnvMode = sc1PRemixModeFromName(value);

    if (sSC1PRemixModeEnvMode < 0)
    {
        port_log("SSB64_REMIX_1P_MODE=%s is not a mode (standard, bonus3, multiman, cruel, remix1p, allstar, hrc)\n", value);
    }
    return sSC1PRemixModeEnvMode;
}

s32 sc1PRemixModeGet(void)
{
    s32 env_mode = sc1PRemixModeReadEnv();

    return (env_mode >= 0) ? env_mode : gSC1PRemixMode;
}

sb32 sc1PRemixModeIs(s32 mode)
{
    return (sc1PRemixModeGet() == mode) ? TRUE : FALSE;
}

#endif
