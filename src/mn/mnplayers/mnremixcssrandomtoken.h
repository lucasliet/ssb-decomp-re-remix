#ifndef _MNREMIXCSSRANDOMTOKEN_H_
#define _MNREMIXCSSRANDOMTOKEN_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Random token of the 1P and Training character selects (src/CharacterSelect.asm save_random_char_ and load_random_1p_ and
 * load_random_training_): the token on the Random bookend is rolled into a real fighter when the screen is left, and the "?" token
 * comes back when the screen is entered again.
 */

#define MNREMIX_CSS_RANDOM_TOKEN_SCREEN_1P 0
#define MNREMIX_CSS_RANDOM_TOKEN_SCREEN_TRAINING 1
#define MNREMIX_CSS_RANDOM_TOKEN_PORTS 4

/** A fighter the Random roll must skip (locked or crossed out). */
typedef sb32 (*MNRemixCssFighterFilter)(s32 fkind);

/**
 * Rolls the fighter a Random token stands for (get_random_char_id_): a pick of the pool that is not unavailable and passes the recent
 * randoms of the port.
 *
 * @param port Port of the token.
 * @param is_unavailable Tells the fighters the screen does not allow.
 * @return Fighter kind.
 */
extern s32 mnRemixCssRandomTokenRoll(s32 port, MNRemixCssFighterFilter is_unavailable);

/** Remembers that the token of a port was a Random one rolled when the screen was left (random_char_flag_*). */
extern void mnRemixCssRandomTokenMarkResolved(s32 screen, s32 port);

/**
 * TRUE once after the token of a port was rolled, so the screen puts the "?" token back; the mark is cleared.
 *
 * @param screen MNREMIX_CSS_RANDOM_TOKEN_SCREEN_*.
 * @param port Port of the token.
 */
extern sb32 mnRemixCssRandomTokenTakeResolved(s32 screen, s32 port);

#endif

#endif
