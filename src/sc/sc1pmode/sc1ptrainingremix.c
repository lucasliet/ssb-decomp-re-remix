/**
 * sc1ptrainingremix.c - D-pad controls and reset count of Training, see sc1ptrainingremix.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/controller.h>
#include <string.h>
#include "it/itremix/itremix.h"
#include <sc/scene.h>
#include "gm/gmremixtoggle.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_symbols.h"
#include "remix/remix_toggles.h"
#include "sc/sc1pmode/sc1ptrainingremix.h"

#define SC1PTRAININGREMIX_DPAD_BUTTONS (U_JPAD | D_JPAD | L_JPAD | R_JPAD)
#define SC1PTRAININGREMIX_MODEL_DISPLAY_COUNT 4
#define SC1PTRAININGREMIX_HUD_SLOT 27
#define SC1PTRAININGREMIX_HUD_X 72.0F
#define SC1PTRAININGREMIX_HUD_Y 64.0F
#define SC1PTRAININGREMIX_HUD_HEIGHT 12.0F
#define SC1PTRAININGREMIX_HUD_COLOR 0xFFFFFFFF
#define SC1PTRAININGREMIX_ITEM_SLOT 26
#define SC1PTRAININGREMIX_ITEM_X 191.0F
#define SC1PTRAININGREMIX_ITEM_Y 90.0F
#define SC1PTRAININGREMIX_CHAR_WIDTH 7

static sb32 sSC1PTrainingRemixIsFrozen;
static sb32 sSC1PTrainingRemixIsAdvancing;
static s32 sSC1PTrainingRemixResetCount;

void sc1PTrainingRemixStartScene(sb32 is_reset)
{
    sSC1PTrainingRemixIsFrozen = FALSE;
    sSC1PTrainingRemixIsAdvancing = FALSE;

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
    if (taps & L_JPAD)
    {
        return TRUE;
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

static const char *dSC1PTrainingRemixItemNames[SC1PTRAININGREMIX_ITEM_COUNT] =
{
    "Cloaking Device", "Super Mushroom", "Poison Mushroom", "Blue Shell", "Lightning", "Deku Nut", "Franklin Badge",
    "Pitfall", "Golden Gun", "Dango", "P-Wing", "Mr. Saturn", "Stopwatch"
};

s32 sc1PTrainingRemixGetItemKind(s32 entry)
{
    return ITREMIX_KIND_START + entry;
}

s32 sc1PTrainingRemixDrawItemName(s32 entry)
{
    const char *name = dSC1PTrainingRemixItemNames[((entry >= 0) && (entry < SC1PTRAININGREMIX_ITEM_COUNT)) ? entry : 0];

    remix_hud_text_putf(SC1PTRAININGREMIX_ITEM_SLOT, SC1PTRAININGREMIX_ITEM_X, SC1PTRAININGREMIX_ITEM_Y, SC1PTRAININGREMIX_HUD_COLOR,
                        SC1PTRAININGREMIX_HUD_HEIGHT, "%s", name);

    return (s32)strlen(name) * SC1PTRAININGREMIX_CHAR_WIDTH;
}

void sc1PTrainingRemixDrawHud(void)
{
    if (gSCManagerBattleState->game_status != nSCBattleGameStatusGo)
    {
        return;
    }
    remix_hud_text_putf(SC1PTRAININGREMIX_HUD_SLOT, SC1PTRAININGREMIX_HUD_X, SC1PTRAININGREMIX_HUD_Y, SC1PTRAININGREMIX_HUD_COLOR,
                        SC1PTRAININGREMIX_HUD_HEIGHT, "Reset Count: %d", (int)sSC1PTrainingRemixResetCount);
}

#endif
