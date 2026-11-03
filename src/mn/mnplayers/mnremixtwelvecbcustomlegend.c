/**
 * mnremixtwelvecbcustomlegend.c - legend of the Custom character set, see mnremixtwelvecbcustomlegend.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <reloc_data.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include "if/ifremixsprite.h"
#include "mn/mnplayers/mnremixtwelvecbcustomlegend.h"
#include "remix/remix_hud_text.h"

#define LEGEND_CURSORS 2 /* one legend per panel: the side of the player whose token is held */
#define LEGEND_SPRITES 8
#define LEGEND_GOBJ_LINK 25
#define LEGEND_DL_LINK 26
#define LEGEND_TEXT_SLOT_FIRST 100
#define LEGEND_TEXT_SLOTS 4
#define LEGEND_RECT_SLOT_FIRST 40
#define LEGEND_RECT_SLOTS 3
#define LEGEND_TEXT_SCALE 0.8125F
#define LEGEND_ICON_SCALE 0.75F
#define LEGEND_BUTTON_TINT 0x848484FF
#define LEGEND_BUTTON_PALETTE 0x303030FF
#define LEGEND_ARROW_TINT 0xFF0000FF
#define LEGEND_ARROW_PALETTE 0x303030FF
#define LEGEND_COLOR_WHITE 0xFFFFFFFF
#define LEGEND_COLOR_YELLOW 0xFFFF00FF
#define LEGEND_PIP_SIZE 2.0F
/* The arrows show for 10 frames of every 20 (update_display, timer below 11 then up to 20). */
#define LEGEND_BLINK_ON_FRAMES 11
#define LEGEND_BLINK_PERIOD 20

typedef enum LegendSprite
{
    LEGEND_SPRITE_ARROW_LEFT,
    LEGEND_SPRITE_ARROW_RIGHT,
    LEGEND_SPRITE_Z,
    LEGEND_SPRITE_R,
    LEGEND_SPRITE_L,
    LEGEND_SPRITE_DPAD_PRESETS,
    LEGEND_SPRITE_DPAD_RANDOM,
    LEGEND_SPRITE_DPAD_COPY
} LegendSprite;

/* X per panel (player 1, player 2) and Y of each sprite; the Y values and the P1 X come from the asm, the P2 X from the panel patch. */
typedef struct LegendSpritePlace
{
    f32 x[2];
    f32 y;
} LegendSpritePlace;

static const LegendSpritePlace sLegendSpritePlaces[LEGEND_SPRITES] =
{
    { { 26.0F, 234.0F }, 172.0F },
    { { 78.0F, 285.0F }, 172.0F },
    { { 26.0F, 234.0F }, 154.0F },
    { { 71.0F, 277.5F }, 156.0F },
    { { 25.0F, 232.0F }, 216.0F },
    { { 93.0F, 161.0F }, 186.0F },
    { { 93.0F, 161.0F }, 200.0F },
    { { 93.0F, 161.0F }, 214.0F }
};

typedef struct LegendText
{
    const char *text;
    f32 x[2];
    f32 y;
} LegendText;

static const LegendText sLegendTexts[LEGEND_TEXT_SLOTS] =
{
    { ": Set All", { 41.0F, 247.5F }, 217.0F },
    { ": Presets", { 105.0F, 173.0F }, 187.0F },
    { ": Random", { 105.0F, 173.0F }, 201.0F },
    { ": Copy", { 105.0F, 173.0F }, 215.0F }
};

typedef struct LegendPip
{
    f32 x[2];
    f32 y;
    u32 rgba;
} LegendPip;

/* Square marks on the D-pad icons: up for Presets, down for Random, and the up mark of the hold for Copy (white). */
static const LegendPip sLegendPips[LEGEND_RECT_SLOTS] =
{
    { { 98.0F, 166.0F }, 188.0F, LEGEND_COLOR_YELLOW },
    { { 98.0F, 166.0F }, 208.0F, LEGEND_COLOR_YELLOW },
    { { 98.0F, 166.0F }, 216.0F, LEGEND_COLOR_WHITE }
};

static GObj *sLegendGObjs[LEGEND_CURSORS][LEGEND_SPRITES];
static s32 sLegendBlinkTimer[LEGEND_CURSORS];

void mnRemixTwelveCBCustomLegendReset(void)
{
    s32 cursor;
    s32 sprite;

    for (cursor = 0; cursor < LEGEND_CURSORS; cursor++)
    {
        for (sprite = 0; sprite < LEGEND_SPRITES; sprite++)
        {
            sLegendGObjs[cursor][sprite] = NULL;
        }
        sLegendBlinkTimer[cursor] = 0;
    }
}

