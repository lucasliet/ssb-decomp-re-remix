#ifndef _FTREMIXAIDI_H_
#define _FTREMIXAIDI_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fttypes.h>
#include <ssb_types.h>

/*
 * Remix CPU DI (AI.asm cpu_di_): a CPU in hitlag after a hit applies a smash or slide DI. The decision is in
 * ftremixaidirules.c; Training uses the menu entries, level 10 and the advanced AI roll their own.
 */

/**
 * Sets the CPU script of the DI for one CPU fighter when it is due (AI.asm hook 0x8013A3C8, before the objective switch).
 *
 * @return TRUE when the script was set and the normal objective must be skipped.
 */
extern sb32 ftRemixAiCpuDiSetObjective(FTStruct *fp);

#endif

#endif
