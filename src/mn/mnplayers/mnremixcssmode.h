#ifndef _MNREMIXCSSMODE_H_
#define _MNREMIXCSSMODE_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Remix VS modes on the VS character select (TwelveCharBattle.asm update_css_header_ and update_announcer_on_entry_,
 * Smashketball.asm enable_toggling_mode_): the header image and the announcer of each mode, and the Smashketball type that
 * the game mode button toggles. The values live in mnremixcssmoderules.c.
 */

/**
 * Header image of the character select and of the results screen.
 *
 * @param vanilla_offset Offset of the Free-for-all or Team Battle header in the game modes file.
 * @return Offset of the header of the picked Remix mode, or `vanilla_offset` for the default mode.
 */
extern intptr_t mnRemixCssModeTitleOffset(intptr_t vanilla_offset);

/**
 * Plays the announcer of the picked Remix mode when the character select opens.
 *
 * @return TRUE when the Remix announcer played, FALSE when the vanilla Free-for-all or Team Battle voice must play.
 */
extern sb32 mnRemixCssModeAnnounce(void);

/** TRUE when the Free-for-all and Team Battle voice plays on the game mode button; the Remix modes skip it (update_announcer_on_toggle_). */
extern sb32 mnRemixCssModeKeepsToggleVoice(void);

/** TRUE when the picked mode is Smashketball. */
extern sb32 mnRemixCssModeIsSmashketball(void);

/**
 * Index of the colour of the mode title after the game mode button toggled: the Smashketball type in Smashketball (the button flips the
 * type, Smashketball.asm enable_toggling_mode_), the team battle flag otherwise.
 *
 * @param is_team_battle TRUE in a team battle.
 */
extern s32 mnRemixCssModeGetTitleColorIndex(sb32 is_team_battle);

/** Flips the Smashketball type between basketball and soccer (the game mode button of the character select). */
extern void mnRemixCssModeToggleSmashketballType(void);

/**
 * Smashketball start rule: exactly two teams on the panels. Does nothing but return TRUE outside Smashketball.
 *
 * @param pkinds Player kind of each of the four panels.
 * @param fkinds Fighter picked or hovered on each of the four panels.
 * @param teams Team of each of the four panels.
 * @return TRUE when the match may start.
 */
extern sb32 mnRemixCssModeTeamsAllowStart(const s32 *pkinds, const s32 *fkinds, const s32 *teams);

#endif

#endif
