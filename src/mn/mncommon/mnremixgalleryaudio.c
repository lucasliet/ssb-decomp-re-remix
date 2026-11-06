/**
 * mnremixgalleryaudio.c - song lists, picture of a song and drum kit of the Remix Gallery, see mnremixgalleryaudio.h.
 *
 * The ids come from Gallery.asm (bgm_<character> arrays, add_bgm_to_gallery calls, fgm_drum_kit) with each {MIDI.id.<NAME>}
 * resolved to its position in the music table of docs/remix/audio.md; the BGM.stage / BGM.menu constants are BGM.asm values.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/mncommon/mnremixgalleryaudio.h>
#include <mn/mncommon/mnremixgalleryrules.h>

#define MN_REMIX_GALLERY_ARRAY_COUNT(array) ((int)(sizeof(array) / sizeof((array)[0])))
#define MN_REMIX_GALLERY_SONG_LIST(array) { array, MN_REMIX_GALLERY_ARRAY_COUNT(array) }

/* Page the tracks without a character of their own show (Gallery.asm id.REMIX). */
#define MN_REMIX_GALLERY_PAGE_REMIX 39

typedef struct MNRemixGallerySongList
{
    const unsigned short *songs;
    int count;

} MNRemixGallerySongList;

typedef struct MNRemixGalleryBgmPage
{
    int bgm_id;
    int page;

} MNRemixGalleryBgmPage;

typedef struct MNRemixGalleryDrumKit
{
    int left_fgm;
    int right_fgm;

} MNRemixGalleryDrumKit;

static const unsigned short sSongsMario[] = { 6, 2, 315, 78, 86, 212, 233, 264, 136, 132, 291, 335 };
static const unsigned short sSongsDonkey[] = { 5, 322, 274, 128, 114, 159, 268, 91 };
static const unsigned short sSongsLink[] = { 9, 102, 119, 204 };
static const unsigned short sSongsSamus[] = { 1, 84, 234 };
static const unsigned short sSongsYoshi[] = { 8, 169, 120, 295, 168, 181, 118 };
static const unsigned short sSongsKirby[] = { 0, 72, 355, 171, 337, 96, 286 };
static const unsigned short sSongsFox[] = { 4, 149, 336 };
static const unsigned short sSongsPikachu[] = { 7, 75, 303, 343 };
static const unsigned short sSongsLuigi[] = { 211, 235, 203, 160, 263, 201, 202, 180, 197, 248 };
static const unsigned short sSongsCaptain[] = { 64, 81, 122, 334, 293, 93 };
static const unsigned short sSongsNess[] = { 83, 103, 112, 364, 301, 314 };
static const unsigned short sSongsJigglypuff[] = { 51, 173, 276, 284 };
static const unsigned short sSongsGanondorf[] = { 47, 258, 353, 288, 230 };
static const unsigned short sSongsYoungLink[] = { 49, 57, 130, 285 };
static const unsigned short sSongsDrMario[] = { 50, 266, 307 };
static const unsigned short sSongsFalco[] = { 48, 143, 329 };
static const unsigned short sSongsWario[] = { 225, 89 };
static const unsigned short sSongsBowser[] = { 74, 76, 77, 137, 133, 347, 255, 196 };
static const unsigned short sSongsWolf[] = { 97, 144 };
static const unsigned short sSongsConker[] = { 271, 148, 155, 147, 200 };
static const unsigned short sSongsMewtwo[] = { 94, 100, 88, 178 };
static const unsigned short sSongsMarth[] = { 177, 273, 102 };
static const unsigned short sSongsSonic[] = { 194, 191, 195, 186, 184, 188, 193, 198, 189, 182, 190, 199, 192 };
static const unsigned short sSongsSheik[] = { 236, 95 };
static const unsigned short sSongsMarina[] = { 223, 153, 154 };
static const unsigned short sSongsDedede[] = { 214, 298, 59, 247, 221, 338, 354 };
static const unsigned short sSongsGoemon[] = { 227, 242, 299 };
static const unsigned short sSongsBanjo[] = { 124, 62, 368, 66, 145, 134, 80, 370 };
static const unsigned short sSongsCrash[] = { 317, 318, 325, 359, 340, 360 };
static const unsigned short sSongsPeach[] = { 323, 259, 245, 55, 175, 63, 87 };
static const unsigned short sSongsDarkSamus[] = { 73, 287, 344 };
static const unsigned short sSongsLucas[] = { 115, 106, 105, 104, 121, 356 };
static const unsigned short sSongsSlippyPeppy[] = { 246, 237 };
static const unsigned short sSongsRoy[] = { 167, 358, 102 };
static const unsigned short sSongsDrLuigi[] = { 282 };
static const unsigned short sSongsLanky[] = { 331, 330, 158, 91 };
static const unsigned short sSongsDragonKing[] = { 213, 54, 126, 174, 127, 129 };
static const unsigned short sSongsEbi[] = { 232 };
static const unsigned short sSongsPiano[] = { 277, 256 };
static const unsigned short sSongsRemix[] = { 39, 37, 25, 36, 26, 22, 43, 34, 310, 281, 142 };

