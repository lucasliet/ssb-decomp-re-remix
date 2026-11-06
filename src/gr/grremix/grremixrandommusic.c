/**
 * grremixrandommusic.c - Random Music list of the stage BGM (BGM.asm random_music_, add_song_to_random_list_).
 *
 * The custom MIDI tracks of Toggles.asm follow the 16 stage and menu tracks. Their toggles are a Menu.entry chain that starts
 * at entry_random_music_first_custom and is already sorted by title, so the list is built by walking that chain: the value
 * of each entry is its Random Music toggle and its word 0x24 is the BGM id (the one Toggles.asm preview_bgm_ plays).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "grremixrandommusic.h"
#include "gm/gmremixtoggle.h"
#include "remix/remix_ram_image.h"
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

/* Menu.asm entry: word 0x1C is the next entry (0 ends the chain), word 0x24 the extra value (the BGM id of a Random Music row). */
#define GRREMIX_MENU_ENTRY_NEXT 0x1C
#define GRREMIX_MENU_ENTRY_EXTRA 0x24

/* midi_count minus the 0x2F vanilla tracks: no chain can be longer, which also bounds a corrupt image. */
#define GRREMIX_RANDOM_CUSTOM_MAX 328
#define GRREMIX_RANDOM_LIST_MAX (ARRAY_COUNT(sRandomTracks) + GRREMIX_RANDOM_CUSTOM_MAX)

/* BGM.asm random_music_ current_track as the port keeps it: the index of the last random pick. */
static s32 sRandomTrackIndex = 0;

/**
 * Appends the BGM ids of the custom MIDI rows whose Random Music toggle is on, in the order of the entry chain.
 *
 * @param bgm_ids Output array.
 * @param count Number of ids already written.
 * @return Number of ids written after the append.
 */
static s32 grRemixRandomMusicAppendCustom(u32 *bgm_ids, s32 count)
{
    u32 entry_addr = REMIX_SYM_Toggles__entry_random_music_first_custom;
    s32 visited;

    for (visited = 0; (entry_addr != 0) && (visited < GRREMIX_RANDOM_CUSTOM_MAX); visited++)
    {
        if (remix_toggle_get(entry_addr) != 0)
        {
            bgm_ids[count++] = remix_ram_image_read_word(entry_addr + GRREMIX_MENU_ENTRY_EXTRA);
        }
        entry_addr = remix_ram_image_read_word(entry_addr + GRREMIX_MENU_ENTRY_NEXT);
    }
    return count;
}

/**
 * Collects the BGM ids whose Random Music toggle is on: the stage and menu tracks, then the custom MIDIs.
 *
 * @param bgm_ids Output array with room for GRREMIX_RANDOM_LIST_MAX ids.
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
    return grRemixRandomMusicAppendCustom(bgm_ids, count);
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
    u32 bgm_ids[GRREMIX_RANDOM_LIST_MAX];
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
    u32 bgm_ids[GRREMIX_RANDOM_LIST_MAX];
    s32 count = grRemixRandomMusicBuildList(bgm_ids);

    if (count == 0)
    {
        return default_bgm;
    }
    return bgm_ids[syUtilsRandIntRange(count)];
}

#endif
