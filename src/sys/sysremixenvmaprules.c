/**
 * sysremixenvmaprules.c - state-free screen rule of the Remix environment mapping fix, see sysremixenvmaprules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sys/sysremixenvmaprules.h"

#define SYSREMIX_ENVMAP_SCREEN_VS_PREVIEW 0x0E
#define SYSREMIX_ENVMAP_SCREEN_CSS_FIRST 0x10
#define SYSREMIX_ENVMAP_SCREEN_CSS_LAST 0x14
#define SYSREMIX_ENVMAP_SCREEN_RESULTS 0x18
#define SYSREMIX_ENVMAP_SCREEN_ROOM 0x30
#define SYSREMIX_ENVMAP_SCREEN_CONTINUE 0x31

int sysRemixEnvMapIsLookAtScreen(int screen_id)
{
    if ((screen_id >= SYSREMIX_ENVMAP_SCREEN_CSS_FIRST) && (screen_id <= SYSREMIX_ENVMAP_SCREEN_CSS_LAST))
    {
        return 1;
    }
    switch (screen_id)
    {
    case SYSREMIX_ENVMAP_SCREEN_VS_PREVIEW:
    case SYSREMIX_ENVMAP_SCREEN_RESULTS:
    case SYSREMIX_ENVMAP_SCREEN_ROOM:
    case SYSREMIX_ENVMAP_SCREEN_CONTINUE:
        return 1;

    default:
        return 0;
    }
}

#endif
