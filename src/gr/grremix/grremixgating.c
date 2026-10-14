/**
 * grremixgating.c - stage and hazard toggle gates of the vanilla ground makers. See grremixgating.h.
 */

#ifdef PORT

#include "grremixgating.h"
#include "grremixhazards.h"
#include "grremixids.h"
#include <sc/scene.h>
#include <sc/scmanager.h>

/** TRUE when the current screen is VS or Training, the only screens where Remix hazard toggles apply. */
static sb32 grRemixIsToggleScreen(void)
{
    u8 scene = gSCManagerSceneData.scene_curr;

    return (scene == nSCKindVSBattle) || (scene == nSCKind1PTrainingMode);
}

sb32 grRemixHazardToggleIsOn(void)
{
    if (grRemixIsToggleScreen() == FALSE)
    {
        return TRUE;
    }
    return grRemixHazardsAreOn();
}

sb32 grRemixMovementToggleIsOn(void)
{
    if (grRemixIsToggleScreen() == FALSE)
    {
        return TRUE;
    }
    return grRemixMovementIsOn();
}

sb32 grRemixIsMushroomKingdomClone(s32 gkind)
{
    return (gkind == nGRRemixStage_SMBBF) || (gkind == nGRRemixStage_SMBO) || (gkind == nGRRemixStage_MK_REMIX);
}

sb32 grRemixStageHasPeachBumper(s32 gkind)
{
    switch (gkind)
    {
    case nGRRemixStage_PEACHS_CASTLE:
    case nGRRemixStage_PCASTLE_DL:
    case nGRRemixStage_PCASTLE_O:
    case nGRRemixStage_PCASTLE_BETA:
    case nGRRemixStage_LMAO_CASTLE:
        return TRUE;

    default:
        return FALSE;
    }
}

#endif
