/**
 * wpmarioremixfireballrules.c - pure rules of Mad Piano's book, see wpmarioremixfireballrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "wp/wpmario/wpmarioremixfireballrules.h"

int wpRemixPianoBookRulesCount(int bonus_ammo)
{
    if (bonus_ammo <= 0)
    {
        return 1;
    }
    return bonus_ammo + 1;
}

float wpRemixPianoBookRulesSpreadIntensity(int iteration)
{
    return 1.0F + (WPREMIX_PIANO_BOOK_SPREAD_STEP * (float)iteration);
}

#endif
