/**
 * sc1ptrainingremix.c - D-pad controls and reset count of Training, see sc1ptrainingremix.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/controller.h>
#include <sys/audio.h>
#include <gm/gmsound.h>
#include <mp/map.h>
#include <port_log.h>
#include "it/itremix/itremix.h"
#include <sc/scene.h>
#include "gm/gmremixtoggle.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "remix/remix_toggles.h"
#include "sc/sc1pmode/sc1ptrainingremix.h"
#include "sc/sc1pmode/sc1ptrainingportrules.h"
#include <if/ifdef.h>
#include "ft/ftremix/ftremixtraining.h"
#include "remix/remix_training.h"

#define SC1PTRAININGREMIX_DPAD_BUTTONS (U_JPAD | D_JPAD | L_JPAD | R_JPAD)
#define SC1PTRAININGREMIX_MODEL_DISPLAY_COUNT 4
#define SC1PTRAININGREMIX_HUD_SLOT 27
#define SC1PTRAININGREMIX_HUD_VALUE_SLOT 25
#define SC1PTRAININGREMIX_HUD_LABEL_X 99.5F
#define SC1PTRAININGREMIX_HUD_VALUE_X 221.0F
#define SC1PTRAININGREMIX_HUD_Y 25.0F
#define SC1PTRAININGREMIX_HUD_SCALE 1.0F
#define SC1PTRAININGREMIX_HUD_COLOR 0xFFFFFFFF

static sb32 sSC1PTrainingRemixIsFrozen;
static sb32 sSC1PTrainingRemixIsAdvancing;
static s32 sSC1PTrainingRemixResetCount;
static u32 sSC1PTrainingRemixPlayedBGM;

void sc1PTrainingRemixStartScene(sb32 is_reset)
{
    sSC1PTrainingRemixIsFrozen = FALSE;
    sSC1PTrainingRemixIsAdvancing = FALSE;
    ftRemixTrainingStartScene();

    if (is_reset == FALSE)
    {
        sSC1PTrainingRemixResetCount = 0;
    }
}

void sc1PTrainingRemixOnReset(void)
{
    sSC1PTrainingRemixResetCount++;
}

static u16 sc1PTrainingRemixGetDpadTaps(void)
{
    u16 taps = 0;
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        taps |= gSYControllerDevices[port].button_tap;
    }
    return taps & SC1PTRAININGREMIX_DPAD_BUTTONS;
}

static void sc1PTrainingRemixCycleModelDisplay(void)
{
    s32 value = (gmRemixToggle(special_model) + 1) % SC1PTRAININGREMIX_MODEL_DISPLAY_COUNT;

    remix_toggle_set(REMIX_SYM_Toggles__entry_special_model, value);
}

sb32 sc1PTrainingRemixPollDpad(void)
{
    u16 taps = sc1PTrainingRemixGetDpadTaps();

    if (gSCManagerBattleState->game_status != nSCBattleGameStatusGo)
    {
        return FALSE;
    }
    if (remix_training_get(REMIX_TRAINING_DPAD) == REMIX_TRAINING_DPAD_OFF)
    {
        return FALSE;
    }
    if (taps & L_JPAD)
    {
        return TRUE;
    }
    if (remix_training_get(REMIX_TRAINING_DPAD) == REMIX_TRAINING_DPAD_RESET_ONLY)
    {
        return FALSE;
    }
    if (taps & U_JPAD)
    {
        sSC1PTrainingRemixIsFrozen = (sSC1PTrainingRemixIsFrozen == FALSE) ? TRUE : FALSE;
    }
    if (taps & R_JPAD)
    {
        sSC1PTrainingRemixIsFrozen = TRUE;
        sSC1PTrainingRemixIsAdvancing = TRUE;
    }
    if (taps & D_JPAD)
    {
        sc1PTrainingRemixCycleModelDisplay();
    }
    return FALSE;
}

static u32 sc1PTrainingRemixGetBGM(void)
{
    u32 table_address = REMIX_SYM_Training__bgm_table + (remix_training_get(REMIX_TRAINING_MUSIC) * 2);

    return remix_ram_image_read_half(table_address);
}

void sc1PTrainingRemixStartMusic(void)
{
    u32 bgm = sc1PTrainingRemixGetBGM();

    port_log("SSB64: training music: start option %d bgm %d\n", (int)remix_training_get(REMIX_TRAINING_MUSIC), (int)bgm);
    sSC1PTrainingRemixPlayedBGM = bgm;
    gMPCollisionBGMDefault = bgm;
    syAudioPlayBGM(0, bgm);
    gMPCollisionBGMCurrent = bgm;
}

void sc1PTrainingRemixPollMusic(void)
{
    u32 bgm = sc1PTrainingRemixGetBGM();

    if (bgm == sSC1PTrainingRemixPlayedBGM)
    {
        return;
    }
    sSC1PTrainingRemixPlayedBGM = bgm;
    gMPCollisionBGMDefault = bgm;

    if ((gMPCollisionBGMCurrent == nSYAudioBGMHammer) || (gMPCollisionBGMCurrent == nSYAudioBGMStar))
    {
        return;
    }
    port_log("SSB64: training music: option %d bgm %d\n", (int)remix_training_get(REMIX_TRAINING_MUSIC), (int)bgm);
    syAudioPlayBGM(0, bgm);
    gMPCollisionBGMCurrent = bgm;
}

static s32 sc1PTrainingRemixGetPortFkind(s32 port, s32 menu_port)
{
    s32 select_fkind = (port == menu_port) ? gSCManagerSceneData.training_man_fkind : gSCManagerSceneData.training_com_fkind;
    s32 chosen = remix_training_character_fkind(remix_training_get(REMIX_TRAINING_CHARACTER(port)));

    return (chosen >= 0) ? chosen : select_fkind;
}

static void sc1PTrainingRemixSetupPort(s32 port, s32 menu_port, s32 type)
{
    SCBattleState *state = gSCManagerBattleState;
    s32 select_costume = (port == menu_port) ? gSCManagerSceneData.training_man_costume : gSCManagerSceneData.training_com_costume;

    if (type == SC1PTRAININGPORTRULES_TYPE_DISABLED)
    {
        state->players[port].pkind = nFTPlayerKindNot;
        return;
    }
    state->players[port].fkind = sc1PTrainingRemixGetPortFkind(port, menu_port);
    state->players[port].costume = sc1PTrainingPortRulesPick(remix_training_get(REMIX_TRAINING_COSTUME(port)), select_costume);

    if (type == SC1PTRAININGPORTRULES_TYPE_HUMAN)
    {
        state->players[port].pkind = nFTPlayerKindMan;
        state->players[port].team = 0;
        state->players[port].color = port;
        return;
    }
    state->players[port].pkind = nFTPlayerKindCom;
    state->players[port].tag = nIFPlayerTagKindCP;
    state->players[port].level = 3;
    state->players[port].team = 1;
    state->players[port].color = 4;
}

s32 sc1PTrainingRemixSetupPorts(s32 menu_port, s32 select_cpu_port)
{
    s32 types[SC1PTRAININGPORTRULES_PORT_COUNT];
    s32 port;

    for (port = 0; port < SC1PTRAININGPORTRULES_PORT_COUNT; port++)
    {
        types[port] = sc1PTrainingPortRulesResolveType(remix_training_get(REMIX_TRAINING_TYPE(port)), port, menu_port, select_cpu_port);
        sc1PTrainingRemixSetupPort(port, menu_port, types[port]);
        port_log("SSB64: training port: %d type %d fkind %d costume %d spawn %d\n", (int)port, (int)types[port],
                 (int)gSCManagerBattleState->players[port].fkind, (int)gSCManagerBattleState->players[port].costume,
                 (int)sc1PTrainingRemixSpawnPort(port));
    }
    gSCManagerBattleState->pl_count = sc1PTrainingPortRulesCountType(types, SC1PTRAININGPORTRULES_TYPE_HUMAN);
    gSCManagerBattleState->cp_count = sc1PTrainingPortRulesCountType(types, SC1PTRAININGPORTRULES_TYPE_CPU);

    return sc1PTrainingPortRulesPickDummy(types, menu_port);
}

s32 sc1PTrainingRemixSpawnPort(s32 port)
{
    return sc1PTrainingPortRulesSpawnPort(remix_training_get(REMIX_TRAINING_SPAWN(port)), port);
}

sb32 sc1PTrainingRemixIsFrameHeld(void)
{
    if (sSC1PTrainingRemixIsFrozen == FALSE)
    {
        return FALSE;
    }
    if (sSC1PTrainingRemixIsAdvancing != FALSE)
    {
        sSC1PTrainingRemixIsAdvancing = FALSE;
        return FALSE;
    }
    return TRUE;
}

s32 sc1PTrainingRemixGetItemKind(s32 entry)
{
    return ITREMIX_KIND_START + entry;
}

void sc1PTrainingRemixDrawHud(void)
{
    if (gSCManagerBattleState->game_status != nSCBattleGameStatusPause)
    {
        return;
    }
    remix_hud_text_put(SC1PTRAININGREMIX_HUD_SLOT, "Reset Count:", SC1PTRAININGREMIX_HUD_LABEL_X, SC1PTRAININGREMIX_HUD_Y,
                       SC1PTRAININGREMIX_HUD_COLOR, SC1PTRAININGREMIX_HUD_SCALE, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_putf(SC1PTRAININGREMIX_HUD_VALUE_SLOT, SC1PTRAININGREMIX_HUD_VALUE_X, SC1PTRAININGREMIX_HUD_Y,
                        SC1PTRAININGREMIX_HUD_COLOR, SC1PTRAININGREMIX_HUD_SCALE, REMIX_HUD_ALIGN_RIGHT, "%d",
                        (int)sSC1PTrainingRemixResetCount);
}

#endif
