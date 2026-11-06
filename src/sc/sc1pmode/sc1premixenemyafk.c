/**
 * sc1premixenemyafk.c - "afk" player tag automation, see sc1premixenemyafk.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mp/mpdef.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include "remix/remix_player_panel.h"
#include "sc/sc1pmode/sc1premixenemyafk.h"
#include "sc/sc1pmode/sc1premixenemyafkrules.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/scsubsys/scsubsys.h"
#include <sys/audio.h>
#include <sys/controller.h>

static u32 sSC1PRemixAfkSoundTestTimer;
static sb32 sSC1PRemixAfkCreditsIsManualTap;

sb32 sc1PRemixAfkIsActive(void)
{
    return (sc1PRemixAfkIsTagName(remix_panel_tag_name(SC1PREMIX_AFK_TAG_NUMBER)) != 0) ? TRUE : FALSE;
}

sb32 sc1PRemixAfkTapButtons(u32 buttons)
{
    if (sc1PRemixAfkIsActive() != FALSE)
    {
        return TRUE;
    }
    return scSubsysControllerGetPlayerTapButtons(buttons);
}

sb32 sc1PRemixAfkForceTrue(sb32 condition)
{
    if (sc1PRemixAfkIsActive() != FALSE)
    {
        return TRUE;
    }
    return condition;
}

void sc1PRemixAfkSetupSlot(s32 slot, s32 mapobj_kind)
{
    s32 is_player_spawn = (mapobj_kind == nMPMapObjKind1PGamePlayer) ? 1 : 0;
    s32 is_contest = (sc1PRemixModeGet() == SC1PREMIX_MODE_HRC) ? 1 : 0;

    if (sc1PRemixAfkMakesPlayerBot(sc1PRemixAfkIsActive(), is_player_spawn, is_contest) == 0)
    {
        return;
    }
    gSCManagerBattleState->players[slot].pkind = nFTPlayerKindCom;
    gSCManagerBattleState->players[slot].level = SC1PREMIX_AFK_BOT_LEVEL;
    port_log("remix afk: slot %d is a level %d CPU\n", (int)slot, SC1PREMIX_AFK_BOT_LEVEL);
}

sb32 sc1PRemixAfkIsSpeedAssistActive(s32 name_id, f32 speed, f32 normal_speed)
{
    sb32 is_active = (sc1PRemixAfkSpeedAssistFires(sc1PRemixAfkIsActive(), name_id, speed, normal_speed) != 0) ? TRUE : FALSE;

    if (is_active != FALSE)
    {
        port_log("remix afk: credits speed assist at name %d\n", (int)name_id);
    }
    return is_active;
}

sb32 sc1PRemixAfkSoundTestTapA(void)
{
    sb32 is_a_tapped = scSubsysControllerGetPlayerTapButtons(A_BUTTON);
    sb32 has_song;
    sb32 fires_a;

    if (sc1PRemixAfkIsActive() == FALSE)
    {
        return is_a_tapped;
    }
    has_song = (syAudioGetBGMPlaying(0) >= 0) ? TRUE : FALSE;
    fires_a = (sc1PRemixAfkSoundTestFiresA(sSC1PRemixAfkSoundTestTimer, is_a_tapped, has_song) != 0) ? TRUE : FALSE;
    sSC1PRemixAfkSoundTestTimer = sc1PRemixAfkSoundTestNextTimer(sSC1PRemixAfkSoundTestTimer, is_a_tapped, has_song);

    return (is_a_tapped != FALSE) || (fires_a != FALSE);
}

sb32 sc1PRemixAfkSoundTestTapRight(void)
{
    return (sc1PRemixAfkSoundTestRightIsDue(sc1PRemixAfkIsActive(), sSC1PRemixAfkSoundTestTimer) != 0) ? TRUE : FALSE;
}

sb32 sc1PRemixAfkCreditsTapShoot(u16 button_tap)
{
    sb32 is_manual_tap = ((button_tap & (A_BUTTON | B_BUTTON)) != 0) ? TRUE : FALSE;

    sSC1PRemixAfkCreditsIsManualTap = is_manual_tap;
    return (sc1PRemixAfkCreditsShootPressed(sc1PRemixAfkIsActive(), is_manual_tap) != 0) ? TRUE : FALSE;
}

sb32 sc1PRemixAfkCreditsShotIsSilent(void)
{
    return (sc1PRemixAfkCreditsShootIsSilent(sc1PRemixAfkIsActive(), sSC1PRemixAfkCreditsIsManualTap) != 0) ? TRUE : FALSE;
}

#endif
