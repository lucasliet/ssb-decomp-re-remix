#ifndef _IFREMIXSPRITERULES_H_
#define _IFREMIXSPRITERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <stddef.h>
#include <stdint.h>

/*
 * Table of the Remix button and extension sprites (docs/remix/menus.md), pure so the headless unit
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
    IFREMIX_SPRITE_LEGEND_WORD,
    IFREMIX_SPRITE_DPAD,
    IFREMIX_SPRITE_C_ARROW_A,
    IFREMIX_SPRITE_C_ARROW_B,
    IFREMIX_SPRITE_C_ARROW_C,
    IFREMIX_SPRITE_C_ARROW_D,
    IFREMIX_SPRITE_KOTH_METER,
    IFREMIX_SPRITE_MULTIMAN_POLYGON,
    IFREMIX_SPRITE_MULTIMAN_CROSS,
    IFREMIX_SPRITE_START_BUTTON,
    IFREMIX_SPRITE_MENU_ARROW,
    IFREMIX_SPRITE_COUNT
} IFRemixSpriteId;

typedef struct IFRemixSpriteEntry
{
    uint32_t file_id;
    uint32_t sprite_offset;
} IFRemixSpriteEntry;

/** Reloc file of the vanilla pause interface (IFCommonBattlePause), where the button sprites live. */
#define IFREMIX_SPRITE_FILE_BUTTONS 0xC5
/** Remix CSS images file (File.CSS_IMAGES): the D-pad and the C stick arrows. */
#define IFREMIX_SPRITE_FILE_CSS_IMAGES 0xA06
/** Vanilla button images file (File 0x19): holds the polygon stock icon of Multi-Man. */
#define IFREMIX_SPRITE_FILE_STOCK_ICONS 0x19
/** Vanilla interface digits file: the "x" sprite (llIFCommonDigitsCrossSprite). */
#define IFREMIX_SPRITE_FILE_DIGITS 0x24
/** King of the Hill graphics file (File.KING_OF_THE_HILL_GFX): the score meter frame. */
#define IFREMIX_SPRITE_FILE_KOTH 0x14F3

/** Vanilla main menu images file (File 0x00, MNCommon): the right arrow of the item switch legend. */
#define IFREMIX_SPRITE_FILE_MENU_COMMON 0x00

/** Vanilla sound test file: the START button sprite (llMNSoundTestStartButtonSprite). */
#define IFREMIX_SPRITE_FILE_SOUND_TEST 0xC4

/** Entry of a sprite, or NULL when id is outside 0..IFREMIX_SPRITE_COUNT-1. */
extern const IFRemixSpriteEntry *ifRemixSpriteRulesGetEntry(int id);

/** Splits a 0xRRGGBBAA colour (the asm colour and palette arguments) into its four bytes. */
extern void ifRemixSpriteRulesUnpackColor(uint32_t rgba, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha);

#endif

#endif
