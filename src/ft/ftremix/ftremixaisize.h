#ifndef _FTREMIXAISIZE_H_
#define _FTREMIXAISIZE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Size hook of the Remix AI (AI.asm size_multiplier_check_). */

/** size_multiplier_check_: detect range factor of the CPU body size, 1.0F for giant DK (its own 1.4F factor applies). */
f32 ftRemixAiSizeDetectScale(FTStruct *fp);

#endif

#endif
