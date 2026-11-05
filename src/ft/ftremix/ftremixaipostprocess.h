#ifndef _FTREMIXAIPOSTPROCESS_H_
#define _FTREMIXAIPOSTPROCESS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Per-frame post process of the Remix AI (AI.asm cpu_post_process), run after ftComputerProcessObjective and before ftComputerUpdateInputs. */

/**
 * Runs the Remix post process sub-behaviours of one CPU for this frame, in the asm order, ending with the routine of the
 * fighter kind when Character.cpu_post_process has an entry for it (ftremixaipost{usp,edge,nsp,input,marina,sheik}.c).
 */
void ftRemixAiPostProcess(FTStruct *fp);

#endif

#endif
