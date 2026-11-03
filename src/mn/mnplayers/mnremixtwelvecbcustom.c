/**
 * mnremixtwelvecbcustom.c - the Custom character set of the 12-Character Battle, see mnremixtwelvecbcustom.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixtwelvecbcustom.h"
#include "mn/mnplayers/mnremixtwelvecblayout.h"

#define CUSTOM_PLAYERS 2

static int sCustomFkinds[CUSTOM_PLAYERS][MNREMIX_TCB_CUSTOM_ENTRIES];
static int sCustomPresetIndex[CUSTOM_PLAYERS];
static int sCustomIsReady;

static int playerIsValid(int player)
{
    return (player >= 0) && (player < CUSTOM_PLAYERS);
}

static void fillFromSet(int player, int set)
{
    int index;

    for (index = 0; index < MNREMIX_TCB_CUSTOM_ENTRIES; index++)
    {
        int row = index / MNREMIX_TCB_HALF_COLUMNS;
        int column = index % MNREMIX_TCB_HALF_COLUMNS;

        sCustomFkinds[player][index] = mnRemixTwelveCBLayoutFkind(set, (row * MNREMIX_TCB_COLUMNS) + column);
    }
}

void mnRemixTwelveCBCustomReset(void)
{
    int player;

    for (player = 0; player < CUSTOM_PLAYERS; player++)
    {
        fillFromSet(player, 0);
        sCustomPresetIndex[player] = 0;
    }
    sCustomIsReady = 1;
}

static void ensureReady(void)
{
    if (sCustomIsReady == 0)
    {
        mnRemixTwelveCBCustomReset();
    }
}

int mnRemixTwelveCBCustomIndexOfSlot(int slot)
{
    if ((slot < 0) || (slot >= MNREMIX_TCB_SLOTS))
    {
        return -1;
    }
    return ((slot / MNREMIX_TCB_COLUMNS) * MNREMIX_TCB_HALF_COLUMNS) + (slot % MNREMIX_TCB_HALF_COLUMNS);
}

int mnRemixTwelveCBCustomFkind(int slot)
{
    int index = mnRemixTwelveCBCustomIndexOfSlot(slot);

    if (index < 0)
    {
        return -1;
    }
    ensureReady();
    return sCustomFkinds[mnRemixTwelveCBLayoutOwner(slot)][index];
}

static int rosterPositionOf(const int *roster, int roster_count, int fkind)
{
    int position;

    for (position = 0; position < roster_count; position++)
    {
        if (roster[position] == fkind)
        {
            return position;
        }
    }
    return 0;
}

int mnRemixTwelveCBCustomCycle(int slot, int direction, const int *roster, int roster_count, int skip_fkind)
{
    int index = mnRemixTwelveCBCustomIndexOfSlot(slot);
    int *entry;
    int position;
    int step;

    if ((index < 0) || (roster_count <= 0) || (direction == 0))
    {
        return 0;
    }
    ensureReady();
    entry = &sCustomFkinds[mnRemixTwelveCBLayoutOwner(slot)][index];
    position = rosterPositionOf(roster, roster_count, *entry);

    for (step = 0; step < roster_count; step++)
    {
        position = (position + roster_count + ((direction > 0) ? 1 : -1)) % roster_count;
        if (roster[position] != skip_fkind)
        {
            *entry = roster[position];
            return 1;
        }
    }
    return 0;
}

void mnRemixTwelveCBCustomApplyAll(int player, int fkind)
{
    int index;

    if (playerIsValid(player) == 0)
    {
        return;
    }
    ensureReady();
    for (index = 0; index < MNREMIX_TCB_CUSTOM_ENTRIES; index++)
    {
        sCustomFkinds[player][index] = fkind;
    }
}

void mnRemixTwelveCBCustomPreset(int player)
{
    if (playerIsValid(player) == 0)
    {
        return;
    }
    ensureReady();
    fillFromSet(player, sCustomPresetIndex[player]);
    sCustomPresetIndex[player] = (sCustomPresetIndex[player] + 1) % MNREMIX_TCB_CUSTOM_PRESETS;
}

void mnRemixTwelveCBCustomCopyOpponent(int player)
{
    int index;

    if (playerIsValid(player) == 0)
    {
        return;
    }
    ensureReady();
    for (index = 0; index < MNREMIX_TCB_CUSTOM_ENTRIES; index++)
    {
        sCustomFkinds[player][index] = sCustomFkinds[player ^ 1][index];
    }
}

void mnRemixTwelveCBCustomRandomize(int player, int (*pick)(void))
{
    int index;

    if (playerIsValid(player) == 0)
    {
        return;
    }
    ensureReady();
    for (index = 0; index < MNREMIX_TCB_CUSTOM_ENTRIES; index++)
    {
        sCustomFkinds[player][index] = pick();
    }
}

int mnRemixTwelveCBCustomRepeat(int *timer, int is_held)
{
    if (is_held == 0)
    {
        *timer = MNREMIX_TCB_CUSTOM_REPEAT_IDLE;
        return 0;
    }
    (*timer)++;
    if (*timer == 0)
    {
        return 1;
    }
    if (*timer < MNREMIX_TCB_CUSTOM_REPEAT_DELAY)
    {
        return 0;
    }
    *timer -= MNREMIX_TCB_CUSTOM_REPEAT_STEP;
    return 1;
}

MNRemixTwelveCBCustomDpadUp mnRemixTwelveCBCustomDpadUp(int *timer, int is_held, int is_released)
{
    if (is_released != 0)
    {
        int is_short = (*timer < MNREMIX_TCB_CUSTOM_COPY_FRAMES);

        *timer = 0;
        return (is_short != 0) ? MNREMIX_TCB_CUSTOM_DPAD_UP_PRESET : MNREMIX_TCB_CUSTOM_DPAD_UP_NONE;
    }
    if (is_held == 0)
    {
        *timer = 0;
        return MNREMIX_TCB_CUSTOM_DPAD_UP_NONE;
    }
    (*timer)++;
    return (*timer == MNREMIX_TCB_CUSTOM_COPY_FRAMES) ? MNREMIX_TCB_CUSTOM_DPAD_UP_COPY : MNREMIX_TCB_CUSTOM_DPAD_UP_NONE;
}

#endif
