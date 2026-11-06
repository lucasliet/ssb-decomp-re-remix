#ifndef _GMREMIXMIDI_H_
#define _GMREMIXMIDI_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/libaudio.h>
#include <ssb_types.h>

/*
 * Per-song overrides of the Remix sequence player (midi.asm priority_override_table, bend_range_override_table and
 * master_volume_override_table). The tables live in the Remix RAM image and are indexed by the song that is playing
 * (BGM.safe_id, syAudioGetBGMPlaying).
 */

/**
 * Applies the priority and bend range overrides of the playing song to a channel that just received a program change
 * (midi.asm override_instrument_parameters_, which replaces __n_setInstChanState_Alt in the CSP MIDI handler). A zero
 * override leaves the instrument value.
 *
 * @param chan_state Channel state, already set from the instrument.
 * @param instrument Program change value (instrument id).
 */
extern void gmRemixMidiOverrideInstrument(ALChanState *chan_state, u8 instrument);

/**
 * Master volume of the sequence player: the override of the playing song when it has one (midi.asm
 * override_master_volume_), else the volume the sequence set.
 *
 * @param master_volume Master volume of the sequence player.
 * @return The volume to use.
 */
extern u8 gmRemixMidiMasterVolume(u8 master_volume);

#endif

#endif
