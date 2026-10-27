/**
 * sc1premixhrcsignrules.c - digits of the Home-Run Contest distance signs, see sc1premixhrcsignrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixhrcsignrules.h"

#define SC1PREMIX_HRC_SIGN_FARTHEST_HUNDREDS 3
#define SC1PREMIX_HRC_SIGN_DIGIT_WRAP 10
#define SC1PREMIX_HRC_SIGN_PAIR_WRAP 11
#define SC1PREMIX_HRC_SIGN_PAIR_FIRST 1
#define SC1PREMIX_HRC_SIGN_MAX_DIGIT 9

void sc1PRemixHrcSignReset(SC1PRemixHrcSign *sign)
{
    sign->state = SC1PREMIX_HRC_SIGN_STATE_UNDER_10K;
    sign->thousands = 0;
    sign->hundreds = SC1PREMIX_HRC_SIGN_FARTHEST_HUNDREDS;
    sign->hundred_pair = 0;
}

static void sc1PRemixHrcSignSetMaxed(SC1PRemixHrcSign *sign)
{
    sign->thousands = SC1PREMIX_HRC_SIGN_MAX_DIGIT;
    sign->hundreds = SC1PREMIX_HRC_SIGN_MAX_DIGIT;
    sign->hundred_pair = SC1PREMIX_HRC_SIGN_PAIR_WRAP;
}

void sc1PRemixHrcSignAdvance(SC1PRemixHrcSign *sign)
{
    if (sign->state == SC1PREMIX_HRC_SIGN_STATE_MAXED)
    {
        return;
    }
    if (sign->state == SC1PREMIX_HRC_SIGN_STATE_UNDER_10K)
    {
        sign->hundreds++;
    }
    else sign->hundred_pair++;

    if (sign->hundred_pair == SC1PREMIX_HRC_SIGN_PAIR_WRAP)
    {
        sign->hundreds++;
    }
    if (sign->hundreds == SC1PREMIX_HRC_SIGN_DIGIT_WRAP)
    {
        sign->thousands++;
    }
    if (sign->thousands == SC1PREMIX_HRC_SIGN_DIGIT_WRAP)
    {
        sign->state++;
    }
    if (sign->hundred_pair == SC1PREMIX_HRC_SIGN_PAIR_WRAP)
    {
        sign->hundred_pair = SC1PREMIX_HRC_SIGN_PAIR_FIRST;
    }
    if (sign->hundreds == SC1PREMIX_HRC_SIGN_DIGIT_WRAP)
    {
        sign->hundreds = 0;
    }
    if (sign->thousands != SC1PREMIX_HRC_SIGN_DIGIT_WRAP)
    {
        return;
    }
    sign->thousands = 1;

    if (sign->state != SC1PREMIX_HRC_SIGN_STATE_UNDER_100K)
    {
        sc1PRemixHrcSignSetMaxed(sign);
        return;
    }
    sign->hundred_pair = SC1PREMIX_HRC_SIGN_PAIR_FIRST;
}

#endif
