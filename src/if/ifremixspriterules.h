#ifndef _IFREMIXSPRITERULES_H_
#define _IFREMIXSPRITERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <stddef.h>
#include <stdint.h>

/*
 * Table of the Remix button and extension sprites (docs/remix/notes/f15_button_sprites.md), pure so the headless unit
 * tests (tests/remix) can check it. Each entry names the reloc file that holds the sprite and the offset of its Sprite
 * header in that file (Render.file_c5_offsets in the asm).
 */
typedef enum IFRemixSpriteId
{
    IFREMIX_SPRITE_BUTTON_A,
    IFREMIX_SPRITE_BUTTON_B,
    IFREMIX_SPRITE_BUTTON_L,
    IFREMIX_SPRITE_BUTTON_R,
    IFREMIX_SPRITE_BUTTON_Z,
    IFREMIX_SPRITE_BUTTON_PLUS,
    IFREMIX_SPRITE_BUTTON_L_THIN,
    IFREMIX_SPRITE_COUNT
} IFRemixSpriteId;

typedef struct IFRemixSpriteEntry
{
    uint32_t file_id;
    uint32_t sprite_offset;
} IFRemixSpriteEntry;

/** Reloc file of the vanilla pause interface (IFCommonBattlePause), where the button sprites live. */
#define IFREMIX_SPRITE_FILE_BUTTONS 0xC5

/** Entry of a sprite, or NULL when id is outside 0..IFREMIX_SPRITE_COUNT-1. */
extern const IFRemixSpriteEntry *ifRemixSpriteRulesGetEntry(int id);

/** Splits a 0xRRGGBBAA colour (the asm colour and palette arguments) into its four bytes. */
extern void ifRemixSpriteRulesUnpackColor(uint32_t rgba, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha);

#endif

#endif
