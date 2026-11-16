/*
 * ftremixaistage.c - stage hook of the Remix AI (.toolchain/pkgs/smashremix-src/src/Hazards.asm planet_zebes_acid_ _ai_fix, ai_extend_hyrule_tornado_check, mk_remix_hardcoding_9).
 * Pure rules live in ftremixaistagerules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/map.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gr/grremix/grremixgating.h"
#include "ft/ftremix/ftremixaistage.h"
#include "ft/ftremix/ftremixaistagerules.h"

sb32 ftRemixAiZebesAcidApplies(void)
{
    return ftRemixAiStageHasZebesAcid(gSCManagerBattleState->gkind, grRemixHazardToggleIsOn());
}


sb32 ftRemixAiHyruleTornadoApplies(void)
{
    return ftRemixAiStageHasHyruleTornado(gSCManagerBattleState->gkind);
}

sb32 ftRemixAiDroppingPlatformApplies(FTStruct *fp)
{
    if (fp->coll_data.floor_line_id < 0)
    {
        return FALSE;
    }
    if (mpCollisionCheckExistPlatformLineID(fp->coll_data.floor_line_id) == FALSE)
    {
        return FALSE;
    }
    return ftRemixAiStageFleesDroppingPlatform
    (
        gSCManagerBattleState->gkind,
        fp->joints[nFTPartsJointTopN]->translate.vec.f.y
    );
}

#endif
