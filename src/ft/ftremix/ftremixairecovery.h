#ifndef _FTREMIXAIRECOVERY_H_
#define _FTREMIXAIRECOVERY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Recovery hooks of the Remix AI (AI.asm recovery_fix_). */

/** recovery_fix_: TRUE when the CPU may repeat its up-B recovery, skipping the vanilla "already used" check. */
sb32 ftRemixAiRecoveryAllowsUpBRepeat(FTStruct *fp);

#endif

#endif
