/**
 * sc1premixhighscores.c - the 1P record switch (SinglePlayer.high_score_enabled), see sc1premixhighscores.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixenemy.h"
#include "sc/sc1pmode/sc1premixhighscores.h"
#include "sc/sc1pmode/sc1premixhighscoresrules.h"

static sb32 sSC1PRemixHighScoresIsEnabled = TRUE;

void sc1PRemixHighScoresSetEnabled(sb32 is_enabled)
{
    sSC1PRemixHighScoresIsEnabled = (is_enabled != FALSE) ? TRUE : FALSE;
}

sb32 sc1PRemixHighScoresIsEnabled(void)
{
    return sSC1PRemixHighScoresIsEnabled;
}

sb32 sc1PRemixHighScoresCanSaveRecord(void)
{
    return (sc1PRemixHighScoresAllowSave(sSC1PRemixHighScoresIsEnabled, sc1PRemixEnemyPortGet()) != 0) ? TRUE : FALSE;
}

sb32 sc1PRemixHighScoresCanSaveKoRecord(void)
{
    return (sc1PRemixHighScoresAllowKoSave(sSC1PRemixHighScoresIsEnabled) != 0) ? TRUE : FALSE;
}

#endif
