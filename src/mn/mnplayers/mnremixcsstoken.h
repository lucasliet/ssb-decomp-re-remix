#ifndef _MNREMIXCSSTOKEN_H_
#define _MNREMIXCSSTOKEN_H_

#include <ssb_types.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Token (puck) helpers shared by the four Remix character selects (src/CharacterSelect.asm adjust_created_token_,
 * adjust_changed_token_, adjust_scaled_token_position_, set_white_flash_texture). The rules are in mnremixcsstokenrules.h.
 */

/** Offset of the white flash image from its portrait image in the Remix portraits file (set_white_flash_texture: 0x860). */
#define MNREMIX_CSS_TOKEN_FLASH_DELTA 0x860

/**
 * Shrinks a token sprite to the Remix portrait scale (0.8125) once the sprite exists, and again after it was rebuilt (a Man to CPU
 * change makes a new sprite).
 *
 * @param token Token GObj.
 */
extern void mnRemixCssTokenScale(GObj *token);

/**
 * Puts a held token on the hand: the hand position + (12, -10).
 *
 * @param token Token GObj.
 * @param cursor Cursor GObj of the hand holding it.
 */
extern void mnRemixCssTokenFollow(GObj *token, GObj *cursor);

/**
 * Slot a fighter's token rests on, in the portrait numbering of the asm: the grid cell of the fighter or of the fighter it is a
 * D-pad variant of, 30 for a bonus fighter, 31 for Random; any other fighter reads as slot 0.
 *
 * @param fkind Fighter kind, equal to the Remix id.
 * @return Slot 0 to 31.
 */
extern s32 mnRemixCssTokenGetSlot(s32 fkind);

/**
 * Fighter of the bookend a token is over: RANDOM on the Random bookend, the bonus fighter the token has scrolled to on the bonus bookend.
 * No lock, cross or variant rule applies; the caller does that.
 *
 * @param pos_x Token centre x (token x + 13).
 * @param pos_y Token centre y (token y + 12).
 * @param token Index the bonus scroll is kept under.
 * @param fkind Receives the fighter kind when the token is over a bookend.
 * @return TRUE when the token is over a bookend.
 */
extern sb32 mnRemixCssTokenGetBookendFighter(s32 pos_x, s32 pos_y, s32 token, s32 *fkind);

/**
 * Puts a token on the portrait of a fighter (place_token_from_id_).
 *
 * @param token Token GObj.
 * @param fkind Fighter kind of the portrait.
 */
extern void mnRemixCssTokenPlace(GObj *token, s32 fkind);

/**
 * Makes the white flash sprite of any portrait slot: the grid portrait flash, or the flash of the bonus or Random bookend.
 *
 * @param gobj Flash GObj, which the caller already linked to the flash display list.
 * @param portraits_file Loaded Remix portraits file.
 * @param images_file Loaded Remix CSS images file.
 * @param slot Portrait slot, 0 to 31.
 */
extern void mnRemixCssTokenMakeSlotFlash(GObj *gobj, void *portraits_file, void *images_file, s32 slot);

/**
 * Makes the white flash sprite of a grid portrait: the 32x32 flash image next to the portrait image, at the portrait position
 * and scale (set_white_flash_texture).
 *
 * @param gobj Flash GObj, which the caller already linked to the flash display list.
 * @param portraits_file Loaded Remix portraits file.
 * @param slot Grid slot, 0 to 29.
 */
extern void mnRemixCssTokenMakeFlash(GObj *gobj, void *portraits_file, s32 slot);

#endif

#endif
