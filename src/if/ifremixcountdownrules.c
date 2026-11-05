/**
 * ifremixcountdownrules.c - pure stoplight countdown sound rules, see ifremixcountdownrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixcountdownrules.h"
#include "gr/grremix/grremixids.h"

#define IFREMIX_COUNTDOWN_FGM_FZERO_FIRST 0x5CB
#define IFREMIX_COUNTDOWN_FGM_MK64_COUNT 0x405
#define IFREMIX_COUNTDOWN_FGM_MK64_GO 0x406

IFRemixCountdownSounds ifRemixCountdownRulesGetSounds(int gkind, int step)
{
    IFRemixCountdownSounds sounds = { IFREMIX_COUNTDOWN_NO_FGM, 0, 0 };

    if (gkind == nGRRemixStage_BLUE)
    {
        sounds.extra_fgm = IFREMIX_COUNTDOWN_FGM_FZERO_FIRST + step;
        sounds.is_voice_replaced = 1;
        return sounds;
    }
    if ((gkind != nGRRemixStage_TOADSTURNPIKE) && (gkind != nGRRemixStage_RAINBOWROAD))
    {
        return sounds;
    }
    if (step == nIFRemixCountdownStepGo)
    {
        sounds.extra_fgm = IFREMIX_COUNTDOWN_FGM_MK64_GO;
        sounds.is_extra_first = 1;
        return sounds;
    }
    sounds.extra_fgm = IFREMIX_COUNTDOWN_FGM_MK64_COUNT;
    return sounds;
}

#endif
