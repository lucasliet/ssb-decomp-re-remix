/**
 * sc1premixscorerules.c - state-free rules of the Remix 1P score tables, see sc1premixscorerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixscorerules.h"

int sc1PRemixScoreTableOf(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_REMIX_1P:
        return SC1PREMIX_SCORE_TABLE_REMIX_1P;

    case SC1PREMIX_MODE_ALLSTAR:
        return SC1PREMIX_SCORE_TABLE_ALLSTAR;

    default:
        return SC1PREMIX_SCORE_TABLE_STANDARD;
    }
}

int sc1PRemixScoreReplaces(unsigned int stored, unsigned int score)
{
    return (stored < score) ? 1 : 0;
}

#endif
