#ifndef _FTREMIXAISTAGE_H_
#define _FTREMIXAISTAGE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>

/* Stage hooks of the Remix AI (Hazards.asm planet_zebes_acid_ _ai_fix, ai_extend_hyrule_tornado_check, mk_remix_hardcoding_9). */

/** TRUE when the current stage is Zebes based and its acid exists (hazards on), so the CPU avoids the acid. */
sb32 ftRemixAiZebesAcidApplies(void);

/** TRUE when the current stage runs the Hyrule tornado, so the CPU keeps away from it (Hazards.asm ai_extend_hyrule_tornado_check). */
sb32 ftRemixAiHyruleTornadoApplies(void);

/** TRUE when the CPU stands on a platform line that drops with the stage hazard and is already low enough to recover (Hazards.asm mk_remix_hardcoding_9). */
sb32 ftRemixAiDroppingPlatformApplies(FTStruct *fp);

#endif

#endif
