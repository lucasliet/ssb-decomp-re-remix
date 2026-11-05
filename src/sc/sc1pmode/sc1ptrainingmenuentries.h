#ifndef _SC1PTRAININGMENUENTRIES_H_
#define _SC1PTRAININGMENUENTRIES_H_

#include <stdint.h>

/*
 * Entries of the Remix Training Z menu in chain order (Training.asm: head "Port:" at 3756, tail_px at 3658-3665 and the shared
 * entries at 3899-3913). The first page shows Port, the seven entries of the chosen port and the first four shared entries.
 * Titles, ranges and value texts are the asm strings. Entries that exist in the port registry (remix_training.h) name their option;
 * Port and Preserve Menu Selections have no registry option because only the menu uses them. Ints and static text only, compiled
 * standalone by tests/remix.
 */

typedef enum SC1PTrainingMenuSlot
{
    SC1PTRAININGMENU_SLOT_PORT,
    SC1PTRAININGMENU_SLOT_CHARACTER,
    SC1PTRAININGMENU_SLOT_COSTUME,
    SC1PTRAININGMENU_SLOT_TYPE,
    SC1PTRAININGMENU_SLOT_SPAWN,
    SC1PTRAININGMENU_SLOT_SET_CUSTOM_SPAWN,
    SC1PTRAININGMENU_SLOT_PERCENT,
    SC1PTRAININGMENU_SLOT_RESET_SETS_PERCENT,
    SC1PTRAININGMENU_SLOT_SHIELD_BREAK,
    SC1PTRAININGMENU_SLOT_OOS,
    SC1PTRAININGMENU_SLOT_MUSIC,
    SC1PTRAININGMENU_SLOT_STAGE_BG,
    SC1PTRAININGMENU_SLOT_TECH,
    SC1PTRAININGMENU_SLOT_DI_TYPE,
    SC1PTRAININGMENU_SLOT_DI_STRENGTH,
    SC1PTRAININGMENU_SLOT_DI_DIRECTION,
    SC1PTRAININGMENU_SLOT_DI_FIRST_HIT,
    SC1PTRAININGMENU_SLOT_DPAD,
    SC1PTRAININGMENU_SLOT_DI_PRACTICE,
    SC1PTRAININGMENU_SLOT_SPAM_PRACTICE,
    SC1PTRAININGMENU_SLOT_SPAM_INTERVAL,
    SC1PTRAININGMENU_SLOT_SPAM_RANDOM,
    SC1PTRAININGMENU_SLOT_PRESERVE,
    SC1PTRAININGMENU_SLOT_COUNT

} SC1PTrainingMenuSlot;

/** Ports of the per-port entries (Character through Reset Sets Percent). */
#define SC1PTRAININGMENU_PORT_COUNT 4

/** Registry option id of an entry that only the menu keeps (Port, Preserve Menu Selections). */
#define SC1PTRAININGMENU_OPTION_LOCAL (-1)

typedef enum SC1PTrainingMenuKind
{
    SC1PTRAININGMENU_KIND_NUMBER,
    SC1PTRAININGMENU_KIND_STRINGS,
    SC1PTRAININGMENU_KIND_CHARACTER,
    SC1PTRAININGMENU_KIND_TITLE

} SC1PTrainingMenuKind;

/** What A does on an entry (the a_function of Menu.entry). */
typedef enum SC1PTrainingMenuAction
{
    SC1PTRAININGMENU_ACTION_NONE,
    SC1PTRAININGMENU_ACTION_COMMIT,
    SC1PTRAININGMENU_ACTION_SET_CUSTOM_SPAWN

} SC1PTrainingMenuAction;

typedef struct SC1PTrainingMenuEntry
{
    const char *title;
    int32_t kind;
    int32_t option;
    int32_t is_per_port;
    int32_t min;
    int32_t max;
    const char *const *strings;
    int32_t action;

} SC1PTrainingMenuEntry;

/**
 * Entry of a slot.
 *
 * @param slot SC1PTrainingMenuSlot, 0 to SC1PTRAININGMENU_SLOT_COUNT - 1.
 * @return The entry, or NULL for a slot outside the list.
 */
extern const SC1PTrainingMenuEntry *sc1PTrainingMenuEntriesGet(int32_t slot);

/**
 * Registry option an entry edits for a port.
 *
 * @param entry Entry with a registry option.
 * @param port Port 0 to 3, ignored by entries that are not per port.
 */
extern int32_t sc1PTrainingMenuEntriesGetOption(const SC1PTrainingMenuEntry *entry, int32_t port);

/**
 * Text the menu draws for a value of a string entry.
 *
 * @return The text, or NULL for a number entry, a title or a value outside the range of the entry.
 */
extern const char *sc1PTrainingMenuEntriesGetText(const SC1PTrainingMenuEntry *entry, int32_t value);

#endif
