/**
 * ftremixaitables.c - CPU attack lists of Remix fighters and of lv10 CPUs, read from the Remix RAM image, see ftremixaitables.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <stdlib.h>
#include <port_log.h>
#include <ft/fighter.h>
#include "ft/ftremix/ftremixaicommands.h"
#include "ft/ftremix/ftremixaitables.h"
#include "ft/ftremix/ftremixaitablesrules.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define FTREMIX_AI_TABLES_MAX_ENTRIES 128
#define FTREMIX_AI_TABLES_MAX_WORDS (FTREMIX_AI_TABLES_MAX_ENTRIES * FTREMIX_AI_TABLES_WORDS_PER_ENTRY)
#define FTREMIX_AI_TABLES_POINTER_BYTES 4

typedef struct FTRemixAiListSlot
{
    s32 built;
    FTComputerAttack *list;
} FTRemixAiListSlot;

static FTRemixAiListSlot sFTRemixAiNormalSlots[FTREMIX_AI_TABLES_CHARACTER_COUNT];
static FTRemixAiListSlot sFTRemixAiLv10Slots[FTREMIX_AI_TABLES_CHARACTER_COUNT];

/**
 * Reads the words of the table at table_addr, up to the walk capacity. Returns FALSE when table_addr is outside the Remix RAM image.
 */
static s32 ftRemixAiReadTableWords(u32 table_addr, unsigned int *words)
{
    s32 index;

    if (remix_ram_image_resolve(table_addr) == NULL)
    {
        return FALSE;
    }
    for (index = 0; index < FTREMIX_AI_TABLES_MAX_WORDS; index++)
    {
        words[index] = remix_ram_image_read_word(table_addr + (index * sizeof(unsigned int)));
    }
    return TRUE;
}

/**
 * Converts the FTREMIX_AI_TABLES_WORDS_PER_ENTRY words of one entry to a FTComputerAttack. The floats are bit-cast, not converted.
 */
static void ftRemixAiWordsToAttack(const unsigned int *words, FTComputerAttack *attack)
{
    attack->input_kind = (s32)words[0];
    attack->hit_start_frame = (s32)words[1];
    attack->hit_end_frame = (s32)words[2];
    attack->detect_near_x = ftRemixAiTablesRulesBitsToFloat(words[3]);
    attack->detect_far_x = ftRemixAiTablesRulesBitsToFloat(words[4]);
    attack->detect_near_y = ftRemixAiTablesRulesBitsToFloat(words[5]);
    attack->detect_far_y = ftRemixAiTablesRulesBitsToFloat(words[6]);
}

/**
 * Entries with a vanilla input id and both terminators are kept. A custom input id is kept only when its routine is ported.
 */
static s32 ftRemixAiKeepsEntry(s32 input_kind)
{
    if (ftRemixAiTablesRulesIsCustomInput(input_kind) == FALSE)
    {
        return TRUE;
    }
    return ftRemixAiIsCustomInputSupported(input_kind);
}

/**
 * Copies the kept entries of the walked table into list, in order. Returns the number of kept entries.
 */
static s32 ftRemixAiCopyKeptEntries(const unsigned int *words, s32 entry_count, FTComputerAttack *list)
{
    s32 entry;
    s32 kept = 0;
    FTComputerAttack attack;

    for (entry = 0; entry < entry_count; entry++)
    {
        ftRemixAiWordsToAttack(&words[entry * FTREMIX_AI_TABLES_WORDS_PER_ENTRY], &attack);
        if (ftRemixAiKeepsEntry(attack.input_kind) == FALSE)
        {
            continue;
        }
        list[kept] = attack;
        kept++;
    }
    return kept;
}

/**
 * Builds the list of the table at table_addr for fkind and logs the result. table_name is only for the log.
 * Returns NULL when the table is outside the image or has no second terminator inside the capacity.
 */