/* Gallery.asm bgm_table, in page order (id_table). */
static const MNRemixGallerySongList sSongLists[MN_REMIX_GALLERY_ENTRY_COUNT] =
{
    MN_REMIX_GALLERY_SONG_LIST(sSongsMario),
    MN_REMIX_GALLERY_SONG_LIST(sSongsDonkey),
    MN_REMIX_GALLERY_SONG_LIST(sSongsLink),
    MN_REMIX_GALLERY_SONG_LIST(sSongsSamus),
    MN_REMIX_GALLERY_SONG_LIST(sSongsYoshi),
    MN_REMIX_GALLERY_SONG_LIST(sSongsKirby),
    MN_REMIX_GALLERY_SONG_LIST(sSongsFox),
    MN_REMIX_GALLERY_SONG_LIST(sSongsPikachu),
    MN_REMIX_GALLERY_SONG_LIST(sSongsLuigi),
    MN_REMIX_GALLERY_SONG_LIST(sSongsCaptain),
    MN_REMIX_GALLERY_SONG_LIST(sSongsNess),
    MN_REMIX_GALLERY_SONG_LIST(sSongsJigglypuff),
    MN_REMIX_GALLERY_SONG_LIST(sSongsGanondorf),
    MN_REMIX_GALLERY_SONG_LIST(sSongsYoungLink),
    MN_REMIX_GALLERY_SONG_LIST(sSongsDrMario),
    MN_REMIX_GALLERY_SONG_LIST(sSongsFalco),
    MN_REMIX_GALLERY_SONG_LIST(sSongsWario),
    MN_REMIX_GALLERY_SONG_LIST(sSongsBowser),
    MN_REMIX_GALLERY_SONG_LIST(sSongsWolf),
    MN_REMIX_GALLERY_SONG_LIST(sSongsConker),
    MN_REMIX_GALLERY_SONG_LIST(sSongsMewtwo),
    MN_REMIX_GALLERY_SONG_LIST(sSongsMarth),
    MN_REMIX_GALLERY_SONG_LIST(sSongsSonic),
    MN_REMIX_GALLERY_SONG_LIST(sSongsSheik),
    MN_REMIX_GALLERY_SONG_LIST(sSongsMarina),
    MN_REMIX_GALLERY_SONG_LIST(sSongsDedede),
    MN_REMIX_GALLERY_SONG_LIST(sSongsGoemon),
    MN_REMIX_GALLERY_SONG_LIST(sSongsBanjo),
    MN_REMIX_GALLERY_SONG_LIST(sSongsCrash),
    MN_REMIX_GALLERY_SONG_LIST(sSongsPeach),
    MN_REMIX_GALLERY_SONG_LIST(sSongsDarkSamus),
    MN_REMIX_GALLERY_SONG_LIST(sSongsLucas),
    MN_REMIX_GALLERY_SONG_LIST(sSongsSlippyPeppy),
    MN_REMIX_GALLERY_SONG_LIST(sSongsRoy),
    MN_REMIX_GALLERY_SONG_LIST(sSongsDrLuigi),
    MN_REMIX_GALLERY_SONG_LIST(sSongsLanky),
    MN_REMIX_GALLERY_SONG_LIST(sSongsDragonKing),
    MN_REMIX_GALLERY_SONG_LIST(sSongsEbi),
    MN_REMIX_GALLERY_SONG_LIST(sSongsPiano),
    MN_REMIX_GALLERY_SONG_LIST(sSongsRemix),
};

