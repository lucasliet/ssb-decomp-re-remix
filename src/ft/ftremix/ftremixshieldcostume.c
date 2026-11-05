/**
 * ftremixshieldcostume.c - Smash Remix Shield option Costume: Character.costume_shield_color (Character.asm 2793).
 *
 * Each character has a byte array of Shield.color indices, one per costume. The vanilla rows are the literal data of
 * Character.asm, laid out as one block exactly like the asm, so an index past the end of a row reads the first bytes of the
 * next row as the asm does. Every Remix character copies the row of its parent (Character.asm 342) unless its own file
 * calls Character.set_costume_shield_colors (Piano, Slippy, Dr. Luigi) or Character.polygon_setup (Remix polygons).
 *
 * Classic Sonic (Sonic.classic_table) adds 6 to the costume id in Shield.asm color_fix_ (ftRemixSonicClassicShieldCostumeOffset),
 * so a classic Sonic reads past the end of the Fox row it shares, into the next rows, like the asm.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_roster_data.h"
#include "fighter_registry.h"
#include "ft/ftremix/sonic/ftremixsonicclassic.h"

#define FTREMIX_SHIELD_NO_ROW (-1)
#define FTREMIX_SHIELD_OWN_ROW_SIZE 8

/* Character.costume_shield_color rows MARIO to POLYGON, in the order and with the offsets of the asm labels. */
#define FTREMIX_SHIELD_ROW_MARIO 0
#define FTREMIX_SHIELD_ROW_FOX 8
#define FTREMIX_SHIELD_ROW_DONKEY 16
#define FTREMIX_SHIELD_ROW_SAMUS 24
#define FTREMIX_SHIELD_ROW_LUIGI 32
#define FTREMIX_SHIELD_ROW_LINK 40
#define FTREMIX_SHIELD_ROW_YOSHI 47
#define FTREMIX_SHIELD_ROW_CAPTAIN 57
#define FTREMIX_SHIELD_ROW_KIRBY 65
#define FTREMIX_SHIELD_ROW_PIKACHU 72
#define FTREMIX_SHIELD_ROW_JIGGLYPUFF 82
#define FTREMIX_SHIELD_ROW_NESS 92
#define FTREMIX_SHIELD_ROW_METAL 99
#define FTREMIX_SHIELD_ROW_POLYGON 105

static const u8 sFTRemixShieldVanillaRows[] =
{
    0x01, 0x03, 0x0D, 0x09, 0x05, 0x0F, 0x0D, 0x03,             /* MARIO */
    0x0D, 0x02, 0x0A, 0x06, 0x0E, 0x07, 0x03, 0x0C,             /* FOX */
    0x0D, 0x0E, 0x01, 0x09, 0x06, 0x0C, 0x0F, 0x03,             /* DONKEY */
    0x02, 0x0C, 0x0E, 0x04, 0x09, 0x0F, 0x07, 0x03,             /* SAMUS */
    0x05, 0x0F, 0x07, 0x0C, 0x0A, 0x03, 0x03, 0x02,             /* LUIGI */
    0x05, 0x0F, 0x01, 0x08, 0x0E, 0x03, 0x0C,                   /* LINK */
    0x05, 0x01, 0x07, 0x03, 0x0C, 0x09, 0x0E, 0x0F, 0x02, 0x0A, /* YOSHI */
    0x01, 0x0A, 0x04, 0x0C, 0x0E, 0x09, 0x02, 0x0F,             /* CAPTAIN */
    0x0C, 0x03, 0x07, 0x01, 0x05, 0x0F, 0x0A,                   /* KIRBY */
    0x03, 0x01, 0x09, 0x05, 0x06, 0x06, 0x02, 0x04, 0x0A, 0x0C, /* PIKACHU */
    0x0D, 0x01, 0x09, 0x05, 0x06, 0x04, 0x02, 0x0A, 0x0E, 0x02, /* JIGGLYPUFF */
    0x01, 0x03, 0x09, 0x05, 0x0F, 0x0A, 0x07,                   /* NESS */
    0x0E, 0x01, 0x05, 0x09, 0x03, 0x02,                         /* METAL */
    0x0A, 0x01, 0x06, 0x09, 0x0E, 0x0F, 0x03                    /* POLYGON */
};

/* Character.polygon_setup: PURPLE, RED, TURQUOISE, BLUE, BLACK, WHITE, YELLOW, then the zero fill of OS.align(4). */
static const u8 sFTRemixShieldPolygonRow[] = { 0x0A, 0x01, 0x06, 0x09, 0x0E, 0x0F, 0x03, 0x00 };

