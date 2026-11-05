#if defined(PORT) && defined(REGION_REMIX)

#ifndef _FTREMIXPARTIMAGE_H_
#define _FTREMIXPARTIMAGE_H_

#include <ft/fighter.h>

/*
 * Image and palette swaps of model parts (Render.MODEL_PART_IMAGE_INIT_ is gcAddMObjForDObj, the swaps are the MObj fields
 * that Costumes.asm check_for_custom_costume_ and Shield.asm alter_yoshi_egg_color_ write). See docs/remix/notes/i7_part_image.md.
 */

/**
 * Applies the extra costume of a fighter to the MObjs of its model parts (Costumes.asm check_for_custom_costume_): palette array and
 * palette index, prim color and diffuse/ambient colors of every part image the character's part table lists. Does nothing for a
 * vanilla costume or a character with no extra costumes, and is idempotent, so it is safe to call every frame. The match calls it from
 * ftDisplayMainProcDisplay; the CSS must call it after it makes or re-costumes a fighter model.
 */
void ftRemixPartImageApplyFighter(FTStruct *fp);

/** Number of extra costumes of `fkind` (byte 0 of its Costumes.extra_costumes_table part table), 0 when it has none. */
s32 ftRemixPartImageGetExtraCostumeCount(s32 fkind);

/** Highest costume id a player can reach with `fkind` (Costumes.asm select_, t4), vanilla ids plus skipped ids plus extra costumes. */
s32 ftRemixPartImageGetMaxCostumeID(s32 fkind);

/** Costume after a C right press, with the extra costumes of the character (Costumes.asm select_ _right). */
s32 ftRemixPartImageGetNextCostume(s32 fkind, s32 costume);

/** Costume after a C left press, with the extra costumes of the character (Costumes.asm select_ _left). */
s32 ftRemixPartImageGetPrevCostume(s32 fkind, s32 costume);

/** TRUE when a Random pick may land on `costume` (Costumes.asm get_random_legal_costume_ skips the unusable ids). */
sb32 ftRemixPartImageIsLegalRandomCostume(s32 fkind, s32 costume);

/**
 * Stock icon LUT token of a fighter for the match HUD (Costumes.asm icon_palette_override_): the Remix palette of an extra costume, or
 * `vanilla_lut` (the token the HUD would use) for any other costume. ifcommon.c indexes the vanilla array by the costume id, which
 * reads past its end for an extra costume.
 */
u32 ftRemixPartImageGetStockIconLUT(const FTStruct *fp, u32 vanilla_lut);

/**
 * Costume id of a fighter at creation (ftManagerMakeFighter), `costume` unless SSB64_REMIX_COSTUME_P<port 1..4> overrides it. Debug
 * only: the VS shortcut always makes costume 0.
 */
s32 ftRemixPartImageDebugCostume(s32 player, s32 costume);

/**
 * Yoshi egg shield colour (Shield.asm alter_yoshi_egg_color_ _shield and _shield_roll). Called from efManagerYoshiShieldProcDisplay
 * before the shield is drawn: on first use it replaces the palette load of the egg display list with a call to the MObj display
 * list and gives the shield DObj an MObj with the 16 Shield.asm palettes, then it sets the palette index of the fighter's
 * Shield option (Costume picks the costume's colour, every other option the default egg palette).
 */
void ftRemixPartImageYoshiEggShieldApply(GObj *effect_gobj, FTStruct *fp);

#endif

#endif
