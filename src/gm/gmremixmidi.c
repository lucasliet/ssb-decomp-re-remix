/**
 * gmremixmidi.c - per-song instrument and master volume overrides of the Remix sequence player, see gmremixmidi.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmidi.h"
#include "gm/gmremixmidirules.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include <port_log.h>
#include <stdlib.h>
#include <sys/audio.h>

/** Number of BGM player whose song selects the overrides (the only one, SYAUDIO_BGMPLAYERS_NUM is 1). */
#define GMREMIX_MIDI_BGM_PLAYER 0

static s32 sGMRemixMidiLogEnabled = -1;
static s32 sGMRemixMidiLoggedTrack = -1;
static u8 sGMRemixMidiLoggedMasterVolume;

/** SSB64_MIDI_LOG=1 turns on the `[midi]` lines of the overrides; unset, empty or "0" is off. */
static sb32 gmRemixMidiLogIsEnabled(void)
{
    const char *text;

    if (sGMRemixMidiLogEnabled < 0)
    {
        text = getenv("SSB64_MIDI_LOG");

        sGMRemixMidiLogEnabled = ((text != NULL) && (text[0] != '\0') && (text[0] != '0')) ? 1 : 0;
    }
    return (sGMRemixMidiLogEnabled != 0) ? TRUE : FALSE;
}

/**
 * Address of the override array of the playing song in a pointer table (priority or bend range).
 *
 * @param table_address RAM address of the table.
 * @return The array address, 0 when the song has none.
 */
static unsigned int gmRemixMidiGetSongArray(unsigned int table_address)
{
    unsigned int entry_address =
        gmRemixMidiRulesSongEntryAddress(table_address, syAudioGetBGMPlaying(GMREMIX_MIDI_BGM_PLAYER), 4);

    if (entry_address == 0)
    {
        return 0;
    }
    return remix_ram_image_read_word(entry_address);
}

/**
 * Priority override of an instrument for the playing song.
 *
 * @param instrument Instrument id.
 * @return The priority, 0 when not overridden.
 */
static int gmRemixMidiGetPriorityOverride(u8 instrument)
{
    unsigned int array_address = gmRemixMidiGetSongArray(REMIX_SYM_MIDI__priority_override_table);
    unsigned int entry_address = gmRemixMidiRulesInstrumentEntryAddress(array_address, instrument, 1);

    if (entry_address == 0)
    {
        return 0;
    }
    return (int)remix_ram_image_read_byte(entry_address);
}

/**
 * Bend range override of an instrument for the playing song (a signed halfword, as the asm lh).
 *
 * @param instrument Instrument id.
 * @return The bend range in cents, 0 when not overridden.
 */
static int gmRemixMidiGetBendRangeOverride(u8 instrument)
{
    unsigned int array_address = gmRemixMidiGetSongArray(REMIX_SYM_MIDI__bend_range_override_table);
    unsigned int entry_address = gmRemixMidiRulesInstrumentEntryAddress(array_address, instrument, 2);

    if (entry_address == 0)
    {
        return 0;
    }
    return (s16)remix_ram_image_read_half(entry_address);
}

void gmRemixMidiOverrideInstrument(ALChanState *chan_state, u8 instrument)
{
    s32 priority = chan_state->priority;
    s32 bend_range = chan_state->bendRange;

    chan_state->priority = gmRemixMidiRulesPickOverride(gmRemixMidiGetPriorityOverride(instrument), priority);
    chan_state->bendRange = gmRemixMidiRulesPickOverride(gmRemixMidiGetBendRangeOverride(instrument), bend_range);
    if (((chan_state->priority != priority) || (chan_state->bendRange != bend_range)) && gmRemixMidiLogIsEnabled())
    {
        port_log("[midi] instrument track=%d inst=%u priority=%d->%d bend=%d->%d\n",
                 syAudioGetBGMPlaying(GMREMIX_MIDI_BGM_PLAYER), (u32)instrument, priority, chan_state->priority,
                 bend_range, chan_state->bendRange);
    }
}

/** Logs a master volume override once per song and value. */
static void gmRemixMidiLogMasterVolume(u8 original, u8 overridden)
{
    s32 track = syAudioGetBGMPlaying(GMREMIX_MIDI_BGM_PLAYER);

    if ((track == sGMRemixMidiLoggedTrack) && (overridden == sGMRemixMidiLoggedMasterVolume))
    {
        return;
    }
    sGMRemixMidiLoggedTrack = track;
    sGMRemixMidiLoggedMasterVolume = overridden;
    port_log("[midi] master track=%d volume=%u->%u\n", track, (u32)original, (u32)overridden);
}

u8 gmRemixMidiMasterVolume(u8 master_volume)
{
    u8 overridden;
    unsigned int entry_address = gmRemixMidiRulesSongEntryAddress(
        REMIX_SYM_MIDI__master_volume_override_table, syAudioGetBGMPlaying(GMREMIX_MIDI_BGM_PLAYER), 1);

    if (entry_address == 0)
    {
        return master_volume;
    }
    overridden = (u8)gmRemixMidiRulesPickOverride((int)remix_ram_image_read_byte(entry_address), master_volume);
    if ((overridden != master_volume) && gmRemixMidiLogIsEnabled())
    {
        gmRemixMidiLogMasterVolume(master_volume, overridden);
    }
    return overridden;
}

#endif