/* Piano.asm 373: BLACK, RED, YELLOW, CYAN, GREEN, WHITE, then the zero fill of OS.align(4). */
static const u8 sFTRemixShieldPianoRow[] = { 0x0E, 0x01, 0x03, 0x07, 0x05, 0x0F, 0x00, 0x00 };

/* Slippy.asm 298: WHITE, RED, BLUE, TURQUOISE, PURPLE, ORANGE, YELLOW, then the zero fill of OS.align(4). */
static const u8 sFTRemixShieldSlippyRow[] = { 0x0F, 0x01, 0x09, 0x06, 0x0A, 0x02, 0x03, 0x00 };

/* DrLuigi.asm 136: WHITE, CYAN, PINK, YELLOW, ORANGE, BLACK, GREEN, PURPLE. */
static const u8 sFTRemixShieldDrLuigiRow[] = { 0x0F, 0x07, 0x0C, 0x03, 0x02, 0x0E, 0x05, 0x0A };

/** Offset in sFTRemixShieldVanillaRows of the row the vanilla table gives a vanilla fighter kind, or FTREMIX_SHIELD_NO_ROW. */
static s32 ftRemixShieldGetVanillaRowOffset(s32 fkind)
{
    switch (fkind)
    {
    case REMIX_ID_MARIO: return FTREMIX_SHIELD_ROW_MARIO;
    case REMIX_ID_FOX: return FTREMIX_SHIELD_ROW_FOX;
    case REMIX_ID_DONKEY:
    case REMIX_ID_GDONKEY: return FTREMIX_SHIELD_ROW_DONKEY;
    case REMIX_ID_SAMUS: return FTREMIX_SHIELD_ROW_SAMUS;
    case REMIX_ID_LUIGI: return FTREMIX_SHIELD_ROW_LUIGI;
    case REMIX_ID_LINK: return FTREMIX_SHIELD_ROW_LINK;
    case REMIX_ID_YOSHI: return FTREMIX_SHIELD_ROW_YOSHI;
    case REMIX_ID_CAPTAIN: return FTREMIX_SHIELD_ROW_CAPTAIN;
    case REMIX_ID_KIRBY: return FTREMIX_SHIELD_ROW_KIRBY;
    case REMIX_ID_PIKACHU: return FTREMIX_SHIELD_ROW_PIKACHU;
    case REMIX_ID_JIGGLY: return FTREMIX_SHIELD_ROW_JIGGLYPUFF;
    case REMIX_ID_NESS: return FTREMIX_SHIELD_ROW_NESS;
    case REMIX_ID_METAL: return FTREMIX_SHIELD_ROW_METAL;
    default: break;
    }
    if ((fkind >= REMIX_ID_NMARIO) && (fkind <= REMIX_ID_NNESS))
    {
        return FTREMIX_SHIELD_ROW_POLYGON;
    }
    return FTREMIX_SHIELD_NO_ROW;
}

/** Row of an added Remix character that is not a copy of its parent, or NULL. */
static const u8* ftRemixShieldGetOwnRow(s32 fkind)
{
    if ((fkind >= REMIX_ID_NWARIO) && (fkind <= REMIX_ID_NCRASH))
    {
        return sFTRemixShieldPolygonRow;
    }
    switch (fkind)
    {
    case REMIX_ID_PIANO: return sFTRemixShieldPianoRow;
    case REMIX_ID_SLIPPY: return sFTRemixShieldSlippyRow;
    case REMIX_ID_DRL: return sFTRemixShieldDrLuigiRow;
    default: return NULL;
    }
}

/**
 * Shield.color index of the costume of a fighter (Shield.asm color_fix_ _costume branch), or -1 when the character has no row
 * or the costume is past the end of the table.
 */
s32 ftRemixShieldGetCostumeColorIndex(const FTStruct *fp)
{
    const u8 *own_row = ftRemixShieldGetOwnRow(fp->fkind);
    s32 row_offset;
    s32 index;

    if (own_row != NULL)
    {
        return (fp->costume < FTREMIX_SHIELD_OWN_ROW_SIZE) ? own_row[fp->costume] : -1;
    }
    row_offset = ftRemixShieldGetVanillaRowOffset(port_fighter_parent_fkind(fp->fkind));
    if (row_offset == FTREMIX_SHIELD_NO_ROW)
    {
        return -1;
    }
    index = row_offset + fp->costume + ftRemixSonicClassicShieldCostumeOffset(fp);

    if (index >= (s32)sizeof(sFTRemixShieldVanillaRows))
    {
        return -1;
    }
    return sFTRemixShieldVanillaRows[index];
}

#endif
