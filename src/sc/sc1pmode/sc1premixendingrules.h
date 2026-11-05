#ifndef _SC1PREMIXENDINGRULES_H_
#define _SC1PREMIXENDINGRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the own ending of the Remix 1P and All-Star runs (BGM.asm remix_1P_ending_song), kept apart so the headless unit tests
 * (tests/remix) run them. The credits of these two modes play the song of the fighter the player used instead of the standard one.
 */

/**
 * Whether the credits play the ending song of the fighter: the run was Remix 1P or All-Star. The credits opened from the debug
 * settings screen keep the standard song (the asm compares the previous screen with the settings screen id 0x39, which the port does
 * not have), and the standard 1P keeps it as well.
 *
 * @param mode Mode id of the run (SC1PREMIX_MODE_*).
 */
extern int sc1PRemixEndingUsesFighterSong(int mode);

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
