/**
 * mnremixchardata.c - Smash Remix Character Data screen (0x1A) fighter order and the sprite references of the 18
 * extended fighters (src/CharacterDataScreen.asm: set_char_order and add_char_to_data_screen). The asm stores the
 * bio, works and name offsets with a top-byte flag that selects the Remix extended file, and adds 0x30, 0x10 and
 * 0x10 to them; the values below are the asm values before those addends.
 */

#ifdef REGION_REMIX

#include <ssb_types.h>
#include <macros.h>
#include <ft/ftdef.h>
#include "mn/mndata/mnremixchardata.h"

#define MNREMIX_CHARDATA_VANILLA_FIGHTER_COUNT (nFTKindNess + 1)
#define MNREMIX_CHARDATA_BIO_ADDEND 0x30
#define MNREMIX_CHARDATA_WORKS_ADDEND 0x10
#define MNREMIX_CHARDATA_NAME_ADDEND 0x10
#define MNREMIX_CHARDATA_FLAG_MASK 0xFF000000
#define MNREMIX_CHARDATA_OFFSET_MASK 0x00FFFFFF

typedef struct MNRemixCharDataEntry
{
    s32 fkind;
    u32 bio_value;
    u32 works_value;
    u32 name_value;
    f32 name_x;
    f32 name_y;
} MNRemixCharDataEntry;

static const s32 kMNRemixCharDataOrder[MNREMIX_CHARDATA_PAGE_COUNT] =
{
    0x00, 0x04, 0x49, 0x34, 0x20, 0x02, 0x21, 0x05, 0x1F, 0x3E,
    0x1E, 0x03, 0x06, 0x08, 0x40, 0x01, 0x1D, 0x37, 0x09, 0x0A,
    0x39, 0x07, 0x0B, 0x3A, 0x41, 0x3B, 0x48, 0x3F, 0x44, 0x38
};

static const MNRemixCharDataEntry kMNRemixCharDataEntries[] =
{
    { 0x1D, 0x80002408, 0x00026508, 0x8000F528, 33.0F, 50.0F }, /* FALCO */
    { 0x1E, 0x80004888, 0x80000A08, 0x80014920, 33.0F, 48.0F }, /* GND */
    { 0x1F, 0x80006D08, 0x80001468, 0x800155E0, 33.0F, 48.0F }, /* YLINK */
    { 0x20, 0x80009188, 0x80008688, 0x80013C60, 33.0F, 48.0F }, /* DRM */
    { 0x21, 0x8000B608, 0x80001EC8, 0x800122E8, 33.0F, 50.0F }, /* WARIO */
    { 0x34, 0x80012388, 0x80002928, 0x80016DB8, 24.0F, 50.0F }, /* BOWSER */
    { 0x37, 0x80014808, 0x80003388, 0x8000FA88, 33.0F, 50.0F }, /* WOLF */
    { 0x38, 0x80016C88, 0x80003DE8, 0x80015DC8, 26.0F, 50.0F }, /* CONKER */
    { 0x39, 0x80019108, 0x80004848, 0x80018F28, 24.0F, 50.0F }, /* MTWO */
    { 0x3A, 0x8001B588, 0x800052A8, 0x800107C8, 33.0F, 50.0F }, /* MARTH */
    { 0x3B, 0x8001DA08, 0x80005D08, 0x80011C20, 33.0F, 50.0F }, /* SONIC */
    { 0x3E, 0x8001FE88, 0x80000A08, 0x80011558, 33.0F, 50.0F }, /* SHEIK */
    { 0x3F, 0x80022308, 0x80006768, 0x80010E90, 26.0F, 48.0F }, /* MARINA */
    { 0x40, 0x80024788, 0x800071C8, 0x800165A8, 20.0F, 50.0F }, /* DEDEDE */
    { 0x41, 0x80026C08, 0x80007C28, 0x80018608, 23.0F, 50.0F }, /* GOEMON */
    { 0x44, 0x80029088, 0x8000A5A8, 0x8001A200, 20.0F, 48.0F }, /* BANJO */
    { 0x48, 0x8000FF08, 0x80009B48, 0x80010128, 33.0F, 50.0F }, /* CRASH */
    { 0x49, 0x8000DA88, 0x800090E8, 0x8001A8C0, 23.0F, 50.0F }  /* PEACH */
};

