#ifndef _MNREMIXVSMODERETURNRULES_H_
#define _MNREMIXVSMODERETURNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rule of the Remix VS Game Mode menu when the player comes back from the VS CSS (VsRemixMenu.asm
 * set_cursor_index_, 0x801336B4): after a Remix mode the menu reopens on the Remix Modes page with the cursor on the mode
 * that was played; after VS Start or from any other scene it opens on page 1 as the vanilla menu does. */

#define MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR (-1)

/** First and last Remix mode values of the menu (12CB, Tag Team, King of the Hill, Smashketball, Tug of War). */
#define MNREMIX_VSMODE_RETURN_MODE_FIRST 1
#define MNREMIX_VSMODE_RETURN_MODE_LAST 5

/**
 * Cursor index on the Remix Modes page for a return from the VS CSS (vs_mode_flag - 1, the button index of the mode), or
 * MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR when the menu opens on page 1.
 *
 * @param is_from_players_vs TRUE when the previous scene is the VS CSS.
 * @param mode Remix mode that was chosen on the menu (0 for VS Start).
 */
extern int mnRemixVSModeReturnCursor(int is_from_players_vs, int mode);

#endif

#endif
