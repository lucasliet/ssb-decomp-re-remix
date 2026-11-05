/**
 * grremixsaffron.c - Saffron City Pokemon Rate setting. See grremixsaffron.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtoggle.h"
#include "grremixsaffron.h"
#include "grremixsaffronrules.h"
#include <sc/sctypes.h>
#include <sc/scmanager.h>

/** Toggles.entry_saffron_poke_rate on the VS and Training screens, the default rate elsewhere. */
static s32 grRemixSaffronRate(void)
{
    s32 scene = gSCManagerSceneData.scene_curr;

    if ((scene != nSCKindVSBattle) && (scene != nSCKind1PTrainingMode))
    {
        return GRREMIX_SAFFRON_RATE_DEFAULT;
    }
    return gmRemixToggle(saffron_poke_rate);
}

u16 grRemixSaffronSleepMonsterWait(s32 base_wait)
{
    return grRemixSaffronRateSleepMonsterWait(grRemixSaffronRate(), base_wait);
}

u16 grRemixSaffronClosedMonsterWait(s32 base_wait)
{
    return grRemixSaffronRateClosedMonsterWait(grRemixSaffronRate(), base_wait);
}

u16 grRemixSaffronClosedGateWait(s32 base_wait)
{
    return grRemixSaffronRateClosedGateWait(grRemixSaffronRate(), base_wait);
}

#endif