static void makeSprites(s32 cursor, s32 side, void *arrows_file)
{
    const LegendSpritePlace *place = sLegendSpritePlaces;
    GObj **gobjs = sLegendGObjs[cursor];

    gobjs[LEGEND_SPRITE_ARROW_LEFT] = ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowLSprite, LEGEND_GOBJ_LINK, LEGEND_DL_LINK,
        place[LEGEND_SPRITE_ARROW_LEFT].x[side], place[LEGEND_SPRITE_ARROW_LEFT].y, LEGEND_ARROW_TINT, LEGEND_ARROW_PALETTE, 1.0F);
    gobjs[LEGEND_SPRITE_ARROW_RIGHT] = ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowRSprite, LEGEND_GOBJ_LINK, LEGEND_DL_LINK,
        place[LEGEND_SPRITE_ARROW_RIGHT].x[side], place[LEGEND_SPRITE_ARROW_RIGHT].y, LEGEND_ARROW_TINT, LEGEND_ARROW_PALETTE, 1.0F);
    gobjs[LEGEND_SPRITE_Z] = ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_Z, LEGEND_GOBJ_LINK, LEGEND_DL_LINK, place[LEGEND_SPRITE_Z].x[side],
        place[LEGEND_SPRITE_Z].y, LEGEND_BUTTON_TINT, LEGEND_BUTTON_PALETTE, 1.0F);
    gobjs[LEGEND_SPRITE_R] = ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_R, LEGEND_GOBJ_LINK, LEGEND_DL_LINK, place[LEGEND_SPRITE_R].x[side],
        place[LEGEND_SPRITE_R].y, LEGEND_BUTTON_TINT, LEGEND_BUTTON_PALETTE, 1.0F);
    gobjs[LEGEND_SPRITE_L] = ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_L, LEGEND_GOBJ_LINK, LEGEND_DL_LINK, place[LEGEND_SPRITE_L].x[side],
        place[LEGEND_SPRITE_L].y, LEGEND_BUTTON_TINT, LEGEND_BUTTON_PALETTE, 1.0F);
    gobjs[LEGEND_SPRITE_DPAD_PRESETS] = ifRemixSpriteMake(IFREMIX_SPRITE_DPAD, LEGEND_GOBJ_LINK, LEGEND_DL_LINK,
        place[LEGEND_SPRITE_DPAD_PRESETS].x[side], place[LEGEND_SPRITE_DPAD_PRESETS].y, LEGEND_BUTTON_TINT, LEGEND_BUTTON_PALETTE, LEGEND_ICON_SCALE);
    gobjs[LEGEND_SPRITE_DPAD_RANDOM] = ifRemixSpriteMake(IFREMIX_SPRITE_DPAD, LEGEND_GOBJ_LINK, LEGEND_DL_LINK,
        place[LEGEND_SPRITE_DPAD_RANDOM].x[side], place[LEGEND_SPRITE_DPAD_RANDOM].y, LEGEND_BUTTON_TINT, LEGEND_BUTTON_PALETTE, LEGEND_ICON_SCALE);
    gobjs[LEGEND_SPRITE_DPAD_COPY] = ifRemixSpriteMake(IFREMIX_SPRITE_DPAD, LEGEND_GOBJ_LINK, LEGEND_DL_LINK,
        place[LEGEND_SPRITE_DPAD_COPY].x[side], place[LEGEND_SPRITE_DPAD_COPY].y, LEGEND_BUTTON_TINT, LEGEND_BUTTON_PALETTE, LEGEND_ICON_SCALE);
}

static void setSpritesHidden(s32 cursor, sb32 is_hidden)
{
    s32 sprite;

    for (sprite = 0; sprite < LEGEND_SPRITES; sprite++)
    {
        GObj *gobj = sLegendGObjs[cursor][sprite];

        if (gobj != NULL)
        {
            gobj->flags = (is_hidden != FALSE) ? GOBJ_FLAG_HIDDEN : GOBJ_FLAG_NONE;
        }
    }
}

static void blinkArrows(s32 cursor)
{
    sb32 is_visible;

    sLegendBlinkTimer[cursor] = (sLegendBlinkTimer[cursor] + 1) % LEGEND_BLINK_PERIOD;
    is_visible = (sLegendBlinkTimer[cursor] < LEGEND_BLINK_ON_FRAMES) ? TRUE : FALSE;

    if (sLegendGObjs[cursor][LEGEND_SPRITE_ARROW_LEFT] != NULL)
    {
        sLegendGObjs[cursor][LEGEND_SPRITE_ARROW_LEFT]->flags = (is_visible != FALSE) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;
    }
    if (sLegendGObjs[cursor][LEGEND_SPRITE_ARROW_RIGHT] != NULL)
    {
        sLegendGObjs[cursor][LEGEND_SPRITE_ARROW_RIGHT]->flags = (is_visible != FALSE) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;
    }
}

static void putTextsAndPips(s32 cursor, s32 side)
{
    s32 index;

    for (index = 0; index < LEGEND_TEXT_SLOTS; index++)
    {
        remix_hud_text_put(LEGEND_TEXT_SLOT_FIRST + (cursor * LEGEND_TEXT_SLOTS) + index, sLegendTexts[index].text,
                           sLegendTexts[index].x[side], sLegendTexts[index].y, LEGEND_COLOR_WHITE, LEGEND_TEXT_SCALE,
                           REMIX_HUD_ALIGN_LEFT);
    }
    for (index = 0; index < LEGEND_RECT_SLOTS; index++)
    {
        remix_hud_rect_put(LEGEND_RECT_SLOT_FIRST + (cursor * LEGEND_RECT_SLOTS) + index, sLegendPips[index].x[side],
                           sLegendPips[index].y, LEGEND_PIP_SIZE, LEGEND_PIP_SIZE, sLegendPips[index].rgba);
    }
}

void mnRemixTwelveCBCustomLegendUpdate(s32 side, sb32 is_shown, void *arrows_file)
{
    if ((side < 0) || (side >= LEGEND_CURSORS))
    {
        return;
    }
    if (is_shown == FALSE)
    {
        setSpritesHidden(side, TRUE);
        return;
    }
    if (sLegendGObjs[side][LEGEND_SPRITE_Z] == NULL)
    {
        makeSprites(side, side, arrows_file);
    }
    setSpritesHidden(side, FALSE);
    blinkArrows(side);
    putTextsAndPips(side, side);
}

#endif
