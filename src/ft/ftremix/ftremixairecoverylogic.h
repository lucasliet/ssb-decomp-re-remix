#ifndef _FTREMIXAIRECOVERYLOGIC_H_
#define _FTREMIXAIRECOVERYLOGIC_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Character.recovery_logic hook of the Remix AI (AI.asm custom_recovery_logic). */

/**
 * custom_recovery_logic: runs the C port of the routine that the fighter's Character.recovery_logic entry points to, right
 * after the walk step of the Recover objective. Does nothing for a fighter with no entry.
 */
void ftRemixAiRecoveryLogicRunFighter(FTStruct *fp);

#endif

#endif
