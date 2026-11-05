#ifndef _MNREMIXCSSRANDOM_H_
#define _MNREMIXCSSRANDOM_H_

#include <ssb_types.h>
#include <PR/os.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Random bookend of the Remix VS character select (src/CharacterSelect.asm): a tall portrait right of the grid. A token dropped on
 * it holds the RANDOM fighter (id 0x1B, the "?" model); the real fighter is rolled when the battle starts, and the roll avoids
 * the fighters that player got in his last six Random rolls.
 */

#define MNREMIX_CSS_RANDOM_PLAYERS 4

/** Offset of the white flash image of the Random bookend from the bookend image (set_white_flash_texture: 0xC60). */
#define MNREMIX_CSS_RANDOM_FLASH_DELTA 0xC60

/**
 * Makes the bookend sprite (File.CSS_IMAGES, portrait_offsets.RANDOM_BOOKEND) right of the grid.
 *
 * @param css_images_file Loaded Remix CSS images file.
 * @param gobj_link GObj link of the sprite.
 * @param dl_link Display list link of the sprite.
 */
extern void mnRemixCssRandomMakeBookend(void *css_images_file, s32 gobj_link, s32 dl_link);

/**
 * Makes the white flash sprite shown over the bookend when a token is dropped on it.
 *
 * @param gobj Flash GObj, which the caller already linked to the flash display list.
 * @param css_images_file Loaded Remix CSS images file.
 */
extern void mnRemixCssRandomMakeFlash(GObj *gobj, void *css_images_file);

/**
 * Decides whether a Random roll for a player is kept (_recent_randoms_lookup): a fighter that is not in the player's last six
 * rolls is kept; one that is gets kept with probability 1 / (times it is in the list + 1). A kept roll enters the list.
 *
 * @param port Player the roll is for.
 * @param fkind Fighter just rolled.
 * @return TRUE when the roll is kept, FALSE when the caller must roll again.
 */
extern sb32 mnRemixCssRandomKeepRoll(s32 port, s32 fkind);

#endif

#endif
