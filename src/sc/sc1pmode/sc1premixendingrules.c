/**
 * sc1premixendingrules.c - state-free rules of the own ending of the Remix 1P modes, see sc1premixendingrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixendingrules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

int sc1PRemixEndingUsesFighterSong(int mode)
{
    return ((mode == SC1PREMIX_MODE_REMIX_1P) || (mode == SC1PREMIX_MODE_ALLSTAR)) ? 1 : 0;
}

unsigned int sc1PRemixEndingPickSong(unsigned int table_entry, unsigned int standard_song)
{
    return (table_entry != 0) ? table_entry : standard_song;
}

#endif
