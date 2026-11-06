#ifndef _MNREMIXTWELVECBDKMODERULES_H_
#define _MNREMIXTWELVECBDKMODERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * DK Mode and Lanky Mode easter egg of the 12-Character Battle character select (DKMode.asm _check_portraits, run by
 * TwelveCharBattle.asm update_portrait_id_table_ after every change of the Custom sets), free of game state so the headless unit
 * tests (tests/remix) can run it. The 24 portraits of the Custom set of each player are read as a 3 x 8 grid: a letter is drawn by
 * the portraits that show the character of the first portrait, every other portrait must show another character. Player 2 spelling
 * 'K' together with player 1 spelling 'D' turns DK Mode on, together with 'L' it turns Lanky Mode on.
 */

#define MNREMIX_TCB_DK_MODE_SLOTS 24

typedef enum MNRemixTwelveCBDKMode
{
    MNREMIX_TCB_DK_MODE_OFF,
    MNREMIX_TCB_DK_MODE_DK,
    MNREMIX_TCB_DK_MODE_LANKY
} MNRemixTwelveCBDKMode;

/**
 * Whether a table of 24 portraits draws a letter: bit n of the mask (slot n, row by row) is clear when the portrait shows the
 * character of the first portrait and set when it shows another one.
 *
 * @param fkinds Fighter kind of each of the 24 portrait slots.
 * @param letter_mask Letter pattern (MNREMIX_TCB_DK_MODE_MASK_*).
 * @return Non-zero when every portrait follows the mask.
 */
extern int mnRemixTwelveCBDKModeMatchesLetter(const int *fkinds, unsigned int letter_mask);

/**
 * The mode the Custom sets of both players switch on.
 *
 * @param player_one_fkinds Fighter kind of each of the 24 slots in the table of player 1.
 * @param player_two_fkinds Fighter kind of each of the 24 slots in the table of player 2.
 * @return MNREMIX_TCB_DK_MODE_DK for 'D' and 'K', MNREMIX_TCB_DK_MODE_LANKY for 'L' and 'K', otherwise MNREMIX_TCB_DK_MODE_OFF.
 */
extern int mnRemixTwelveCBDKModeSelect(const int *player_one_fkinds, const int *player_two_fkinds);

#endif

#endif
