#ifndef _FTREMIXAIPOSTEDGE_H_
#define _FTREMIXAIPOSTEDGE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Off-stage safety entries of the Remix AI post process (Character.cpu_post_process table). Pure rules live in ftremixaipostrules.c. */

/** Falco (Falco/AI/Attacks.asm): the Fire Bird dives to the ground, an NSP that would end off stage is cancelled. */
void ftRemixAiPostProcessFalco(FTStruct *fp);

/** Wario (Wario/AI/Attacks.asm): a Wario NSP towards that would end off stage is cancelled. */
void ftRemixAiPostProcessWario(FTStruct *fp);

/** Wolf (Wolf/AI/Attacks.asm): the Wolf Flash steers by the target height, an NSP that would end off stage is cancelled. */
void ftRemixAiPostProcessWolf(FTStruct *fp);

/** DK and J DK (dkshared.asm): the hand slap is released, a Spinning Kong that drifts off stage is turned back, level 10 only. */
void ftRemixAiPostProcessDonkeyKong(FTStruct *fp);

#endif

#endif
