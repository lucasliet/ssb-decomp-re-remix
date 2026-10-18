/**
 * mnremixsoundtest.c - Smash Remix sound test ids (src/midi.asm augment_sound_test_music_, src/FGM.asm
 * augment_sound_test_voice_).
 *
 * The vanilla screen picks from 45 music and 244 voice entries. Remix extends both: music slots past the vanilla ones map
 * through MIDI.extended_music_map_table (0xA, 0xB, then every added song), voice slots past them through
 * FGM.extended_voice_map_table (one word per added sound, the low halfword is the FGM id).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define MNREMIX_SOUNDTEST_VANILLA_MUSIC 0x2D
#define MNREMIX_SOUNDTEST_VANILLA_VOICE 0xF4

extern u32 dMNSoundTestMusicIDs[];
extern u32 dMNSoundTestVoiceIDs[];

static u32 mnRemixSoundTestReadLow(u32 ram_addr)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr);

    return (word != NULL) ? (*word & 0xFFFFU) : 0;
}

u32 mnRemixSoundTestMusicID(s32 select)
{
    if (select < MNREMIX_SOUNDTEST_VANILLA_MUSIC)
    {
        return dMNSoundTestMusicIDs[select];
    }
    return mnRemixSoundTestReadLow(REMIX_SYM_MIDI__extended_music_map_table + ((select - MNREMIX_SOUNDTEST_VANILLA_MUSIC) * 4));
}

u32 mnRemixSoundTestVoiceID(s32 select)
{
    if (select < MNREMIX_SOUNDTEST_VANILLA_VOICE)
    {
        return dMNSoundTestVoiceIDs[select];
    }
    return mnRemixSoundTestReadLow(REMIX_SYM_FGM__extended_voice_map_table + ((select - MNREMIX_SOUNDTEST_VANILLA_VOICE) * 4));
}

#endif
