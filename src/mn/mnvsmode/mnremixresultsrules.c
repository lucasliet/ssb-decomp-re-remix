/**
 * mnremixresultsrules.c - state free rules of the Remix VS results winner screen, see mnremixresultsrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <string.h>
#include "mn/mnvsmode/mnremixresultsrules.h"
#include "remix/remix_roster_data.h"

int mnRemixResultsRulesIsExtendedFkind(int fkind)
{
    return (fkind >= MNREMIX_RESULTS_FIRST_EXTENDED_FKIND) ? 1 : 0;
}

int mnRemixResultsRulesPickVictoryIndex(int random_index, unsigned int held_buttons)
{
    if ((held_buttons & MNREMIX_RESULTS_C_BUTTONS_MASK) == 0)
    {
        return random_index;
    }
    if ((held_buttons & MNREMIX_RESULTS_C_RIGHT) != 0)
    {
        return 2;
    }
    if ((held_buttons & MNREMIX_RESULTS_C_LEFT) != 0)
    {
        return 1;
    }
    return 0;
}

int mnRemixResultsRulesUsesSingularWin(int fkind, int is_japanese_sound)
{
    if (fkind == REMIX_ID_DRAGONKING)
    {
        return 0;
    }
    if (is_japanese_sound != 0)
    {
        return 1;
    }
    return (fkind == REMIX_ID_BANJO) ? 1 : 0;
}

float mnRemixResultsRulesBitsToFloat(unsigned int bits)
{
    float value;

    memcpy(&value, &bits, sizeof(value));

    return value;
}

int mnRemixResultsRulesCopyName(MNRemixResultsReadByteFunc read_byte, unsigned int ram_addr, char *buffer, int buffer_size)
{
    int length = 0;

    if ((read_byte == NULL) || (buffer == NULL) || (buffer_size < 1))
    {
        return 0;
    }
    while (length < (buffer_size - 1))
    {
        unsigned int character = read_byte(ram_addr + (unsigned int)length) & 0xFF;

        if (character == 0)
        {
            break;
        }
        buffer[length++] = (char)character;
    }
    buffer[length] = '\0';

    return length;
}

#endif
