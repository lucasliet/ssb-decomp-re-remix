/**
 * sc1premixhighscoresrules.c - state-free rules of the 1P record switch, see sc1premixhighscoresrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixhighscoresrules.h"

int sc1PRemixHighScoresAllowSave(int high_scores_enabled, int enemy_port)
{
    if (high_scores_enabled == 0)
    {
        return 0;
    }
    return (enemy_port == 0) ? 1 : 0;
}

int sc1PRemixHighScoresAllowKoSave(int high_scores_enabled)
{
    return (high_scores_enabled != 0) ? 1 : 0;
}

#endif
