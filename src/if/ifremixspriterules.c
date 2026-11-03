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
