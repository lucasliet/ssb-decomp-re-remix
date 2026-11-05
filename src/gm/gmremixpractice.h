#ifndef _GMREMIXPRACTICE_H_
#define _GMREMIXPRACTICE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Stage a 1P Game run starts on under the Remix Practice_1P panel option (Practice_1P.asm _1p_practice). `stage` is the
 * stage the run would start on without the option (nSC1PGameStage), the returned stage replaces it for the human `player`.
 */
extern s32 gmRemixPracticeStartStage(s32 player, s32 stage);

/* Practice state of a 1P run (Practice_1P.asm practice_active) and the L retry; see gmremixpractice.c. */
extern void gmRemixPracticeBegin(s32 player);
extern sb32 gmRemixPracticeIsActive(void);
extern sb32 gmRemixPracticeTryRetryByHold(void);
extern sb32 gmRemixPracticeTryRetryByPause(u16 button_tap);
extern sb32 gmRemixPracticeConsumeRetry(void);
extern sb32 gmRemixPracticeConsumeIntroSkip(void);

#endif

#endif
