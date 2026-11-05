#ifndef _SC1PREMIXENDING_H_
#define _SC1PREMIXENDING_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Own ending of the Remix 1P and All-Star runs (BGM.asm remix_1P_ending_song): the credits play a song chosen by the fighter the
 * player used.
 */

/**
 * Song the credits play.
 *
 * @param standard_song Song the credits play outside the Remix 1P and All-Star runs.
 * @return The song of the fighter in those two modes, the given one otherwise.
 */
extern u32 sc1PRemixEndingGetCreditsSong(u32 standard_song);

#endif

#endif