/* Gallery.asm bgm_to_gallery_table: the stage tracks 0 to 9, then the add_bgm_to_gallery calls. Every other id shows the Remix page. */
static const MNRemixGalleryBgmPage sBgmPages[] =
{
    { 0, 5 }, { 1, 3 }, { 2, 8 }, { 3, 8 }, { 4, 6 }, { 5, 1 }, { 6, 0 }, { 7, 7 }, { 8, 4 }, { 9, 2 },
    { 47, 12 }, { 48, 15 }, { 49, 13 }, { 50, 14 }, { 51, 11 }, { 53, 16 }, { 55, 0 }, { 56, 23 }, { 57, 13 }, { 59, 25 },
    { 60, 0 }, { 62, 27 }, { 63, 0 }, { 64, 9 }, { 66, 27 }, { 72, 5 }, { 73, 30 }, { 74, 17 }, { 75, 7 }, { 76, 17 },
    { 77, 17 }, { 78, 0 }, { 79, 0 }, { 80, 27 }, { 81, 9 }, { 83, 10 }, { 84, 3 }, { 86, 0 }, { 87, 0 }, { 88, 20 },
    { 89, 16 }, { 274, 1 }, { 93, 9 }, { 94, 20 }, { 95, 23 }, { 96, 5 }, { 97, 18 }, { 98, 16 }, { 100, 20 }, { 102, 2 },
    { 103, 10 }, { 104, 31 }, { 105, 31 }, { 106, 31 }, { 112, 10 }, { 114, 1 }, { 115, 31 }, { 118, 4 }, { 119, 2 },
    { 120, 4 }, { 121, 31 }, { 122, 9 }, { 123, 0 }, { 124, 27 }, { 128, 1 }, { 130, 13 }, { 132, 0 }, { 133, 17 },
    { 134, 27 }, { 135, 1 }, { 136, 0 }, { 137, 17 }, { 143, 15 }, { 144, 18 }, { 145, 27 }, { 147, 19 }, { 148, 19 },
    { 149, 6 }, { 153, 24 }, { 154, 24 }, { 155, 19 }, { 157, 0 }, { 158, 35 }, { 159, 1 }, { 160, 8 }, { 167, 33 },
    { 168, 4 }, { 169, 4 }, { 170, 4 }, { 171, 5 }, { 173, 11 }, { 175, 0 }, { 177, 21 }, { 178, 20 }, { 179, 8 },
    { 180, 8 }, { 181, 4 }, { 182, 22 }, { 183, 22 }, { 184, 22 }, { 186, 22 }, { 188, 22 }, { 189, 22 }, { 190, 22 },
    { 191, 22 }, { 192, 22 }, { 193, 22 }, { 194, 22 }, { 195, 22 }, { 196, 4 }, { 197, 8 }, { 198, 22 }, { 199, 22 },
    { 200, 19 }, { 201, 8 }, { 202, 8 }, { 203, 8 }, { 204, 13 }, { 206, 25 }, { 207, 8 }, { 211, 8 }, { 212, 0 },
    { 214, 25 }, { 218, 16 }, { 220, 5 }, { 221, 25 }, { 223, 24 }, { 225, 16 }, { 227, 26 }, { 228, 17 }, { 230, 12 },
    { 232, 37 }, { 233, 0 }, { 234, 3 }, { 235, 8 }, { 236, 23 }, { 237, 32 }, { 242, 26 }, { 245, 0 }, { 246, 32 },
    { 247, 25 }, { 248, 8 }, { 255, 17 }, { 256, 38 }, { 258, 12 }, { 263, 8 }, { 264, 0 }, { 265, 37 }, { 266, 14 },
    { 268, 1 }, { 273, 21 }, { 91, 1 }, { 276, 11 }, { 277, 38 }, { 281, 39 }, { 285, 13 }, { 286, 5 }, { 287, 30 },
    { 288, 12 }, { 290, 5 }, { 282, 34 }, { 291, 0 }, { 293, 9 }, { 295, 4 }, { 298, 25 }, { 299, 26 }, { 301, 10 },
    { 303, 7 }, { 304, 16 }, { 307, 14 }, { 85, 27 }, { 138, 27 }, { 139, 19 }, { 306, 19 }, { 284, 11 }, { 314, 10 },
    { 315, 8 }, { 322, 1 }, { 317, 28 }, { 318, 28 }, { 323, 29 }, { 325, 28 }, { 329, 15 }, { 330, 35 }, { 331, 35 },
    { 334, 9 }, { 335, 0 }, { 336, 6 }, { 337, 5 }, { 338, 25 }, { 340, 28 }, { 342, 0 }, { 343, 7 }, { 344, 30 },
    { 347, 17 }, { 352, 8 }, { 353, 12 }, { 354, 25 }, { 355, 5 }, { 356, 31 }, { 358, 33 }, { 359, 28 }, { 360, 28 },
    { 364, 10 }, { 368, 27 }, { 370, 27 }, { 54, 36 }, { 126, 36 }, { 174, 36 }, { 127, 36 }, { 129, 36 },
};

