#ifndef _FTREMIXAIRECOVERYDRIFT_H_
#define _FTREMIXAIRECOVERYDRIFT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Level 10 recovery drift of the Remix AI (AI.asm _improve_recovery_drift). */

/**
 * _improve_recovery_drift: right before the walk step compares its Y with the target, a level 10 CPU that is in special fall,
 * has used all its jumps or is recovering with no floor below refreshes its two ledge positions from the floor lines, picks
 * the ledge it faces or is closest to, and aims the drift at it (outside the ledge, or at its grab box).
 */
void ftRemixAiRecoveryDrift(FTStruct *fp);

#endif

#endif
