/**
 * ftremixaitablesrules.c - pure rules of the Remix CPU attack tables, see ftremixaitablesrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <string.h>
#include "ft/ftremix/ftremixaitablesrules.h"

int ftRemixAiTablesRulesIsTerminator(unsigned int input_word)
{
    return (input_word == FTREMIX_AI_TABLES_TERMINATOR_INPUT) ? 1 : 0;
}

int ftRemixAiTablesRulesIsCustomInput(int input_kind)
{
    return (input_kind >= FTREMIX_AI_TABLES_FIRST_CUSTOM_INPUT) ? 1 : 0;
}

int ftRemixAiTablesRulesCountEntries(const unsigned int *words, int entry_capacity)
{
    int entry;
    int terminators = 0;

    for (entry = 0; entry < entry_capacity; entry++)
    {
        if (ftRemixAiTablesRulesIsTerminator(words[entry * FTREMIX_AI_TABLES_WORDS_PER_ENTRY]) == 0)
        {
            continue;
        }
        terminators++;
        if (terminators == FTREMIX_AI_TABLES_TERMINATORS_PER_TABLE)
        {
            return entry + 1;
        }
    }
    return -1;
}

float ftRemixAiTablesRulesBitsToFloat(unsigned int bits)
{
    float value;

    memcpy(&value, &bits, sizeof(value));
    return value;
}

int ftRemixAiTablesRulesIsLv10Level(int level)
{
    return (level == FTREMIX_AI_TABLES_LV10_LEVEL) ? 1 : 0;
}

int ftRemixAiTablesRulesIsTableIndex(int fkind)
{
    return ((fkind >= 0) && (fkind < FTREMIX_AI_TABLES_CHARACTER_COUNT)) ? 1 : 0;
}

#endif
