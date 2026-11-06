#include <mn/mncommon/mnremixgalleryrules.h>

#if defined(PORT) && defined(REGION_REMIX)
#include "remix/remix_roster_data.h"

typedef struct MNRemixVictoryImage
{
    int character_id;
    int bottom_file_id;

} MNRemixVictoryImage;

/* Gallery.asm:445-487 id_table, in page order. */
static const int sMNRemixGalleryCharacterIds[MN_REMIX_GALLERY_ENTRY_COUNT] =
{
    REMIX_ID_MARIO,
    REMIX_ID_DONKEY,
    REMIX_ID_LINK,
    REMIX_ID_SAMUS,
    REMIX_ID_YOSHI,
    REMIX_ID_KIRBY,
    REMIX_ID_FOX,
    REMIX_ID_PIKACHU,
    REMIX_ID_LUIGI,
    REMIX_ID_CAPTAIN,
    REMIX_ID_NESS,
    REMIX_ID_JIGGLY,
    REMIX_ID_GND,
    REMIX_ID_YLINK,
    REMIX_ID_DRM,
    REMIX_ID_FALCO,
    REMIX_ID_WARIO,
    REMIX_ID_BOWSER,
    REMIX_ID_WOLF,
    REMIX_ID_CONKER,
    REMIX_ID_MTWO,
    REMIX_ID_MARTH,
    REMIX_ID_SONIC,
    REMIX_ID_SHEIK,
    REMIX_ID_MARINA,
    REMIX_ID_DEDEDE,
    REMIX_ID_GOEMON,
    REMIX_ID_BANJO,
    REMIX_ID_CRASH,
    REMIX_ID_PEACH,
    REMIX_ID_DSAMUS,
    REMIX_ID_LUCAS,
    REMIX_ID_SLIPPY,
    REMIX_ID_ROY,
    REMIX_ID_DRL,
    REMIX_ID_LANKY,
    REMIX_ID_DRAGONKING,
    REMIX_ID_EBI,
    REMIX_ID_PIANO,
    REMIX_ID_BOSS
};

/* SinglePlayer.asm:2303-2319 custom_victory_file_table (Boss to Giant Donkey Kong) and the set_ending_image calls of
 * SinglePlayer.asm:2337-2378 and of the character files (the last call of a character wins: Lanky). Characters without
 * a row use MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE (replace_victory_image_ beqzl). */
static const MNRemixVictoryImage sMNRemixVictoryImages[] =
{
    { REMIX_ID_BOSS, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NONE, MN_REMIX_SPLASH_IMAGE_FILE },
    { REMIX_ID_METAL, 0xBA },
    { REMIX_ID_NMARIO, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NFOX, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NDONKEY, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NSAMUS, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NLUIGI, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NLINK, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NYOSHI, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NCAPTAIN, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NKIRBY, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NPIKACHU, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NJIGGLY, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_NNESS, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_GDONKEY, 0xB8 },
    { REMIX_ID_FALCO, 0xF1A },
    { REMIX_ID_GND, 0xF06 },
    { REMIX_ID_YLINK, 0xF10 },
    { REMIX_ID_DRM, 0xF18 },
    { REMIX_ID_WARIO, 0xF0C },
    { REMIX_ID_DSAMUS, 0xF14 },
    { REMIX_ID_ELINK, 0xB2 },
    { REMIX_ID_JSAMUS, 0xB0 },
    { REMIX_ID_JNESS, 0xC0 },
    { REMIX_ID_LUCAS, 0xF16 },
    { REMIX_ID_JLINK, 0xB2 },
    { REMIX_ID_JFALCON, 0xB6 },
    { REMIX_ID_JFOX, 0xBE },
    { REMIX_ID_JMARIO, 0xBA },
    { REMIX_ID_JLUIGI, 0xBC },
    { REMIX_ID_JDK, 0xB8 },
    { REMIX_ID_EPIKA, 0xAE },
    { REMIX_ID_JPUFF, 0xB4 },
    { REMIX_ID_EPUFF, 0xB4 },
    { REMIX_ID_JKIRBY, 0xAA },
    { REMIX_ID_JYOSHI, 0xAC },
    { REMIX_ID_JPIKA, 0xAE },
    { REMIX_ID_ESAMUS, 0xB0 },
    { REMIX_ID_BOWSER, 0xF02 },
    { REMIX_ID_GBOWSER, 0xF02 },
    { REMIX_ID_PIANO, 0x1523 },
    { REMIX_ID_WOLF, 0xF12 },
    { REMIX_ID_CONKER, 0xF04 },
    { REMIX_ID_MTWO, 0xEE8 },
    { REMIX_ID_MARTH, 0xF08 },
    { REMIX_ID_SONIC, 0xF0E },
    { REMIX_ID_SANDBAG, MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE },
    { REMIX_ID_SSONIC, 0xF0E },
    { REMIX_ID_SHEIK, 0xF0A },
    { REMIX_ID_MARINA, 0x1045 },
    { REMIX_ID_DEDEDE, 0x10BE },
    { REMIX_ID_GOEMON, 0x1159 },
    { REMIX_ID_PEPPY, 0x1176 },
    { REMIX_ID_SLIPPY, 0x1176 },
    { REMIX_ID_BANJO, 0x12C7 },
    { REMIX_ID_MLUIGI, 0xBC },
    { REMIX_ID_EBI, 0x12C9 },
    { REMIX_ID_DRL, 0x151F },
    { REMIX_ID_CRASH, 0x13FC },
    { REMIX_ID_LANKY, 0x152F },
    { REMIX_ID_ROY, 0x1521 },
    { REMIX_ID_DRAGONKING, 0x153C },
    { REMIX_ID_PEACH, 0x13FA },
};

