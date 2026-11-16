/**
 * sc1ptrainingremixpause.c - the Training pause any controller may open and close, see sc1ptrainingremixpause.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scene.h>
#include <sys/controller.h>
#include "sc/sc1pmode/sc1ptrainingportrules.h"
#include "sc/sc1pmode/sc1ptrainingremixpause.h"

#define SC1PTRAININGREMIXPAUSE_NO_OWNER (-1)

/* Training.asm owner_player_port: the owner while another port has the menu, else -1. */
static s32 sSC1PTrainingRemixPauseOwner = SC1PTRAININGREMIXPAUSE_NO_OWNER;

s32 sc1PTrainingRemixPauseFindPort(u16 buttons)
{
    uint16_t taps[SC1PTRAININGPORTRULES_PORT_COUNT];
    s32 port;

    for (port = 0; port < SC1PTRAININGPORTRULES_PORT_COUNT; port++)
    {
        taps[port] = gSYControllerDevices[port].button_tap;
    }
    return sc1PTrainingPortRulesFindPressingPort(taps, gSCManagerSceneData.player, buttons);
}

void sc1PTrainingRemixPauseTakeMenu(s32 port)
{
    if (port == gSCManagerSceneData.player)
    {
        return;
    }
    sSC1PTrainingRemixPauseOwner = gSCManagerSceneData.player;
    gSCManagerSceneData.player = port;
}

s32 sc1PTrainingRemixPauseReturnMenu(void)
{
    if (sSC1PTrainingRemixPauseOwner != SC1PTRAININGREMIXPAUSE_NO_OWNER)
    {
        gSCManagerSceneData.player = sSC1PTrainingRemixPauseOwner;
        sSC1PTrainingRemixPauseOwner = SC1PTRAININGREMIXPAUSE_NO_OWNER;
    }
    return gSCManagerSceneData.player;
}

s32 sc1PTrainingRemixPauseGetOwner(void)
{
    return (sSC1PTrainingRemixPauseOwner != SC1PTRAININGREMIXPAUSE_NO_OWNER) ? sSC1PTrainingRemixPauseOwner : gSCManagerSceneData.player;
}

GObj* sc1PTrainingRemixPauseGetFighter(void)
{
    SCPlayerData *menu_player = &gSCManagerBattleState->players[gSCManagerSceneData.player];

    if ((menu_player->pkind != nFTPlayerKindNot) && (menu_player->fighter_gobj != NULL))
    {
        return menu_player->fighter_gobj;
    }
    return gSCManagerBattleState->players[sc1PTrainingRemixPauseGetOwner()].fighter_gobj;
}

#endif
