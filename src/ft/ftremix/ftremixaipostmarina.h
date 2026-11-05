#ifndef _FTREMIXAIPOSTMARINA_H_
#define _FTREMIXAIPOSTMARINA_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/**
 * Marina entry of the Remix AI post process (Marina/AI/Attacks.asm cpu_post_process): picks the throw of a jet snatch pull,
 * shakes the cargo victim, and decides when to use the charged or item-absorbing down special.
 */
void ftRemixAiPostProcessMarina(FTStruct *fp);

#endif

#endif
