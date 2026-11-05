/**
 * sc1ptrainingmenuentries.c - entries of the Training Z menu, see sc1ptrainingmenuentries.h.
 */

#include "sc/sc1pmode/sc1ptrainingmenuentries.h"
#include "remix/remix_training.h"

#include <stddef.h>

/* Menu.asm bool_0 and bool_1. */
static const char *const sBoolTexts[] = { "OFF", "ON" };
static const char *const sShieldBreakTexts[] = { "OFF", "ON", "Infinite" };
static const char *const sDpadTexts[] = { "ON", "Reset Only", "OFF" };
static const char *const sTypeTexts[] = { "Auto", "Human", "CPU", "Disabled" };
static const char *const sCostumeTexts[] = { "Auto", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };
static const char *const sSpawnTexts[] = { "Auto", "Port 1", "Port 2", "Port 3", "Port 4", "Custom" };

/* Training.asm string_table_oos_options (2951-2962). */
static const char *const sOosTexts[] =
{
    "Jump", "Roll Left", "Roll Right", "Grab", "Up Smash", "Up Special", "Down Special", "Neutral Air", "Shield Drop", "S. Drop DSP",
    "S. Drop NAir", "None"
};

/* Training.asm string_table_tech_options (2965-2972). */
static const char *const sTechTexts[] = { "Random", "Roll Backward", "Roll Forward", "In Place", "Roll Left", "Roll Right", "None" };

static const char *const sDiTypeTexts[] = { "None", "Random", "Smash", "Slide" };
static const char *const sDiStrengthTexts[] = { "Random", "High", "Medium", "Low" };
static const char *const sDiDirectionTexts[] = { "Random", "Left", "Right", "Up", "Down", "Away", "Toward" };

/* Training.asm string_table_spam_practice (3002-3016): the tilt and smash names are Action.asm string_0x0C7 and its neighbours. */
static const char *const sSpamTexts[] =
{
    "OFF", "UTilt", "DTilt", "FTilt", "USmash", "DSmash", "FSmash", "Down Special", "Neutral Special", "Up Special", "Grab", "Jab",
    "Short Hop", "Full Hop"
};

/* Training.asm string_table_music (3772-3788): the track names are the Toggles.asm random music entries. */
static const char *const sMusicTexts[] =
{
    "Training Mode", "Bonus", "Congo Jungle", "Credits", "Data", "Dream Land", "Duel Zone", "Final Destination", "How To Play",
    "Hyrule Castle", "Meta Crystal", "Mushroom Kingdom", "Peach's Castle", "Planet Zebes", "Saffron City", "Sector Z", "Yoshi's Island"
};

typedef struct SC1PTrainingMenuCharacter
{
    int32_t fkind;
    const char *name;

} SC1PTrainingMenuCharacter;

/* The same table the port registry maps Character values with (string_table_char order). */
static const SC1PTrainingMenuCharacter sCharacters[] =
{
#include "remix/remix_training_characters_data.inc"
};

#define SC1PTRAININGMENUENTRIES_CHARACTER_COUNT ((int32_t)(sizeof(sCharacters) / sizeof(sCharacters[0])))

