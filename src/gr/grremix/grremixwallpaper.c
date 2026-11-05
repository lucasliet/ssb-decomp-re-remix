/**
 * grremixwallpaper.c - wallpaper kind of the Remix stages and Big Blue background scroll. See grremixwallpaper.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "grremixids.h"
#include "grremixstage.h"
#include "grremixwallpaper.h"
#include "grremixwallpaperrules.h"
#include <gr/ground.h>
#include <sc/scene.h>
#include <sc/scmanager.h>
#include <sc/sc1pmode/sc1premixhrcrules.h>

/* Scale and X position the asm keeps in the background object; set on the first frame of the match. */
static sb32 sGRRemixBlueBackgroundIsInitialized;
static f32 sGRRemixBlueBackgroundX;
static f32 sGRRemixBlueBackgroundScaleY;

static void grRemixBlueBackgroundReset(void)
{
    sGRRemixBlueBackgroundIsInitialized = FALSE;
}

void grRemixWallpaperMake(s32 gkind)
{
    grRemixBlueBackgroundReset();

    switch (grRemixWallpaperRulesKind(gkind, grRemixGetGroundFunction(gkind)))
    {
    case GRREMIX_WALLPAPER_KIND_BONUS3:
        grWallpaperMakeBonus3();
        break;

    case GRREMIX_WALLPAPER_KIND_STATIC:
        grWallpaperMakeStatic();
        break;

    default:
        grWallpaperMakeCommon();
        break;
    }
}

static sb32 grRemixBlueBackgroundIsActive(void)
{
    if (gSCManagerSceneData.scene_curr != nSCKindVSBattle)
    {
        return FALSE;
    }
    return (gSCManagerBattleState->gkind == nGRRemixStage_BLUE) ? TRUE : FALSE;
}

static void grRemixBlueBackgroundInitialize(SObj *wallpaper_sobj)
{
    sGRRemixBlueBackgroundX = wallpaper_sobj->pos.x;
    sGRRemixBlueBackgroundScaleY = wallpaper_sobj->sprite.scaley;
    sGRRemixBlueBackgroundIsInitialized = TRUE;
}

void grRemixBlueBackgroundApply(SObj *wallpaper_sobj)
{
    if (grRemixBlueBackgroundIsActive() == FALSE)
    {
        return;
    }
    if (sGRRemixBlueBackgroundIsInitialized == FALSE)
    {
        grRemixBlueBackgroundInitialize(wallpaper_sobj);
    }
    sGRRemixBlueBackgroundX = sc1PRemixHrcCloudStep(sGRRemixBlueBackgroundX, grRemixBlueBackgroundRulesSpeed(gSCManagerBattleState->game_status));

    wallpaper_sobj->pos.x = sGRRemixBlueBackgroundX;
    wallpaper_sobj->pos.y = GRREMIX_BLUE_BACKGROUND_POS_Y;
    wallpaper_sobj->sprite.scalex = GRREMIX_BLUE_BACKGROUND_SCALE_X;
    wallpaper_sobj->sprite.scaley = sGRRemixBlueBackgroundScaleY;
}

#endif
