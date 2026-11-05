#ifndef _IFREMIXCOUNTDOWN_H_
#define _IFREMIXCOUNTDOWN_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "if/ifremixcountdownrules.h"

/**
 * Plays one step of the stoplight countdown: the vanilla voice and the sounds the stage adds to it (Hazards.asm stoplight_1,
 * stoplight_2, stoplight_3, stoplight_go). The step 2 also plays the Pokemon announcer opening comment first, as the asm does.
 *
 * @param step IFRemixCountdownStep.
 * @param voice_fgm Vanilla FGM id of the step.
 */
extern void ifRemixCountdownPlayStep(IFRemixCountdownStep step, u16 voice_fgm);

#endif
#endif
