/**
 * gmremixpractice.c - 1P Game start stage of the Remix Practice_1P panel option (Practice_1P.asm _1p_practice).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scdef.h>
#include "gm/gmremixpractice.h"
#include "remix/remix_player_panel.h"

#define GMREMIX_PRACTICE_OFF 0

/* Practice value 1..11 (Stage 1..10, Final Stage) to nSC1PGameStage. The asm skips the bonus stages (Practice_1P.asm 41). */
static const s32 sGMRemixPracticeStages[] = {
    nSC1PGameStageLink,
    nSC1PGameStageYoshi,
    nSC1PGameStageFox,
    nSC1PGameStageMario,
    nSC1PGameStagePikachu,
    nSC1PGameStageDonkey,
    nSC1PGameStageKirby,
    nSC1PGameStageSamus,
    nSC1PGameStageMMario,
    nSC1PGameStageZako,
    nSC1PGameStageBoss,
};

/**
 * Stage of a 1P Game run. The option is read straight from the registry because the start runs before the battle state
 * holds the players, so the human-only guard of gmRemixPanel cannot be applied here.
 *
 * @param player Port index of the human fighter (0..3).
 * @param stage nSC1PGameStage the run starts on without the option.
 * @return The practice stage, or `stage` when the option is off.
 */
s32 gmRemixPracticeStartStage(s32 player, s32 stage)
{
    s32 practice_value = remix_panel_get(REMIX_PANEL_PRACTICE_1P, player);

    if (practice_value == GMREMIX_PRACTICE_OFF)
    {
        return stage;
    }
    return sGMRemixPracticeStages[practice_value - 1];
}

#endif
