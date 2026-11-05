/**
 * ftremixslippyhipsrules.c - pure rule of the Slippy hip translation tables, see ftremixslippyhipsrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/slippy/ftremixslippyhipsrules.h"

int ftRemixSlippyHipFindWordAddress(unsigned int status_id, unsigned int table_address, unsigned int menu_table_address, unsigned int *word_address)
{
    unsigned int index = status_id & 0xFFFF;

    if ((status_id & FTREMIX_SLIPPY_HIP_MENU_FLAG) != 0)
    {
        if (index >= FTREMIX_SLIPPY_HIP_MENU_TABLE_SIZE)
        {
            return 0;
        }
        *word_address = menu_table_address + (index * FTREMIX_SLIPPY_HIP_ENTRY_SIZE);
        return 1;
    }
    if (index >= FTREMIX_SLIPPY_HIP_TABLE_SIZE)
    {
        return 0;
    }
    *word_address = table_address + (index * FTREMIX_SLIPPY_HIP_ENTRY_SIZE);
    return 1;
}

#endif