/** Fighter kind shown on a Character Data page, or nFTKindNull when the page is outside the Remix order. */
s32 mnRemixCharDataGetFighterKind(s32 page)
{
    if ((page < 0) || (page >= MNREMIX_CHARDATA_PAGE_COUNT))
    {
        return nFTKindNull;
    }
    return kMNRemixCharDataOrder[page];
}

/** Page of `fkind` in the Remix order, or 0 when the fighter is not on the screen. */
s32 mnRemixCharDataGetPage(s32 fkind)
{
    s32 page;

    for (page = 0; page < MNREMIX_CHARDATA_PAGE_COUNT; page++)
    {
        if (kMNRemixCharDataOrder[page] == fkind)
        {
            return page;
        }
    }
    return 0;
}

/** TRUE for the fighters beyond the vanilla twelve, whose sprites live in the Remix extended files. */
sb32 mnRemixCharDataIsExtended(s32 fkind)
{
    return fkind >= MNREMIX_CHARDATA_VANILLA_FIGHTER_COUNT;
}

/** The table entry of an extended fighter, or NULL when `fkind` has no Remix sprite data. */
static const MNRemixCharDataEntry* mnRemixCharDataFindEntry(s32 fkind)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(kMNRemixCharDataEntries); i++)
    {
        if (kMNRemixCharDataEntries[i].fkind == fkind)
        {
            return &kMNRemixCharDataEntries[i];
        }
    }
    return NULL;
}

/** Splits a raw asm offset into a file index and an offset: the flag selects the extended file, else file 0. */
static void mnRemixCharDataMakeRef(u32 value, u32 addend, s32 extended_file_index, MNRemixCharDataSpriteRef *ref)
{
    u32 sum = value + addend;

    if (sum & MNREMIX_CHARDATA_FLAG_MASK)
    {
        ref->file_index = extended_file_index;
    }
    else
    {
        ref->file_index = 0;
    }
    ref->offset = sum & MNREMIX_CHARDATA_OFFSET_MASK;
}

/** Writes the file index and offset of a sprite of an extended fighter; FALSE when the fighter has no entry. */
sb32 mnRemixCharDataGetSpriteRef(s32 fkind, s32 sprite_kind, MNRemixCharDataSpriteRef *ref)
{
    const MNRemixCharDataEntry *entry = mnRemixCharDataFindEntry(fkind);

    if (entry == NULL)
    {
        return FALSE;
    }
    switch (sprite_kind)
    {
        case MNREMIX_CHARDATA_SPRITE_BIO:
            mnRemixCharDataMakeRef(entry->bio_value, MNREMIX_CHARDATA_BIO_ADDEND, MNREMIX_CHARDATA_BIOS_FILE_INDEX, ref);
            return TRUE;
        case MNREMIX_CHARDATA_SPRITE_WORKS:
            mnRemixCharDataMakeRef(entry->works_value, MNREMIX_CHARDATA_WORKS_ADDEND, MNREMIX_CHARDATA_IMAGES_FILE_INDEX, ref);
            return TRUE;
        case MNREMIX_CHARDATA_SPRITE_NAME:
            mnRemixCharDataMakeRef(entry->name_value, MNREMIX_CHARDATA_NAME_ADDEND, MNREMIX_CHARDATA_IMAGES_FILE_INDEX, ref);
            return TRUE;
        default:
            return FALSE;
    }
}

/** Name sprite x of an extended fighter, 0 when the fighter has no entry. */
f32 mnRemixCharDataGetNameX(s32 fkind)
{
    const MNRemixCharDataEntry *entry = mnRemixCharDataFindEntry(fkind);

    if (entry == NULL)
    {
        return 0.0F;
    }
    return entry->name_x;
}

/** Name sprite y of an extended fighter, 0 when the fighter has no entry. */
f32 mnRemixCharDataGetNameY(s32 fkind)
{
    const MNRemixCharDataEntry *entry = mnRemixCharDataFindEntry(fkind);

    if (entry == NULL)
    {
        return 0.0F;
    }
    return entry->name_y;
}

#endif
