/**
 * grremixrandommusic.c - Random Music list of the stage BGM (BGM.asm random_music_, add_song_to_random_list_).
 *
 * Custom MIDI tracks of Toggles.asm (entry_random_music_sorted_<id>) are not listed: their ids are assembly-time
 * values with no symbol in remix_symbols.h. Menu Music RANDOM ALL and the pause D-pad cycle are not ported.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "grremixrandommusic.h"
#include "gm/gmremixtoggle.h"
#include "remix/remix_symbols.h"
#include "remix/remix_toggles.h"
#include <sys/utils.h>
#include <macros.h>

typedef struct GRRemixRandomTrack
{
    u32 entry_addr;
    u32 bgm_id;
} GRRemixRandomTrack;

/* Same order as BGM.asm random_music_, the order fixes which index a salty runback replays. */
static const GRRemixRandomTrack sRandomTracks[] =
{
    { REMIX_SYM_Toggles__entry_random_music_peachs_castle, 6 },
    { REMIX_SYM_Toggles__entry_random_music_sector_z, 4 },
    { REMIX_SYM_Toggles__entry_random_music_congo_jungle, 5 },
    { REMIX_SYM_Toggles__entry_random_music_planet_zebes, 1 },
    { REMIX_SYM_Toggles__entry_random_music_hyrule_castle, 9 },
    { REMIX_SYM_Toggles__entry_random_music_yoshis_island, 8 },
    { REMIX_SYM_Toggles__entry_random_music_dream_land, 0 },
    { REMIX_SYM_Toggles__entry_random_music_saffron_city, 7 },
    { REMIX_SYM_Toggles__entry_random_music_meta_crystal, 37 },
    { REMIX_SYM_Toggles__entry_random_music_mushroom_kingdom, 2 },
    { REMIX_SYM_Toggles__entry_random_music_duel_zone, 36 },
    { REMIX_SYM_Toggles__entry_random_music_final_destination, 25 },
    { REMIX_SYM_Toggles__entry_random_music_how_to_play, 34 },
    { REMIX_SYM_Toggles__entry_random_music_data, 43 },
    { REMIX_SYM_Toggles__entry_random_music_bonus, 26 },
    { REMIX_SYM_Toggles__entry_random_music_credits, 39 },
};

/* BGM.asm random_music_ current_track as the port keeps it: the index of the last random pick. */
static s32 sRandomTrackIndex = 0;

/**
 * Collects the BGM ids whose Random Music toggle is on.
 *
 * @param bgm_ids Output array with room for every entry of sRandomTracks.
 * @return Number of ids written.
 */
static s32 grRemixRandomMusicBuildList(u32 *bgm_ids)
{
    s32 count = 0;
    s32 index;

    for (index = 0; index < ARRAY_COUNT(sRandomTracks); index++)
    {
        if (remix_toggle_get(sRandomTracks[index].entry_addr) != 0)
        {
            bgm_ids[count++] = sRandomTracks[index].bgm_id;
        }
    }
    return count;
}

/**
 * Picks the index into the list: the last pick when a salty runback preserves the song, otherwise a new random index.
 * A preserved index beyond the current list length is rolled again, since the list changes with the toggles.
 *
 * @param count Number of tracks in the list, at least 1.
 * @param salty_runback TRUE when the match is a salty runback.
 * @return Index of the track to play.
 */
static s32 grRemixRandomMusicPickIndex(s32 count, sb32 salty_runback)
{
    sb32 preserve = (gmRemixToggle(preserve_salty_song) != 0) && salty_runback;

    if (preserve && (sRandomTrackIndex < count))
    {
        return sRandomTrackIndex;
    }
    sRandomTrackIndex = syUtilsRandIntRange(count);
    return sRandomTrackIndex;
}

u32 grRemixRandomMusicPick(u32 default_bgm, sb32 salty_runback)
{
    u32 bgm_ids[ARRAY_COUNT(sRandomTracks)];
    s32 count;

    if (gmRemixToggle(random_music) == 0)
    {
        return default_bgm;
    }
    count = grRemixRandomMusicBuildList(bgm_ids);
    if (count == 0)
    {
        return default_bgm;
    }
    return bgm_ids[grRemixRandomMusicPickIndex(count, salty_runback)];
}

u32 grRemixRandomMusicPickForced(u32 default_bgm)
{
    u32 bgm_ids[ARRAY_COUNT(sRandomTracks)];
    s32 count = grRemixRandomMusicBuildList(bgm_ids);

    if (count == 0)
    {
        return default_bgm;
    }
    return bgm_ids[syUtilsRandIntRange(count)];
}

#endif
