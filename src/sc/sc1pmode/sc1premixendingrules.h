#ifndef _SC1PREMIXENDINGRULES_H_
#define _SC1PREMIXENDINGRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the own ending of the Remix 1P and All-Star runs (BGM.asm remix_1P_ending_song), kept apart so the headless unit tests
 * (tests/remix) run them. The credits of these two modes play the song of the fighter the player used instead of the standard one.
 */

/**
 * Whether the credits play the ending song of the fighter: the run was Remix 1P or All-Star and the credits follow its ending.
 * The credits opened from the settings screen keep the standard song whatever mode is stored (the asm compares the previous
 * screen with the settings screen id 0x39; the port opens them from its menu, Other Screens), and the standard 1P keeps it as well.
 *
 * @param mode Mode id of the run (SC1PREMIX_MODE_*).
 * @param is_opened_from_menu Non-zero when the credits were opened from the port menu and not by the end of a 1P run.
 */
extern int sc1PRemixEndingUsesFighterSong(int mode, int is_opened_from_menu);

/**
 * Song of the credits: the entry of the fighter in Character.remix_1p_end_bgm.table, or the standard song when the entry is zero
 * (the asm treats zero as the Dream Land music index and keeps the standard one).
 *
 * @param table_entry Halfword of the table for the fighter.
 * @param standard_song Song the credits play by default.
 */
extern unsigned int sc1PRemixEndingPickSong(unsigned int table_entry, unsigned int standard_song);

#endif

#endif
