/**
 * sc1ptrainingremixmenu.c - the Z menu of the Remix Training, see sc1ptrainingremixmenu.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/mnplayers/mnremixcostumes.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/audio.h>
#include <sys/controller.h>
#include <sys/objtypes.h>
#include "remix/remix_training.h"
#include "sc/sc1pmode/sc1ptrainingmenuentries.h"
#include "sc/sc1pmode/sc1ptrainingmenurules.h"
#include "sc/sc1pmode/sc1ptrainingmode.h"
#include "sc/sc1pmode/sc1ptrainingremix.h"
#include "sc/sc1pmode/sc1ptrainingremixmenu.h"
#include "sc/sc1pmode/sc1ptrainingremixmenudraw.h"
#include "sc/sc1pmode/sc1ptrainingremixspawn.h"

/* FGM.asm: menu.CONFIRM, menu.TOGGLE and menu.SCROLL; run_ plays 0x116 when Z opens the menu. */
#define SC1PTRAININGREMIXMENU_FGM_OPEN 0x116
#define SC1PTRAININGREMIXMENU_FGM_CONFIRM 158
#define SC1PTRAININGREMIXMENU_FGM_TOGGLE 163
#define SC1PTRAININGREMIXMENU_FGM_SCROLL 164

#define SC1PTRAININGREMIXMENU_SPAWN_CUSTOM_VALUE 5

/* Joypad.LEFT, RIGHT, DOWN and UP. */
#define SC1PTRAININGREMIXMENU_DIRECTION_LEFT 0
#define SC1PTRAININGREMIXMENU_DIRECTION_RIGHT 1
#define SC1PTRAININGREMIXMENU_DIRECTION_DOWN 2
#define SC1PTRAININGREMIXMENU_DIRECTION_UP 3
#define SC1PTRAININGREMIXMENU_DIRECTION_COUNT 4

static sb32 sSC1PTrainingRemixMenuIsOpen;
static s32 sSC1PTrainingRemixMenuSelection;
static s32 sSC1PTrainingRemixMenuPort = 1;
static sb32 sSC1PTrainingRemixMenuIsPreserved;
static s32 sSC1PTrainingRemixMenuStickFrames[SC1PTRAININGREMIXMENU_DIRECTION_COUNT];

/* Percent and Music apply on A (their a_function); until then the menu shows the value being edited. */
static s32 sSC1PTrainingRemixMenuPending[SC1PTRAININGMENU_SLOT_COUNT][SC1PTRAININGMENU_PORT_COUNT];

sb32 sc1PTrainingRemixMenuIsOpen(void)
{
    return sSC1PTrainingRemixMenuIsOpen;
}

sb32 sc1PTrainingRemixMenuIsPreserved(void)
{
    return sSC1PTrainingRemixMenuIsPreserved;
}

s32 sc1PTrainingRemixMenuGetSelection(void)
{
    return sSC1PTrainingRemixMenuSelection;
}

/** Port (0 to 3) the Port entry names. */
static s32 sc1PTrainingRemixMenuGetPort(void)
{
    return sSC1PTrainingRemixMenuPort - 1;
}

static s32 sc1PTrainingRemixMenuGetPendingPort(const SC1PTrainingMenuEntry *entry)
{
    return (entry->is_per_port != 0) ? sc1PTrainingRemixMenuGetPort() : 0;
}

/** Copies the registry value of every entry that waits for A, so the menu starts from what the match uses. */
static void sc1PTrainingRemixMenuSyncPending(void)
{
    s32 slot;
    s32 port;

    for (slot = 0; slot < SC1PTRAININGMENU_SLOT_COUNT; slot++)
    {
        const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);

        if (entry->action != SC1PTRAININGMENU_ACTION_COMMIT)
        {
            continue;
        }
        for (port = 0; port < SC1PTRAININGMENU_PORT_COUNT; port++)
        {
            sSC1PTrainingRemixMenuPending[slot][port] = remix_training_get(sc1PTrainingMenuEntriesGetOption(entry, port));
        }
    }
}

