#ifndef _GMREMIXMIDIRULES_H_
#define _GMREMIXMIDIRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix MIDI and FGM patches (midi.asm override_instrument_parameters_ and override_master_volume_,
 * FGM.asm fix_0x2b7_), separate from the audio calls so the headless unit tests (tests/remix) can run them.
 */

/** Number of songs of the Remix music table (MIDI.midi_count): the length of the three override tables. */
#define GMREMIX_MIDI_COUNT 376

/** Number of instruments of the Remix bank (MIDI.instrument_count): the length of every per-song override array. */
#define GMREMIX_MIDI_INSTRUMENT_COUNT 71

/** FGM id that stands for "no sound" (FGM.ORIGINAL_FGM_COUNT, the first id the Remix adds). */
#define GMREMIX_MIDI_FGM_SILENT_ID 0x2B7

/** TRUE when the FGM id plays nothing (fix_0x2b7_ treats it as out of bounds). */
extern int gmRemixMidiRulesFgmIsSilenced(unsigned int fgm_id);

/**
 * Address of the entry of a per-song table: a pointer table (4 bytes per song, entry_size 4) or the master volume table
 * (1 byte per song, entry_size 1).
 *
 * @param table_address RAM address of the table.
 * @param bgm_id Song being played, -1 when none.
 * @param entry_size Bytes per entry.
 * @return The address, or 0 when the song has no entry (none playing or beyond the music table).
 */
extern unsigned int gmRemixMidiRulesSongEntryAddress(unsigned int table_address, int bgm_id, unsigned int entry_size);

/**
 * Address of the entry of a per-instrument override array.
 *
 * @param array_address RAM address of the array, 0 when the song has none.
 * @param instrument Program change value (instrument id).
 * @param entry_size Bytes per entry (1 for priority, 2 for bend range).
 * @return The address, or 0 when the song has no array or the instrument is beyond the Remix bank.
 */
extern unsigned int gmRemixMidiRulesInstrumentEntryAddress(unsigned int array_address, unsigned int instrument,
                                                           unsigned int entry_size);

/** Override when it is non-zero, else the original value (every override table uses 0 for "not set"). */
extern int gmRemixMidiRulesPickOverride(int override_value, int original_value);

#endif

#endif
