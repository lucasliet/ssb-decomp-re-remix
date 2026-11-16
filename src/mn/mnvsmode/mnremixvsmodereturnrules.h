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

/** Mode values of the menu that can be played in teams (VsRemixMenu.asm mode.KOTH, mode.SMASHKETBALL, mode.TUG_OF_WAR). */
#define MNREMIX_VSMODE_RETURN_MODE_DEFAULT 0
#define MNREMIX_VSMODE_RETURN_MODE_KOTH 3
#define MNREMIX_VSMODE_RETURN_MODE_SMASHKETBALL 4
#define MNREMIX_VSMODE_RETURN_MODE_TUG_OF_WAR 5

/**
 * TRUE when leaving the menu gives the fighters already chosen the costume of their team (VsRemixMenu.asm
 * save_global_settings_ :438-467): always for Smashketball, and for the default VS, King of the Hill and Tug of War when
 * their teams setting is on. 12-Character Battle and Tag Team never do.
 *
 * @param mode Mode the menu leaves with (0 for the default VS).
 * @param is_mode_teams Teams setting of that mode (the VS rule for the default VS).
 */
extern int mnRemixVSModeExitSyncsTeamCostumes(int mode, int is_mode_teams);

#endif

#endif
