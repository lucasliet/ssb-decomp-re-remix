#ifndef _MNREMIXCSSBONUS_H_
#define _MNREMIXCSSBONUS_H_

#include <ssb_types.h>
#include <PR/os.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Bonus bookend of the Remix VS character select (src/CharacterSelect.asm): a tall portrait at the left of the grid that
 * holds the ten bonus fighters. A token over it picks one of them, and Z or R held scroll the list.
 */

/** Forgets the scene objects and the scroll state; called when the character select builds its portraits. */
extern void mnRemixCssBonusReset(void);

/**
 * Makes the bookend sprite (File.CSS_IMAGES, portrait_offsets.BONUS_BOOKEND) at the left of the grid.
 *
 * @param css_images_file Loaded Remix CSS images file.
 * @param gobj_link GObj link of the sprite.
 * @param dl_link Display list link of the sprite.
 */
extern void mnRemixCssBonusMakeBookend(void *css_images_file, s32 gobj_link, s32 dl_link);

/** Fighter the bonus bookend shows for the token of a player (bonus_chars[bonus_char_index_vs[token]]). */
extern s32 mnRemixCssBonusGetFighterKind(s32 token);

/**
 * Scrolls the bonus list of a held token while Z or R is held on the cursor that holds it (check_variant_input_ in the
 * asm). Call it every frame for a cursor holding a token over the bookend, and with is_over_bookend FALSE otherwise so the
 * repeat timer rearms.
 *
 * @param token Player whose token is held; his list is the one that scrolls.
 * @param cursor_player Player whose cursor holds the token; his repeat timer counts.
 * @param held_buttons Held button mask of the cursor controller.
 * @param is_over_bookend TRUE while the token is over the bonus bookend.
 * @return TRUE on a frame that changed the fighter.
 */
extern sb32 mnRemixCssBonusScroll(s32 token, s32 cursor_player, u16 held_buttons, sb32 is_over_bookend);

/**
 * Moves the scroll hint to another panel column: the hint is placed from the left edge of the panel card, which is not the VS one on
 * the Bonus and 1P screens. Call it before the first hint; Reset restores the VS position.
 *
 * @param panel_x Left edge of the card of panel 0.
 */
extern void mnRemixCssBonusSetHintPanelX(f32 panel_x);

/**
 * Shows or hides the scroll hint of a panel (draw_bonus_scroll_indicators_): blinking arrows at both sides and the Z and R
 * buttons, created on first use.
 *
 * @param panel Panel of the token that hovers the bookend.
 * @param is_visible TRUE while that token is held over the bookend.
 * @param arrows_file Loaded VS players common file (the arrow sprites).
 */
extern void mnRemixCssBonusUpdateHint(s32 panel, sb32 is_visible, void *arrows_file);

#endif

#endif
