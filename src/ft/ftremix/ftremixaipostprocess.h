#ifndef _FTREMIXAIPOSTPROCESS_H_
#define _FTREMIXAIPOSTPROCESS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Per-frame post process of the Remix AI (AI.asm cpu_post_process), run after ftComputerUpdateInputs. */

/**
 * Runs the Remix post process sub-behaviours of one CPU for this frame, in the asm order. The per-fighter post process
 * table is not ported yet (REMIX-TODO(AI) in ftremixaipostprocess.c).
 */
void ftRemixAiPostProcess(FTStruct *fp);

#endif

#endif