s32 sc1PTrainingRemixMenuGetValue(s32 slot)
{
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);
    s32 port = sc1PTrainingRemixMenuGetPort();

    if (slot == SC1PTRAININGMENU_SLOT_PORT)
    {
        return sSC1PTrainingRemixMenuPort;
    }
    if (slot == SC1PTRAININGMENU_SLOT_PRESERVE)
    {
        return sSC1PTrainingRemixMenuIsPreserved;
    }
    if (entry->kind == SC1PTRAININGMENU_KIND_TITLE)
    {
        return 0;
    }
    if ((slot == SC1PTRAININGMENU_SLOT_SPAWN) && (sc1PTrainingRemixSpawnIsCustom(port) != FALSE))
    {
        return SC1PTRAININGREMIXMENU_SPAWN_CUSTOM_VALUE;
    }
    if (entry->action == SC1PTRAININGMENU_ACTION_COMMIT)
    {
        return sSC1PTrainingRemixMenuPending[slot][sc1PTrainingRemixMenuGetPendingPort(entry)];
    }
    return remix_training_get(sc1PTrainingMenuEntriesGetOption(entry, port));
}

/** Highest value of an entry; Costume stops at the costumes the character of the port has (run_ 1818-1864). */
static s32 sc1PTrainingRemixMenuGetMax(s32 slot)
{
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);
    s32 native_max;

    if (slot != SC1PTRAININGMENU_SLOT_COSTUME)
    {
        return entry->max;
    }
    native_max = mnRemixCostumeGetMaxID(sc1PTrainingRemixGetPortFighter(sc1PTrainingRemixMenuGetPort())) + 1;

    return sc1PTrainingMenuRulesClampValue(native_max, entry->max);
}

static void sc1PTrainingRemixMenuSetSpawn(s32 port, s32 value)
{
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_SPAWN);

    if (value == SC1PTRAININGREMIXMENU_SPAWN_CUSTOM_VALUE)
    {
        sc1PTrainingRemixSpawnSelect(port);
        return;
    }
    sc1PTrainingRemixSpawnClear(port);
    remix_training_set(sc1PTrainingMenuEntriesGetOption(entry, port), value);
}

static void sc1PTrainingRemixMenuSetValue(s32 slot, s32 value)
{
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);
    s32 port = sc1PTrainingRemixMenuGetPort();

    if (slot == SC1PTRAININGMENU_SLOT_PORT)
    {
        sSC1PTrainingRemixMenuPort = value;
        return;
    }
    if (slot == SC1PTRAININGMENU_SLOT_PRESERVE)
    {
        sSC1PTrainingRemixMenuIsPreserved = value;
        return;
    }
    if (slot == SC1PTRAININGMENU_SLOT_SPAWN)
    {
        sc1PTrainingRemixMenuSetSpawn(port, value);
        return;
    }
    if (entry->action == SC1PTRAININGMENU_ACTION_COMMIT)
    {
        sSC1PTrainingRemixMenuPending[slot][sc1PTrainingRemixMenuGetPendingPort(entry)] = value;
        return;
    }
    remix_training_set(sc1PTrainingMenuEntriesGetOption(entry, port), value);
}

static sb32 sc1PTrainingRemixMenuIsTapped(u16 buttons)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        if (gSYControllerDevices[port].button_tap & buttons)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** Stick axis a direction is read on (x for left and right, y for down and up). */
static s32 sc1PTrainingRemixMenuGetStickAxis(s32 port, s32 direction)
{
    Vec2b *stick = &gSYControllerDevices[port].stick_range;

    return ((direction == SC1PTRAININGREMIXMENU_DIRECTION_LEFT) || (direction == SC1PTRAININGREMIXMENU_DIRECTION_RIGHT)) ? stick->x
                                                                                                                       : stick->y;
}

/** Left and down push the axis negative, right and up positive. */
static s32 sc1PTrainingRemixMenuGetDirectionSign(s32 direction)
{
    return ((direction == SC1PTRAININGREMIXMENU_DIRECTION_LEFT) || (direction == SC1PTRAININGREMIXMENU_DIRECTION_DOWN)) ? -1 : 1;
}

/** Whether any controller holds the stick past the dead zone in a direction (Joypad.check_stick_x_ and check_stick_y_). */
static sb32 sc1PTrainingRemixMenuIsStickPast(s32 direction)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        s32 pushed = sc1PTrainingRemixMenuGetStickAxis(port, direction) * sc1PTrainingRemixMenuGetDirectionSign(direction);

        if (pushed > SC1PTRAININGMENURULES_STICK_DEADZONE)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** Joypad.update_stick_: counts the frames each stick direction has been held. */