/**
 * Character.id shown by a Gallery page (Gallery.asm id_table).
 *
 * @param index Page index, 0 to MN_REMIX_GALLERY_ENTRY_COUNT - 1.
 * @return The Character.id, or -1 when index is out of range.
 */
int mnRemixGalleryRulesGetCharacterId(int index)
{
    if ((index < 0) || (index >= MN_REMIX_GALLERY_ENTRY_COUNT))
    {
        return -1;
    }
    return sMNRemixGalleryCharacterIds[index];
}

/**
 * Page reached by moving through the Gallery (Gallery.asm _cycle_image): it wraps at both ends.
 *
 * @param index Current page index.
 * @param step Pages to move, negative to go back.
 * @return The new page index.
 */
int mnRemixGalleryRulesStepIndex(int index, int step)
{
    return (((index + step) % MN_REMIX_GALLERY_ENTRY_COUNT) + MN_REMIX_GALLERY_ENTRY_COUNT) % MN_REMIX_GALLERY_ENTRY_COUNT;
}

int mnRemixGalleryRulesNextIdleMode(int idle_mode)
{
    return (idle_mode == MN_REMIX_GALLERY_IDLE_SHUFFLE) ? MN_REMIX_GALLERY_IDLE_RANDOM_SONG : MN_REMIX_GALLERY_IDLE_SHUFFLE;
}

int mnRemixGalleryRulesNextIdleIndex(int idle_index)
{
    if ((idle_index + 1) >= MN_REMIX_GALLERY_ENTRY_COUNT)
    {
        return 0;
    }
    return idle_index + 1;
}

void mnRemixGalleryRulesBuildIdleOrder(int *order, int (*random_range)(int range))
{
    int page;
    int position;

    for (position = 0; position < MN_REMIX_GALLERY_ENTRY_COUNT; position++)
    {
        order[position] = -1;
    }
    for (page = 0; page < MN_REMIX_GALLERY_ENTRY_COUNT; page++)
    {
        do
        {
            position = random_range(MN_REMIX_GALLERY_ENTRY_COUNT);
        } while ((position < 0) || (position >= MN_REMIX_GALLERY_ENTRY_COUNT) || (order[position] >= 0));
        order[position] = page;
    }
}

/**
 * Bottom file of the victory picture of a Remix character (SinglePlayer.asm replace_victory_image_ _custom and the
 * set_ending_image calls); the top file is the next one and both use MN_REMIX_VICTORY_IMAGE_OFFSET.
 *
 * @param character_id Character.id, MN_REMIX_GALLERY_VANILLA_CHARACTER_COUNT or above.
 * @return The bottom file id, or 0 when character_id uses the vanilla picture table.
 */
int mnRemixGalleryRulesGetVictoryFileId(int character_id)
{
    int i;

    if (character_id < MN_REMIX_GALLERY_VANILLA_CHARACTER_COUNT)
    {
        return 0;
    }
    for (i = 0; i < (int)(sizeof(sMNRemixVictoryImages) / sizeof(sMNRemixVictoryImages[0])); i++)
    {
        if (sMNRemixVictoryImages[i].character_id == character_id)
        {
            return sMNRemixVictoryImages[i].bottom_file_id;
        }
    }
    return MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE;
}
#endif
