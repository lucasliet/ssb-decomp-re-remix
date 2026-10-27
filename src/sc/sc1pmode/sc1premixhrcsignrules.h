#ifndef _SC1PREMIXHRCSIGNRULES_H_
#define _SC1PREMIXHRCSIGNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Digits of the distance signs of the Home-Run Contest (HRC.asm main_ "update signs" and initialize_signs_). Each sign draws three digit
 * images: the 00/X00 pair, the hundreds/thousands digit and the thousands/ten thousands digit. Kept apart so the headless unit tests
 * (tests/remix) run them.
 */

#define SC1PREMIX_HRC_SIGN_STATE_UNDER_10K 0
#define SC1PREMIX_HRC_SIGN_STATE_UNDER_100K 1
#define SC1PREMIX_HRC_SIGN_STATE_MAXED 2

/** Digit image indices of the farthest sign plus the range it is in. */
typedef struct SC1PRemixHrcSign
{
    int state;
    int thousands;
    int hundreds;
    int hundred_pair;

} SC1PRemixHrcSign;

/**
 * Digits of the farthest sign right after the setup: the hundreds image counts from 3 and the other two are blank.
 *
 * @param sign Receives the digits.
 */
extern void sc1PRemixHrcSignReset(SC1PRemixHrcSign *sign);

/**
 * Moves the farthest sign 100 ft further, as the camera warps back by that distance. Stays at 99,999 once reached.
 *
 * @param sign Digits of the sign, updated.
 */
extern void sc1PRemixHrcSignAdvance(SC1PRemixHrcSign *sign);

#endif

#endif
