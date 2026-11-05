/*
 * ftremixaistage.c - stage hook of the Remix AI (.toolchain/pkgs/smashremix-src/src/Hazards.asm planet_zebes_acid_ _ai_fix).
 * Pure rules live in ftremixaistagerules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gr/grremix/grremixgating.h"
#include "ft/ftremix/ftremixaistage.h"
#include "ft/ftremix/ftremixaistagerules.h"

sb32 ftRemixAiZebesAcidApplies(void)
{
    return ftRemixAiStageHasZebesAcid(gSCManagerBattleState->gkind, grRemixHazardToggleIsOn());
}

#endif
