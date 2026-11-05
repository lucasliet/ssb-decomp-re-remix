#ifndef _FTREMIXAISTAGE_H_
#define _FTREMIXAISTAGE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/* Stage hooks of the Remix AI (Hazards.asm planet_zebes_acid_ _ai_fix). */

/** TRUE when the current stage is Zebes based and its acid exists (hazards on), so the CPU avoids the acid. */
sb32 ftRemixAiZebesAcidApplies(void);

#endif

#endif