static void sc1PTrainingRemixMenuUpdateStick(void)
{
    s32 direction;

    for (direction = 0; direction < SC1PTRAININGREMIXMENU_DIRECTION_COUNT; direction++)
    {
        if (sc1PTrainingRemixMenuIsStickPast(direction) == FALSE)
        {
            sSC1PTrainingRemixMenuStickFrames[direction] = 0;
            continue;
        }
        sSC1PTrainingRemixMenuStickFrames[direction]++;
    }
}

/** A direction is pressed by the stick (with the repeat) or by its C button; the D-pad is not allowed in this menu (info dpad = FALSE). */
static sb32 sc1PTrainingRemixMenuIsDirectionPressed(s32 direction, u16 c_button)
{
    if (sc1PTrainingMenuRulesIsStickStep(sSC1PTrainingRemixMenuStickFrames[direction]) != 0)
    {
        return TRUE;
    }
    return sc1PTrainingRemixMenuIsTapped(c_button);
}

static sb32 sc1PTrainingRemixMenuHandleVertical(void)
{
    s32 entry_count = SC1PTRAININGMENU_SLOT_COUNT;

    if (sc1PTrainingRemixMenuIsDirectionPressed(SC1PTRAININGREMIXMENU_DIRECTION_DOWN, D_CBUTTONS) != FALSE)
    {
        syAudioPlayFGM(SC1PTRAININGREMIXMENU_FGM_SCROLL);
        sSC1PTrainingRemixMenuSelection = sc1PTrainingMenuRulesMoveDown(sSC1PTrainingRemixMenuSelection, entry_count);
        return TRUE;
    }
    if (sc1PTrainingRemixMenuIsDirectionPressed(SC1PTRAININGREMIXMENU_DIRECTION_UP, U_CBUTTONS) != FALSE)
    {
        syAudioPlayFGM(SC1PTRAININGREMIXMENU_FGM_SCROLL);
        sSC1PTrainingRemixMenuSelection = sc1PTrainingMenuRulesMoveUp(sSC1PTrainingRemixMenuSelection, entry_count);
        return TRUE;
    }
    return FALSE;
}

static void sc1PTrainingRemixMenuStepSelected(s32 direction)
{
    s32 slot = sSC1PTrainingRemixMenuSelection;
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);

    syAudioPlayFGM(SC1PTRAININGREMIXMENU_FGM_TOGGLE);
    sc1PTrainingRemixMenuSetValue(slot, sc1PTrainingMenuRulesStepValue(sc1PTrainingRemixMenuGetValue(slot), entry->min,
                                                                       sc1PTrainingRemixMenuGetMax(slot), direction));
}

static sb32 sc1PTrainingRemixMenuHandleHorizontal(void)
{
    if (sc1PTrainingRemixMenuIsDirectionPressed(SC1PTRAININGREMIXMENU_DIRECTION_RIGHT, R_CBUTTONS) != FALSE)
    {
        sc1PTrainingRemixMenuStepSelected(SC1PTRAININGMENURULES_STEP_RIGHT);
        return TRUE;
    }
    if (sc1PTrainingRemixMenuIsDirectionPressed(SC1PTRAININGREMIXMENU_DIRECTION_LEFT, L_CBUTTONS) != FALSE)
    {
        sc1PTrainingRemixMenuStepSelected(SC1PTRAININGMENURULES_STEP_LEFT);
        return TRUE;
    }
    return FALSE;
}

/** R goes to the next page and Z to the previous one. */
static sb32 sc1PTrainingRemixMenuHandlePages(void)
{
    if (sc1PTrainingRemixMenuIsTapped(R_TRIG) != FALSE)
    {
        syAudioPlayFGM(SC1PTRAININGREMIXMENU_FGM_SCROLL);
        sSC1PTrainingRemixMenuSelection = sc1PTrainingMenuRulesNextPage(sSC1PTrainingRemixMenuSelection, SC1PTRAININGMENU_SLOT_COUNT);
        return TRUE;
    }
    if (sc1PTrainingRemixMenuIsTapped(Z_TRIG) != FALSE)
    {
        syAudioPlayFGM(SC1PTRAININGREMIXMENU_FGM_SCROLL);
        sSC1PTrainingRemixMenuSelection = sc1PTrainingMenuRulesPrevPage(sSC1PTrainingRemixMenuSelection, SC1PTRAININGMENU_SLOT_COUNT);
        return TRUE;
    }
    return FALSE;
}

