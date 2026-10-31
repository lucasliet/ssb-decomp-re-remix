#ifndef _FTREMIXAIPOSTSHEIK_H_
#define _FTREMIXAIPOSTSHEIK_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/**
 * Sheik entry of the Remix AI post process (Sheik/AI/Attacks.asm cpu_post_process): teleport steering during the USP and
 * the B press of the DSP control.
 */
void ftRemixAiPostProcessSheik(FTStruct *fp);

#endif

#endif
