/**
 * grremixhazards_baydraculatwilight.c - Remix hazard setups, group BayDraculaTwilight. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixwater.h"
#include "remix/remix_symbols.h"

#define GRREMIX_GREAT_BAY_WATER_Y -1128.0F
#define GRREMIX_GREAT_BAY_SPLASH_Y -828.0F
#define GRREMIX_TWILIGHT_CITY_WATER_Y 0.0F
#define GRREMIX_TWILIGHT_CITY_SPLASH_Y 0.0F

sb32 grRemixHazardsMakeGroundBayDraculaTwilight(s32 gkind, u32 function)
{
    switch (function)
    {
    case REMIX_SYM_Hazards__great_bay_setup:
        grRemixWaterMakeGround(GRREMIX_GREAT_BAY_WATER_Y, GRREMIX_GREAT_BAY_SPLASH_Y);
        return TRUE;

    case REMIX_SYM_Hazards__twilight_city_setup:
        grRemixWaterMakeGround(GRREMIX_TWILIGHT_CITY_WATER_Y, GRREMIX_TWILIGHT_CITY_SPLASH_Y);
        return TRUE;

    /* REMIX-TODO(CharEnvColor): draculas_castle_setup_ sets the global player env colour 0xFFA8C4FF; no port writer yet. */
    case REMIX_SYM_Hazards__draculas_castle_setup_:
        return TRUE;

    /* REMIX-TODO(CharEnvColor): reapers_setup_ sets the global player env colour 0xFF0000FF when hazards are on. */
    case REMIX_SYM_Hazards__reapers_setup_:
        return TRUE;

    default:
        return FALSE;
    }
}

#endif
