#include "gr/grremix/grremixwallpaperrules.h"
#include "gr/grremix/grremixids.h"

/* SCBattleState game_status values for which the asm advances the background: Wait and Go. */
#define GRREMIX_WALLPAPER_STATUS_WAIT 0
#define GRREMIX_WALLPAPER_STATUS_GO 1

/** One set_bg_type call of Stages.asm: the background type of an added stage. */
typedef struct GRRemixWallpaperBgType
{
    int32_t gkind;
    int32_t kind;

} GRRemixWallpaperBgType;

/* Stages.asm 5065-5275: the set_bg_type calls. Every other added stage keeps bg_type.DEFAULT (the common wallpaper). */
static const GRRemixWallpaperBgType dGRRemixWallpaperBgTypes[] =
{
    { nGRRemixStage_FLAT_ZONE, GRREMIX_WALLPAPER_KIND_BONUS3 },
    { nGRRemixStage_DRAGONKING, GRREMIX_WALLPAPER_KIND_SECTOR },
    { nGRRemixStage_N64, GRREMIX_WALLPAPER_KIND_SECTOR },
    { nGRRemixStage_FROSTY, GRREMIX_WALLPAPER_KIND_SECTOR },
    { nGRRemixStage_FLAT_ZONE_2, GRREMIX_WALLPAPER_KIND_BONUS3 },
    { nGRRemixStage_SECTOR_Z_DL, GRREMIX_WALLPAPER_KIND_SECTOR },
    { nGRRemixStage_YOSHI_ISLAND_DL, GRREMIX_WALLPAPER_KIND_STATIC },
    { nGRRemixStage_SECTOR_Z_O, GRREMIX_WALLPAPER_KIND_SECTOR },
    { nGRRemixStage_YOSHI_ISLAND_O, GRREMIX_WALLPAPER_KIND_STATIC },
    { nGRRemixStage_GB_LAND, GRREMIX_WALLPAPER_KIND_BONUS3 },
    { nGRRemixStage_SECTOR_Z_REMIX, GRREMIX_WALLPAPER_KIND_SECTOR },
    { nGRRemixStage_DRAGONKING_REMIX, GRREMIX_WALLPAPER_KIND_SECTOR },
    { nGRRemixStage_REMIX_RTTF, GRREMIX_WALLPAPER_KIND_BONUS3 },
    { nGRRemixStage_N64_REMIX, GRREMIX_WALLPAPER_KIND_SECTOR }
};

/**
 * Background type the bg_type table gives a stage (Stages.asm bg_type, read at 0x80104C40).
 *
 * @param gkind Remix stage id.
 * @return GRREMIX_WALLPAPER_KIND_*, the common wallpaper when no set_bg_type call names the stage.
 */
static int32_t grRemixWallpaperRulesBgType(int32_t gkind)
{
    uint32_t index;

    for (index = 0; index < (sizeof(dGRRemixWallpaperBgTypes) / sizeof(dGRRemixWallpaperBgTypes[0])); index++)
    {
        if (dGRRemixWallpaperBgTypes[index].gkind == gkind)
        {
            return dGRRemixWallpaperBgTypes[index].kind;
        }
    }
    return GRREMIX_WALLPAPER_KIND_COMMON;
}

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
    return grRemixWallpaperRulesBgType(gkind);
}

float grRemixBlueBackgroundRulesSpeed(int32_t game_status)
{
    if ((game_status == GRREMIX_WALLPAPER_STATUS_WAIT) || (game_status == GRREMIX_WALLPAPER_STATUS_GO))
    {
        return GRREMIX_BLUE_BACKGROUND_SPEED;
    }
    return 0.0F;
}
