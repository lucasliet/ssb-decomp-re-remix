/**
 * ifremixspriterules.c - pure table and colour rules of the Remix extension sprites, see ifremixspriterules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixspriterules.h"

/* Offsets of Render.file_c5_offsets (Render.asm:136) plus L_THIN; indexed by IFRemixSpriteId. */
static const IFRemixSpriteEntry sIFRemixSpriteEntries[IFREMIX_SPRITE_COUNT] =
{
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x0958 },
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x0A88 },
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x18C8 },
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x0CF8 },
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x0BD8 },
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x04D8 },
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x1B10 },
    { IFREMIX_SPRITE_FILE_BUTTONS, 0x19F8 },   /* the "Legend" word of the pause R legend (Pause.asm 182) */
    { IFREMIX_SPRITE_FILE_CSS_IMAGES, 0x0218 }, /* D-pad */
    { IFREMIX_SPRITE_FILE_CSS_IMAGES, 0x1958 }, /* the four C stick arrows of the camera legend (Pause.asm 326-332) */
    { IFREMIX_SPRITE_FILE_CSS_IMAGES, 0x0688 },
    { IFREMIX_SPRITE_FILE_CSS_IMAGES, 0x1A88 },
    { IFREMIX_SPRITE_FILE_CSS_IMAGES, 0x1BB8 },
    { IFREMIX_SPRITE_FILE_KOTH, 0x0AA8 },       /* the score meter frame (KingOfTheHill.asm setup_, image footer 0xAA8) */
    { IFREMIX_SPRITE_FILE_STOCK_ICONS, 0x0080 }, /* the polygon stock icon of the Multi-Man counter (SinglePlayerModes.asm 2312) */
    { IFREMIX_SPRITE_FILE_DIGITS, 0x0828 },      /* the "x" of the Multi-Man counter (llIFCommonDigitsCrossSprite) */
    { IFREMIX_SPRITE_FILE_SOUND_TEST, 0x1D50 },  /* the START button of the Gentlemen's Reset request (TwelveCharBattle.asm 3610) */
};

const IFRemixSpriteEntry *ifRemixSpriteRulesGetEntry(int id)
{
    if ((id < 0) || (id >= IFREMIX_SPRITE_COUNT))
    {
        return NULL;
    }
    return &sIFRemixSpriteEntries[id];
}

void ifRemixSpriteRulesUnpackColor(uint32_t rgba, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha)
{
    *red = (uint8_t)(rgba >> 24);
    *green = (uint8_t)(rgba >> 16);
    *blue = (uint8_t)(rgba >> 8);
    *alpha = (uint8_t)rgba;
}

#endif
