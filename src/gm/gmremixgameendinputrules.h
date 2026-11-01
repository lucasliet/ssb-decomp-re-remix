#ifndef _GMREMIXGAMEENDINPUTRULES_H_
#define _GMREMIXGAMEENDINPUTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Button comparison of the VS match end (GameEnd.asm update_screen_), pure so the headless unit tests (tests/remix) can run it.
 */

/**
 * Joypad.check_buttons_ with "match any" false: the held buttons of one controller must be exactly the mask, so a combination
 * with one extra button or one missing button does not match. GameEnd.asm calls it through Joypad.check_buttons_all_, which
 * ORs the result of the four controllers (the caller does that).
 *
 * @param held Held button bits of one controller.
 * @param mask Button bits of the combination.
 * @return 1 when held equals mask and mask is not empty, else 0.
 */
extern int gmRemixGameEndInputMatchesExactly(unsigned int held, unsigned int mask);

#endif

#endif