/* Gallery.asm fgm_drum_kit (the commented F-Zero announcer pair is not part of the 18 entries). */
static const MNRemixGalleryDrumKit sDrumKits[MN_REMIX_GALLERY_DRUM_KIT_COUNT] =
{
    { 0x020, 0x01F }, { 0x11F, 0x033 }, { 0x117, 0x038 }, { 0x03D, 0x02F }, { 0x2C6, 0x2C7 }, { 0x59C, 0x59B },
    { 0x567, 0x568 }, { 0x0EE, 0x0F7 }, { 0x0D9, 0x1B0 }, { 0x569, 0x56A }, { 0x180, 0x181 }, { 0x256, 0x257 },
    { 0x230, 0x234 }, { 0x52B, 0x52C }, { 0x5A1, 0x583 }, { 0x523, 0x556 }, { 0x5F8, 0x5F9 }, { 0x547, 0x549 },
};

int mnRemixGalleryAudioGetSongCount(int page)
{
    if ((page < 0) || (page >= MN_REMIX_GALLERY_ENTRY_COUNT))
    {
        return 0;
    }
    return sSongLists[page].count;
}

int mnRemixGalleryAudioGetSong(int page, int song_number)
{
    if (song_number == 0)
    {
        return MN_REMIX_GALLERY_BGM_GALLERY;
    }
    if ((song_number < 0) || (song_number > mnRemixGalleryAudioGetSongCount(page)))
    {
        return -1;
    }
    return sSongLists[page].songs[song_number - 1];
}

int mnRemixGalleryAudioNextSongNumber(int page, int song_number)
{
    if (song_number >= mnRemixGalleryAudioGetSongCount(page))
    {
        return 1;
    }
    return song_number + 1;
}

int mnRemixGalleryAudioGetPageForBgm(int bgm_id)
{
    int i;

    for (i = 0; i < MN_REMIX_GALLERY_ARRAY_COUNT(sBgmPages); i++)
    {
        if (sBgmPages[i].bgm_id == bgm_id)
        {
            return sBgmPages[i].page;
        }
    }
    return MN_REMIX_GALLERY_PAGE_REMIX;
}

int mnRemixGalleryAudioGetDrumFgm(int kit_index, int is_right)
{
    if ((kit_index < 0) || (kit_index >= MN_REMIX_GALLERY_DRUM_KIT_COUNT))
    {
        return -1;
    }
    return (is_right != 0) ? sDrumKits[kit_index].right_fgm : sDrumKits[kit_index].left_fgm;
}

int mnRemixGalleryAudioNextDrumKit(int kit_index)
{
    if ((kit_index + 1) >= MN_REMIX_GALLERY_DRUM_KIT_COUNT)
    {
        return 0;
    }
    return kit_index + 1;
}

#endif