static FTComputerAttack *ftRemixAiBuildList(s32 fkind, u32 table_addr, const char *table_name)
{
    unsigned int words[FTREMIX_AI_TABLES_MAX_WORDS];
    s32 entry_count;
    s32 kept;
    FTComputerAttack *list;

    if (ftRemixAiReadTableWords(table_addr, words) == FALSE)
    {
        port_log("SSB64: ftRemix AI: fkind %d %s table at 0x%08X is outside the Remix RAM image, using the registry list\n", fkind, table_name, table_addr);
        return NULL;
    }
    entry_count = ftRemixAiTablesRulesCountEntries(words, FTREMIX_AI_TABLES_MAX_ENTRIES);
    if (entry_count < 0)
    {
        port_log("SSB64: ftRemix AI: fkind %d %s table at 0x%08X has no second terminator within %d entries, using the registry list\n", fkind, table_name, table_addr, FTREMIX_AI_TABLES_MAX_ENTRIES);
        return NULL;
    }
    list = malloc(entry_count * sizeof(FTComputerAttack));
    if (list == NULL)
    {
        port_log("SSB64: ftRemix AI: fkind %d %s table at 0x%08X: out of memory for %d entries, using the registry list\n", fkind, table_name, table_addr, entry_count);
        return NULL;
    }
    kept = ftRemixAiCopyKeptEntries(words, entry_count, list);
    port_log("SSB64: ftRemix AI: fkind %d %s table at 0x%08X built: %d entries kept, %d dropped\n", fkind, table_name, table_addr, kept, entry_count - kept);
    return list;
}

/**
 * Attack list of a Remix fighter from Character.ai_behaviour.table. A zero pointer is logged: every Remix fighter has its own table.
 */
static FTComputerAttack *ftRemixAiGetNormalList(s32 fkind)
{
    FTRemixAiListSlot *slot = &sFTRemixAiNormalSlots[fkind];
    u32 table_addr;

    if (slot->built)
    {
        return slot->list;
    }
    table_addr = remix_ram_image_read_word(REMIX_SYM_Character__ai_behaviour__table + (fkind * FTREMIX_AI_TABLES_POINTER_BYTES));
    if (table_addr == 0)
    {
        port_log("SSB64: ftRemix AI: fkind %d has no entry in Character.ai_behaviour.table, using the registry list\n", fkind);
    }
    else
    {
        slot->list = ftRemixAiBuildList(fkind, table_addr, "normal");
    }
    slot->built = TRUE;
    return slot->list;
}

/**
 * Lv10 attack list from Character.lv10_ai_behaviour.table. A zero pointer is the normal case for most fighters, so it is not logged.
 */
static FTComputerAttack *ftRemixAiGetLv10List(s32 fkind)
{
    FTRemixAiListSlot *slot = &sFTRemixAiLv10Slots[fkind];
    u32 table_addr;

    if (slot->built)
    {
        return slot->list;
    }
    table_addr = remix_ram_image_read_word(REMIX_SYM_Character__lv10_ai_behaviour__table + (fkind * FTREMIX_AI_TABLES_POINTER_BYTES));
    if (table_addr != 0)
    {
        slot->list = ftRemixAiBuildList(fkind, table_addr, "lv10");
    }
    slot->built = TRUE;
    return slot->list;
}

FTComputerAttack *ftRemixAiGetAttackList(s32 fkind, s32 level)
{
    FTComputerAttack *lv10_list;

    if (ftRemixAiTablesRulesIsTableIndex(fkind) == FALSE)
    {
        return NULL;
    }
    if (ftRemixAiTablesRulesIsLv10Level(level))
    {
        lv10_list = ftRemixAiGetLv10List(fkind);
        if (lv10_list != NULL)
        {
            return lv10_list;
        }
    }
    if (fkind < nFTKindEnumCount)
    {
        return NULL;
    }
    return ftRemixAiGetNormalList(fkind);
}

#endif
