/**
 * grremixhazards_baydraculatwilight.c - Remix hazard setups, group BayDraculaTwilight. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixwater.h"
#include "remix/remix_symbols.h"
#include <mp/map.h>

#define GRREMIX_GREAT_BAY_WATER_Y -1128.0F
#define GRREMIX_GREAT_BAY_SPLASH_Y -828.0F
#define GRREMIX_TWILIGHT_CITY_WATER_Y 0.0F
#define GRREMIX_TWILIGHT_CITY_SPLASH_Y 0.0F
#define GRREMIX_DRACULA_ENV_COLOR 0xFFA8C4FF  /* asm draculas_castle_setup_ */
#define GRREMIX_REAPERS_ENV_COLOR 0xFF0000FF  /* asm reapers_setup_ */

/** Writes the stage light colour fighters are drawn with (asm set_player_env_colour_, RAM 0x80131388). */
static void grRemixSetPlayerEnvColor(u32 rgba)
{
    gMPCollisionLightColor.r = (u8)(rgba >> 24);
    gMPCollisionLightColor.g = (u8)(rgba >> 16);
    gMPCollisionLightColor.b = (u8)(rgba >> 8);
    gMPCollisionLightColor.a = (u8)rgba;
}

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

    case REMIX_SYM_Hazards__draculas_castle_setup_:
        grRemixSetPlayerEnvColor(GRREMIX_DRACULA_ENV_COLOR);
        return TRUE;

    case REMIX_SYM_Hazards__reapers_setup_:
        if (grRemixHazardsAreOn())
        {
            grRemixSetPlayerEnvColor(GRREMIX_REAPERS_ENV_COLOR);
        }
        return TRUE;

    default:
        return FALSE;
    }
}

#endif