/** Training.asm set_percent_ and play_bgm_: the edited value becomes the one the match uses. */
static void sc1PTrainingRemixMenuCommitSelected(s32 slot)
{
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);
    s32 value = sc1PTrainingRemixMenuGetValue(slot);

    remix_training_set(sc1PTrainingMenuEntriesGetOption(entry, sc1PTrainingRemixMenuGetPort()), value);

    if (slot == SC1PTRAININGMENU_SLOT_MUSIC)
    {
        sc1PTrainingRemixPlayMusic();
    }
}

/** A or START on an entry runs its a_function, and the confirm sound plays only when it has one. */
static void sc1PTrainingRemixMenuHandleAction(void)
{
    s32 slot = sSC1PTrainingRemixMenuSelection;
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);

    if (sc1PTrainingRemixMenuIsTapped(A_BUTTON | START_BUTTON) == FALSE)
    {
        return;
    }
    if (entry->action == SC1PTRAININGMENU_ACTION_NONE)
    {
        return;
    }
    if (entry->action == SC1PTRAININGMENU_ACTION_COMMIT)
    {
        sc1PTrainingRemixMenuCommitSelected(slot);
    }
    else sc1PTrainingRemixSpawnSave(sc1PTrainingRemixMenuGetPort());

    syAudioPlayFGM(SC1PTRAININGREMIXMENU_FGM_CONFIRM);
}

/** Training.asm run_ 1818-1864: the Costume of the port on show cannot pass the costumes its character has. */
static void sc1PTrainingRemixMenuClampCostume(void)
{
    s32 costume = sc1PTrainingRemixMenuGetValue(SC1PTRAININGMENU_SLOT_COSTUME);
    s32 clamped = sc1PTrainingMenuRulesClampValue(costume, sc1PTrainingRemixMenuGetMax(SC1PTRAININGMENU_SLOT_COSTUME));

    if (clamped != costume)
    {
        sc1PTrainingRemixMenuSetValue(SC1PTRAININGMENU_SLOT_COSTUME, clamped);
    }
}

/** Menu.update_: one change per frame, in the order down, up, right, left, R, Z, A. */
static void sc1PTrainingRemixMenuHandleInput(void)
{
    if (sc1PTrainingRemixMenuHandleVertical() != FALSE)
    {
        return;
    }
    if (sc1PTrainingRemixMenuHandleHorizontal() != FALSE)
    {
        return;
    }
    if (sc1PTrainingRemixMenuHandlePages() != FALSE)
    {
        return;
    }
    sc1PTrainingRemixMenuHandleAction();
}

static void sc1PTrainingRemixMenuOpen(void)
{
    syAudioPlayFGM(SC1PTRAININGREMIXMENU_FGM_OPEN);
    sSC1PTrainingRemixMenuIsOpen = TRUE;
    sc1PTrainingRemixMenuSyncPending();
    sc1PTrainingModeSetMenuGObjFlags(GOBJ_FLAG_HIDDEN);
}

static void sc1PTrainingRemixMenuClose(void)
{
    sSC1PTrainingRemixMenuIsOpen = FALSE;
    sc1PTrainingModeSetMenuGObjFlags(GOBJ_FLAG_NONE);
}

void sc1PTrainingRemixMenuStartScene(void)
{
    sSC1PTrainingRemixMenuIsOpen = FALSE;
    sSC1PTrainingRemixMenuSelection = 0;
}

void sc1PTrainingRemixMenuUpdate(void)
{
    if (gSCManagerBattleState->game_status != nSCBattleGameStatusPause)
    {
        sSC1PTrainingRemixMenuIsOpen = FALSE;
    }
    else if (sSC1PTrainingRemixMenuIsOpen == FALSE)
    {
        if (sc1PTrainingRemixMenuIsTapped(Z_TRIG) != FALSE)
        {
            sc1PTrainingRemixMenuOpen();
        }
    }
    else
    {
        sc1PTrainingRemixMenuUpdateStick();
        sc1PTrainingRemixMenuHandleInput();

        if (sSC1PTrainingRemixMenuSelection < SC1PTRAININGMENURULES_ROWS_PER_PAGE)
        {
            sc1PTrainingRemixMenuClampCostume();
        }
        if (sc1PTrainingRemixMenuIsTapped(B_BUTTON) != FALSE)
        {
            sc1PTrainingRemixMenuClose();
        }
    }
    sc1PTrainingRemixMenuDrawUpdate(sSC1PTrainingRemixMenuIsOpen);
}

#endif
