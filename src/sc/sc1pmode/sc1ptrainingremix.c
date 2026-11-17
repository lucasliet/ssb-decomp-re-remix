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
#include "gm/gmremixmode.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "remix/remix_toggles.h"
#include "sc/sc1pmode/sc1ptrainingmode.h"
#include "sc/sc1pmode/sc1ptrainingremix.h"
#include "sc/sc1pmode/sc1ptrainingremixaction.h"
#include "sc/sc1pmode/sc1ptrainingremixexit.h"
#include "sc/sc1pmode/sc1ptrainingremixlegend.h"
#include "sc/sc1pmode/sc1ptrainingremixmenu.h"
#include "sc/sc1pmode/sc1ptrainingremixpause.h"
#include "sc/sc1pmode/sc1ptrainingremixspawn.h"
#include "sc/sc1pmode/sc1ptrainingframerules.h"
#include "sc/sc1pmode/sc1ptrainingportrules.h"
#include <if/ifdef.h>
#include "ft/ftremix/ftremixtraining.h"
#include "remix/remix_roster_data.h"
#include "ft/ftremix/sonic/ftremixsonicclassic.h"
#include "ft/ftremix/sonic/ftremixsonicclassicrules.h"
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

/* Training.asm advance_frame_ freeze, du_pressed and dr_pressed */
static SC1PTrainingFrameState sSC1PTrainingRemixFrame;
static s32 sSC1PTrainingRemixResetCount;
static u32 sSC1PTrainingRemixPlayedBGM;

/* Training.asm special_model_display: the Special Model Display value on entry from the stage select, -1 once restored */
static s32 sSC1PTrainingRemixSavedModelDisplay = -1;
static sb32 sSC1PTrainingRemixSavedModelIsRuntime;

/** Remembers the Special Model Display value on entry from the stage select (Training.asm load_from_sss_). */
static void sc1PTrainingRemixSaveModelDisplay(void)
{
    s32 runtime_value;

    sSC1PTrainingRemixSavedModelDisplay = gmRemixToggle(special_model);
    sSC1PTrainingRemixSavedModelIsRuntime = remix_toggle_get_runtime(REMIX_SYM_Toggles__entry_special_model, &runtime_value);
}

void sc1PTrainingRemixRestoreModelDisplay(void)
{
    if (sSC1PTrainingRemixSavedModelDisplay < 0)
    {
        return;
    }
    if (sSC1PTrainingRemixSavedModelIsRuntime != FALSE)
    {
        remix_toggle_set_runtime(REMIX_SYM_Toggles__entry_special_model, sSC1PTrainingRemixSavedModelDisplay);
    }
    else
    {
        remix_toggle_clear_runtime(REMIX_SYM_Toggles__entry_special_model);
    }
    sSC1PTrainingRemixSavedModelDisplay = -1;
}

