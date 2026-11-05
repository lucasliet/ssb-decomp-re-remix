/**
 * sc1ptrainingremixsbm.c - Training gate of Single Button Mode, see sc1ptrainingremixsbm.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/scene.h>
#include "ft/ftremix/ftremixhooks.h"
#include "sc/sc1pmode/sc1ptrainingremixmenu.h"
#include "sc/sc1pmode/sc1ptrainingremixsbm.h"
#include "sc/sc1pmode/sc1ptrainingsbmrules.h"

s32 sc1PTrainingRemixGetSingleButtonGate(void)
{
    s32 toggle_menu;

    if (gSCManagerSceneData.scene_curr != nSCKind1PTrainingMode)
    {
        return FTREMIX_SBM_GATE_OPEN;
    }
    toggle_menu = sc1PTrainingSbmGetToggleMenu(gSCManagerBattleState->game_status, (sc1PTrainingRemixMenuIsOpen() != FALSE) ? 1 : 0);

    return (sc1PTrainingSbmIsMenuUp(toggle_menu) != 0) ? FTREMIX_SBM_GATE_TRAINING_MENU : FTREMIX_SBM_GATE_OPEN;
}

#endif
