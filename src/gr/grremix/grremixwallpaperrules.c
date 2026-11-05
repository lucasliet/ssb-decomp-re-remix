#include "gr/grremix/grremixwallpaperrules.h"
#include "gr/grremix/grremixids.h"

/* SCBattleState game_status values for which the asm advances the background: Wait and Go. */
#define GRREMIX_WALLPAPER_STATUS_WAIT 0
#define GRREMIX_WALLPAPER_STATUS_GO 1

int32_t grRemixWallpaperRulesKind(int32_t gkind, uint32_t function)
{
    if (gkind == nGRRemixStage_REMIX_RTTF)
    {
        return GRREMIX_WALLPAPER_KIND_BONUS3;
    }
    if (function == 0)
    {
        return GRREMIX_WALLPAPER_KIND_STATIC;
    }
    return GRREMIX_WALLPAPER_KIND_COMMON;
}

float grRemixBlueBackgroundRulesSpeed(int32_t game_status)
{
    if ((game_status == GRREMIX_WALLPAPER_STATUS_WAIT) || (game_status == GRREMIX_WALLPAPER_STATUS_GO))
    {
        return GRREMIX_BLUE_BACKGROUND_SPEED;
    }
    return 0.0F;
}
