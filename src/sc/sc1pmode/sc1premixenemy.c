/**
 * sc1premixenemy.c - Single Player Enemy option of the 1P game, see sc1premixenemy.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/mpdef.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include <sys/audio.h>
#include <sys/controller.h>
#include "sc/sc1pmode/sc1premixenemy.h"
#include "sc/sc1pmode/sc1premixenemyafk.h"
#include "sc/sc1pmode/sc1premixenemyrules.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/scsubsys/scsubsys.h"

extern void *func_800269C0_275C0(u16 id);

/* FGM "Master Hand Laugh" played by the first choice of the enemy port (z_button) */
#define SC1PREMIX_ENEMY_LAUGH_FGM 0x1ED

static SC1PRemixEnemySelection sSC1PRemixEnemySelection;

s32 sc1PRemixEnemyPortGet(void)
{
    return sSC1PRemixEnemySelection.enemy_port;
}

void sc1PRemixEnemyClear(void)
{
    sSC1PRemixEnemySelection.enemy_port = 0;
    sSC1PRemixEnemySelection.is_selected = 0;
}

void sc1PRemixEnemyOnSelectZ(s32 man_port)
{
    s32 presser = scSubsysControllerGetPlayerTapButtons(Z_TRIG);

    if (sc1PRemixEnemySelectOnZ(&sSC1PRemixEnemySelection, presser, man_port) != 0)
    {
        func_800269C0_275C0(SC1PREMIX_ENEMY_LAUGH_FGM);
    }
}

void sc1PRemixEnemyOnSelectB(s32 player)
{
    u32 b_tap_mask = 0;
    s32 port;

    for (port = 0; port < SC1PREMIX_ENEMY_PORT_COUNT; port++)
    {
        if (gSYControllerDevices[port].button_tap & B_BUTTON)
        {
            b_tap_mask |= 1U << port;
        }
    }
    if (sc1PRemixEnemyIsClearedByB(player, b_tap_mask) != 0)
    {
        sc1PRemixEnemyClear();
    }
}

static sb32 sc1PRemixEnemyIsFirstEnemy(s32 mapobj_kind)
{
    return (mapobj_kind == nMPMapObjKind1PGameEnemy1) ? TRUE : FALSE;
}

static s32 sc1PRemixEnemyGetSetupPort(s32 mapobj_kind)
{
    return sc1PRemixEnemyControlPort(sSC1PRemixEnemySelection.enemy_port, sc1PRemixModeGet(), sc1PRemixEnemyIsFirstEnemy(mapobj_kind));
}

void sc1PRemixEnemySetupSlot(s32 slot, s32 mapobj_kind)
{
    s32 port;

    sc1PRemixAfkSetupSlot(slot, mapobj_kind);

    port = sc1PRemixEnemyGetSetupPort(mapobj_kind);

    if (port < 0)
    {
        return;
    }
    gSCManagerBattleState->players[slot].pkind = nFTPlayerKindMan;
    gSCManagerBattleState->players[slot].color = port;
    gSCManagerBattleState->players[slot].tag = port;
}

SYController *sc1PRemixEnemySetupController(s32 mapobj_kind, SYController *controller)
{
    s32 port = sc1PRemixEnemyGetSetupPort(mapobj_kind);

    return (port < 0) ? controller : &gSYControllerDevices[port];
}

SYController *sc1PRemixEnemyRespawnController(s32 mapobj_kind, SYController *controller)
{
    s32 port = sc1PRemixEnemyRespawnPort(sSC1PRemixEnemySelection.enemy_port, sc1PRemixEnemyIsFirstEnemy(mapobj_kind));

    return (port < 0) ? controller : &gSYControllerDevices[port];
}

s32 sc1PRemixEnemyShieldPlayer(FTStruct *fp)
{
    s32 controller_port = -1;

    if (fp->input.controller != NULL)
    {
        controller_port = (s32)(fp->input.controller - gSYControllerDevices);
    }
    return sc1PRemixEnemyShieldPort((gSCManagerSceneData.scene_curr == nSCKind1PGame) ? 1 : 0, sSC1PRemixEnemySelection.enemy_port,
                                    (fp->pkind == nFTPlayerKindMan) ? 1 : 0, controller_port, fp->player);
}

#endif
