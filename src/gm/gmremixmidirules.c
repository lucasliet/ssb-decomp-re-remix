/**
 * gmremixmidirules.c - pure rules of the Remix MIDI and FGM patches, see gmremixmidirules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmidirules.h"

int gmRemixMidiRulesFgmIsSilenced(unsigned int fgm_id)
{
    return fgm_id == GMREMIX_MIDI_FGM_SILENT_ID;
}

unsigned int gmRemixMidiRulesSongEntryAddress(unsigned int table_address, int bgm_id, unsigned int entry_size)
{
    if ((bgm_id < 0) || (bgm_id >= GMREMIX_MIDI_COUNT))
    {
        return 0;
    }
    return table_address + ((unsigned int)bgm_id * entry_size);
}

unsigned int gmRemixMidiRulesInstrumentEntryAddress(unsigned int array_address, unsigned int instrument,
                                                    unsigned int entry_size)
{
    if ((array_address == 0) || (instrument >= GMREMIX_MIDI_INSTRUMENT_COUNT))
    {
        return 0;
    }
    return array_address + (instrument * entry_size);
}

int gmRemixMidiRulesPickOverride(int override_value, int original_value)
{
    if (override_value == 0)
    {
        return original_value;
    }
    return override_value;
}

#endif