/* Menu.entry order: head, tail_px (3658-3665), then 3899-3913. Ranges are the asm min and max, with Auto added where the registry has it. */
static const SC1PTrainingMenuEntry sEntries[SC1PTRAININGMENU_SLOT_COUNT] =
{
    { "Port:", SC1PTRAININGMENU_KIND_NUMBER, SC1PTRAININGMENU_OPTION_LOCAL, 0, 1, SC1PTRAININGMENU_PORT_COUNT, NULL,
      SC1PTRAININGMENU_ACTION_NONE },
    { "Character:", SC1PTRAININGMENU_KIND_CHARACTER, REMIX_TRAINING_CHARACTER_P1, 1, 0, SC1PTRAININGMENUENTRIES_CHARACTER_COUNT, NULL,
      SC1PTRAININGMENU_ACTION_NONE },
    { "Costume:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_COSTUME_P1, 1, 0, SC1PTRAININGMENUENTRIES_COSTUME_MAX, sCostumeTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "Type:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_TYPE_P1, 1, 0, 3, sTypeTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "Spawn:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_SPAWN_P1, 1, 0, 5, sSpawnTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "Set Custom Spawn", SC1PTRAININGMENU_KIND_TITLE, SC1PTRAININGMENU_OPTION_LOCAL, 1, 0, 0, NULL,
      SC1PTRAININGMENU_ACTION_SET_CUSTOM_SPAWN },
    { "Percent:", SC1PTRAININGMENU_KIND_NUMBER, REMIX_TRAINING_PERCENT_P1, 1, 0, 999, NULL, SC1PTRAININGMENU_ACTION_COMMIT },
    { "Reset Sets Percent:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_RESET_SETS_PERCENT_P1, 1, 0, 1, sBoolTexts,
      SC1PTRAININGMENU_ACTION_NONE },
    { "Shield Break Mode:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_SHIELD_BREAK, 0, 0, 2, sShieldBreakTexts,
      SC1PTRAININGMENU_ACTION_NONE },
    { "OOS Action:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_OOS, 0, 0, 11, sOosTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "Music:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_MUSIC, 0, 0, 16, sMusicTexts, SC1PTRAININGMENU_ACTION_COMMIT },
    { "Stage Background:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_STAGE_BG, 0, 0, 1, sBoolTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "CPU Teching:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_TECH, 0, 0, 6, sTechTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "CPU DI Type:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_DI_TYPE, 0, 0, 3, sDiTypeTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "CPU DI Strength:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_DI_STRENGTH, 0, 0, 3, sDiStrengthTexts,
      SC1PTRAININGMENU_ACTION_NONE },
    { "CPU DI Direction:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_DI_DIRECTION, 0, 0, 6, sDiDirectionTexts,
      SC1PTRAININGMENU_ACTION_NONE },
    { "CPU DI Start on Hit:", SC1PTRAININGMENU_KIND_NUMBER, REMIX_TRAINING_DI_FIRST_HIT, 0, 1, 99, NULL, SC1PTRAININGMENU_ACTION_NONE },
    { "D-pad Controls:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_DPAD, 0, 0, 2, sDpadTexts, SC1PTRAININGMENU_ACTION_NONE },
    { "DI Practice Mode:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_DI_PRACTICE, 0, 0, 1, sBoolTexts,
      SC1PTRAININGMENU_ACTION_NONE },
    { "Spam Practice:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_SPAM_PRACTICE, 0, 0, 13, sSpamTexts,
      SC1PTRAININGMENU_ACTION_NONE },
    { "Spam Interval:", SC1PTRAININGMENU_KIND_NUMBER, REMIX_TRAINING_SPAM_INTERVAL, 0, 1, 999, NULL, SC1PTRAININGMENU_ACTION_NONE },
    { "Spam Interval Random:", SC1PTRAININGMENU_KIND_STRINGS, REMIX_TRAINING_SPAM_RANDOM, 0, 0, 1, sBoolTexts,
      SC1PTRAININGMENU_ACTION_NONE },
    { "Preserve Menu Selections:", SC1PTRAININGMENU_KIND_STRINGS, SC1PTRAININGMENU_OPTION_LOCAL, 0, 0, 1, sBoolTexts,
      SC1PTRAININGMENU_ACTION_NONE }
};

const SC1PTrainingMenuEntry *sc1PTrainingMenuEntriesGet(int32_t slot)
{
    if ((slot < 0) || (slot >= SC1PTRAININGMENU_SLOT_COUNT))
    {
        return NULL;
    }
    return &sEntries[slot];
}

int32_t sc1PTrainingMenuEntriesGetOption(const SC1PTrainingMenuEntry *entry, int32_t port)
{
    if (entry->option == SC1PTRAININGMENU_OPTION_LOCAL)
    {
        return SC1PTRAININGMENU_OPTION_LOCAL;
    }
    return (entry->is_per_port != 0) ? (entry->option + port) : entry->option;
}

const char *sc1PTrainingMenuEntriesGetText(const SC1PTrainingMenuEntry *entry, int32_t value)
{
    if ((value < entry->min) || (value > entry->max))
    {
        return NULL;
    }
    if (entry->kind == SC1PTRAININGMENU_KIND_CHARACTER)
    {
        return (value == 0) ? "Auto" : sCharacters[value - 1].name;
    }
    if (entry->kind != SC1PTRAININGMENU_KIND_STRINGS)
    {
        return NULL;
    }
    return entry->strings[value - entry->min];
}
