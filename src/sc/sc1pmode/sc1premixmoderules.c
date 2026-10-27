/**
 * sc1premixmoderules.c - state-free rules of the Remix 1P modes, see sc1premixmoderules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixmoderules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

#define SC1PREMIX_SEED_FRAME_MASK 0x3F

int sc1PRemixModeLeavesBonusCSSFor1P(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_BONUS3:
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
    case SC1PREMIX_MODE_HRC:
        return 1;

    default:
        return 0;
    }
}

unsigned int sc1PRemixModeSeedAdvanceCount(unsigned int frame_count)
{
    return (frame_count & SC1PREMIX_SEED_FRAME_MASK) + 1;
}

#endif
