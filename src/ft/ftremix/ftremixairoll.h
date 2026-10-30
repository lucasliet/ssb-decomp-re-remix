#ifndef _FTREMIXAIROLL_H_
#define _FTREMIXAIROLL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Level 10 routines of the Remix AI (AI.asm fix_multi_doublejump_usage, repeated input, shield_drop_). */

/** fix_multi_doublejump_usage: TRUE when a CPU below the ledge may try another jump. */
sb32 ftRemixAiCanJumpWhileFalling(FTStruct *fp);

/** skip_repeated_input_check: TRUE when level 10 skips the recently-used-input range adjustment. */
sb32 ftRemixAiSkipRepeatedInputCheck(FTStruct *fp);

/** skip_repeated_input_skip: level 10 never reports the four-in-a-row limit, and resets the counter. */
sb32 ftRemixAiInputRepeatLimitHit(FTStruct *fp, FTComputer *com);

/** shield_drop_: the input routine for a platform drop, SHIELD_DROP for an idle level 10 CPU, the vanilla drop otherwise. */
s32 ftRemixAiDownCommand(FTStruct *fp);

#endif

#endif
