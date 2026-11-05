#ifndef _GRREMIXWALLPAPERRULES_H_
#define _GRREMIXWALLPAPERRULES_H_

/*
 * Pure rules of the stage wallpaper (Stages.asm id_fix_1_, HRC.asm animate_background_image_ for Big Blue). Ints and floats
 * only, so the headless tests can run them.
 */

#include <stdint.h>

/** Wallpaper makers of a Remix stage: grWallpaperMakeStatic, grWallpaperMakeCommon and grWallpaperMakeBonus3. */
#define GRREMIX_WALLPAPER_KIND_STATIC 0
#define GRREMIX_WALLPAPER_KIND_COMMON 1
#define GRREMIX_WALLPAPER_KIND_BONUS3 2

/** Big Blue scrolls its background at this X speed per frame (asm word 0xBF17FDFF). */
#define GRREMIX_BLUE_BACKGROUND_SPEED (-0.5937194F)
#define GRREMIX_BLUE_BACKGROUND_SCALE_X 2.2334F
#define GRREMIX_BLUE_BACKGROUND_POS_Y 8.0F

/**
 * Wallpaper maker of a stage past the vanilla bonus stages. Remix Race to the Finish has no wallpaper (the Bonus3 maker), a
 * stage whose Stages.function_table entry is 0 (the added Break the Targets and Board the Platforms stages) takes the
 * static one, every other stage the common one.
 *
 * @param gkind Remix stage id.
 * @param function Stages.function_table entry of the stage (Remix RAM address, 0 when it has none).
 * @return GRREMIX_WALLPAPER_KIND_*.
 */
int32_t grRemixWallpaperRulesKind(int32_t gkind, uint32_t function);

/**
 * Scroll speed of the Big Blue background for the match status: it moves while the match waits to start or runs, and stands
 * still otherwise (the asm reads the pause byte of the match info, 0 or 1 when not paused).
 *
 * @param game_status SCBattleState game_status.
 */
float grRemixBlueBackgroundRulesSpeed(int32_t game_status);

#endif
