#ifndef _GRREMIXWALLPAPER_H_
#define _GRREMIXWALLPAPER_H_

/*
 * Stage wallpaper of the Remix build. grwallpaper.c calls these under #ifdef REGION_REMIX (Stages.asm id_fix_1_, HRC.asm
 * animate_background_image_ for Big Blue). The rules are in grremixwallpaperrules.h.
 */

#include <ssb_types.h>
#include <sys/objdef.h>

/**
 * Builds the wallpaper of a stage past the vanilla bonus stages: the static, common or Bonus3 maker of
 * grRemixWallpaperRulesKind. Also puts the Big Blue background scroll back at its start.
 *
 * @param gkind Remix stage id.
 */
void grRemixWallpaperMake(s32 gkind);

/**
 * Scrolls the background image of Big Blue in a VS match by one step; no-op elsewhere.
 *
 * @param wallpaper_sobj Background image, after grWallpaperCalcPersp.
 */
void grRemixBlueBackgroundApply(SObj *wallpaper_sobj);

#endif
