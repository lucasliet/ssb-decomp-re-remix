#ifndef _IFREMIXGENTLEMENSRESET_H_
#define _IFREMIXGENTLEMENSRESET_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Gentlemen's Reset of the 12-Character Battle pause (TwelveCharBattle.asm gentlemens_reset_): the reset combo of the pause
 * (A + B + Z + R) no longer resets at once. The first press shows a request and hands the pause to the opponent, a second press
 * takes the game back and resets, START declines. Outside the mode the combo works as in the vanilla game.
 */

/**
 * Handles the reset combo of the pause.
 *
 * @param pause_player Port that controls the pause; passed to the opponent when the request is made.
 * @return TRUE when the combo only made the request (the pause stays), FALSE when the reset must go ahead.
 */
extern sb32 ifRemixGentlemensResetHandleCombo(u8 *pause_player);

/** Puts the request texts every frame while a request is shown. */
extern void ifRemixGentlemensResetUpdate(void);

/** TRUE while a reset request is shown. */
extern sb32 ifRemixGentlemensResetIsRequested(void);

/** Forgets the request and removes its sprites and texts; called when the pause objects go away. */
extern void ifRemixGentlemensResetEnd(void);

#endif

#endif