void sc1PTrainingRemixStartScene(sb32 is_reset)
{
    if (is_reset != FALSE)
    {
        /* A reset runs scManagerFuncUpdate again without scManagerRunLoop, the only caller of gmRemixModeSceneClear: the layer
         * routes, the sprite file table and the Input Display blocks would still point into the freed scene heap. */
        gmRemixModeSceneClear();
    }
    sc1PTrainingFrameRulesInit(&sSC1PTrainingRemixFrame);
    sc1PTrainingRemixMenuStartScene();
    sc1PTrainingRemixActionStartScene();
    sc1PTrainingRemixExitStartScene();
    ftRemixTrainingStartScene();

    if (is_reset == FALSE)
    {
        sSC1PTrainingRemixResetCount = 0;
        sc1PTrainingRemixSaveModelDisplay();
        sc1PTrainingRemixSpawnResetAll();
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

/* Training.asm _check_dr reads D-pad right as TURBO (the press frame and its repeats), so a held DR keeps advancing frames. */
static sb32 sc1PTrainingRemixIsDpadRightRepeated(void)
{
    u16 repeats = 0;
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        repeats |= gSYControllerDevices[port].button_update;
    }
    return ((repeats & R_JPAD) != 0) ? TRUE : FALSE;
}

/** D-pad down: next Special Model Display value, for this run only (Training.asm _check_dd writes the RAM entry). */
static void sc1PTrainingRemixCycleModelDisplay(void)
{
    s32 value = (gmRemixToggle(special_model) + 1) % SC1PTRAININGREMIX_MODEL_DISPLAY_COUNT;

    remix_toggle_set_runtime(REMIX_SYM_Toggles__entry_special_model, value);
}

sb32 sc1PTrainingRemixPollDpad(void)
{
    u16 taps = sc1PTrainingRemixGetDpadTaps();
    s32 actions = sc1PTrainingFrameRulesReadDpad(&sSC1PTrainingRemixFrame, remix_training_get(REMIX_TRAINING_DPAD), taps & L_JPAD,
                                                 sc1PTrainingRemixIsDpadRightRepeated(), taps & U_JPAD, taps & D_JPAD);

    if (actions & SC1PTRAININGFRAMERULES_ACTION_MODEL_DISPLAY)
    {
        sc1PTrainingRemixCycleModelDisplay();
    }
    return (actions & SC1PTRAININGFRAMERULES_ACTION_RESET) ? TRUE : FALSE;
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

/** Makes `bgm` the stage track and plays it, unless a hammer or a star is playing its own theme. */
static void sc1PTrainingRemixApplyMusic(u32 bgm)
{
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

void sc1PTrainingRemixPollMusic(void)
{
    u32 bgm = sc1PTrainingRemixGetBGM();

    if (bgm == sSC1PTrainingRemixPlayedBGM)
    {
        return;
    }
    sc1PTrainingRemixApplyMusic(bgm);
}

void sc1PTrainingRemixPlayMusic(void)
{
    sc1PTrainingRemixApplyMusic(sc1PTrainingRemixGetBGM());
}

static s32 sc1PTrainingRemixGetPortFkind(s32 port, s32 menu_port)
{
    s32 select_fkind = (port == menu_port) ? gSCManagerSceneData.training_man_fkind : gSCManagerSceneData.training_com_fkind;
    s32 chosen = remix_training_character_fkind(remix_training_get(REMIX_TRAINING_CHARACTER(port)));

    return (chosen >= 0) ? chosen : select_fkind;
}

s32 sc1PTrainingRemixGetPortFighter(s32 port)
{
    return sc1PTrainingRemixGetPortFkind(port, sc1PTrainingRemixPauseGetOwner());
}

/**
 * Training.asm load_character_: a Sonic costume from 6 up is Classic Sonic, stored as the modern id plus 6. Only an edited costume
 * item decides it; Auto keeps the Classic flag the character select left.
 */
static s32 sc1PTrainingRemixApplyClassicSonic(s32 port, s32 fkind, s32 costume)
{
    s32 is_classic;

    if ((fkind != REMIX_ID_SONIC) || (remix_training_get(REMIX_TRAINING_COSTUME(port)) == 0))
    {
        return costume;
    }
    costume = ftRemixSonicClassicRulesSplitTrainingCostume(costume, &is_classic);
    ftRemixSonicSetClassic(port, is_classic);

    return costume;
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
    state->players[port].costume = sc1PTrainingRemixApplyClassicSonic(port, state->players[port].fkind, state->players[port].costume);

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

s32 sc1PTrainingRemixResolveFrame(sb32 is_lag_skip)
{
    return sc1PTrainingFrameRulesResolve(&sSC1PTrainingRemixFrame, is_lag_skip);
}

s32 sc1PTrainingRemixGetItemKind(s32 entry)
{
    return ITREMIX_KIND_START + entry;
}

void sc1PTrainingRemixDrawHud(void)
{
    sc1PTrainingRemixLegendUpdate();
    sc1PTrainingRemixMenuUpdate();
    sc1PTrainingRemixActionUpdate();
    sc1PTrainingRemixExitUpdate(sc1PTrainingModeIsCursorOnExit());

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
