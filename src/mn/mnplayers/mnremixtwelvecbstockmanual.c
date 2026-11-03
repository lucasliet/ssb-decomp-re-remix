/**
 * mnremixtwelvecbstockmanual.c - Manual stock mode controls of the 12-Character Battle panels, see mnremixtwelvecbstockmanual.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <reloc_data.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include "if/ifremixsprite.h"
#include "mn/mnplayers/mnremixtwelvecbstockmanual.h"
#include "remix/remix_hud_text.h"

#define STOCK_MANUAL_PLAYERS 2
/* The links of the stock icon and the "x" next to them: the card of the panel is drawn over the links 25 and 26. */
#define STOCK_MANUAL_GOBJ_LINK 22
#define STOCK_MANUAL_DL_LINK 28
#define STOCK_MANUAL_ARROW_Y 193.0F
#define STOCK_MANUAL_ARROW_SCALE 0.625F
#define STOCK_MANUAL_ARROW_TINT 0xFF0000FF
#define STOCK_MANUAL_ARROW_PALETTE 0x303030FF
#define STOCK_MANUAL_ARROW_LEFT_INSET 20.0F
#define STOCK_MANUAL_ARROW_RIGHT_INSET 5.0F
#define STOCK_MANUAL_ARROW_MARGIN 8.0F
#define STOCK_MANUAL_DPAD_X (-27.5F)
#define STOCK_MANUAL_DPAD_Y 192.0F
#define STOCK_MANUAL_DPAD_SCALE 0.5625F
#define STOCK_MANUAL_DPAD_TINT 0x848484FF
#define STOCK_MANUAL_DPAD_PALETTE 0x303030FF
#define STOCK_MANUAL_PIP_LEFT_X (-25.0F)
#define STOCK_MANUAL_PIP_RIGHT_X (-20.0F)
#define STOCK_MANUAL_PIP_Y 196.0F
#define STOCK_MANUAL_PIP_SIZE 1.0F
#define STOCK_MANUAL_PIP_RGBA 0xFFFF00FF
#define STOCK_MANUAL_PIP_SLOT_FIRST 30
/* The marks and arrows show for 11 frames of every 20 (arrow_state_routine_: timer below 11, the timer counts 0 to 19). */
#define STOCK_MANUAL_BLINK_ON_FRAMES 11
#define STOCK_MANUAL_BLINK_PERIOD 20

typedef struct StockManualSprites
{
    GObj *arrow_left;
    GObj *arrow_right;
    GObj *dpad;
    sb32 are_arrows_active;
} StockManualSprites;

static StockManualSprites sSprites[STOCK_MANUAL_PLAYERS];
static s32 sBlinkTimer;

void mnRemixTwelveCBStockManualReset(void)
{
    s32 player;

    for (player = 0; player < STOCK_MANUAL_PLAYERS; player++)
    {
        sSprites[player].arrow_left = NULL;
        sSprites[player].arrow_right = NULL;
        sSprites[player].dpad = NULL;
        sSprites[player].are_arrows_active = FALSE;
    }
    sBlinkTimer = 0;
}

static void setHidden(GObj *gobj, sb32 is_hidden)
{
    if (gobj != NULL)
    {
        gobj->flags = (is_hidden != FALSE) ? GOBJ_FLAG_HIDDEN : GOBJ_FLAG_NONE;
    }
}

static void setPosition(GObj *gobj, f32 x, f32 y)
{
    SObj *sobj = SObjGetStruct(gobj);

    sobj->pos.x = x;
    sobj->pos.y = y;
}

static void showArrows(StockManualSprites *sprites, f32 panel_x, f32 card_width, sb32 is_visible, void *arrows_file)
{
    f32 right_x = panel_x + card_width - STOCK_MANUAL_ARROW_RIGHT_INSET;
    f32 left_x = panel_x + STOCK_MANUAL_ARROW_LEFT_INSET;

    if (sprites->arrow_left == NULL)
    {
        sprites->arrow_left = ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowLSprite, STOCK_MANUAL_GOBJ_LINK,
            STOCK_MANUAL_DL_LINK, left_x, STOCK_MANUAL_ARROW_Y, STOCK_MANUAL_ARROW_TINT, STOCK_MANUAL_ARROW_PALETTE, STOCK_MANUAL_ARROW_SCALE);
        sprites->arrow_right = ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowRSprite, STOCK_MANUAL_GOBJ_LINK,
            STOCK_MANUAL_DL_LINK, right_x, STOCK_MANUAL_ARROW_Y, STOCK_MANUAL_ARROW_TINT, STOCK_MANUAL_ARROW_PALETTE, STOCK_MANUAL_ARROW_SCALE);
    }
    if ((sprites->arrow_left == NULL) || (sprites->arrow_right == NULL))
    {
        return;
    }
    setPosition(sprites->arrow_left, left_x, STOCK_MANUAL_ARROW_Y);
    setPosition(sprites->arrow_right, right_x, STOCK_MANUAL_ARROW_Y);
    setHidden(sprites->arrow_left, !is_visible);
    setHidden(sprites->arrow_right, !is_visible);
}

static void showDpad(StockManualSprites *sprites, s32 player, f32 digits_x, sb32 is_visible)
{
    f32 icon_x = digits_x + STOCK_MANUAL_DPAD_X;

    if (sprites->dpad == NULL)
    {
        sprites->dpad = ifRemixSpriteMake(IFREMIX_SPRITE_DPAD, STOCK_MANUAL_GOBJ_LINK, STOCK_MANUAL_DL_LINK, icon_x, STOCK_MANUAL_DPAD_Y,
            STOCK_MANUAL_DPAD_TINT, STOCK_MANUAL_DPAD_PALETTE, STOCK_MANUAL_DPAD_SCALE);
    }
    if (sprites->dpad == NULL)
    {
        return;
    }
    setPosition(sprites->dpad, icon_x, STOCK_MANUAL_DPAD_Y);
    setHidden(sprites->dpad, !is_visible);

    if (is_visible != FALSE)
    {
        remix_hud_rect_put(STOCK_MANUAL_PIP_SLOT_FIRST + (player * 2), digits_x + STOCK_MANUAL_PIP_LEFT_X, STOCK_MANUAL_PIP_Y,
                           STOCK_MANUAL_PIP_SIZE, STOCK_MANUAL_PIP_SIZE, STOCK_MANUAL_PIP_RGBA);
        remix_hud_rect_put(STOCK_MANUAL_PIP_SLOT_FIRST + (player * 2) + 1, digits_x + STOCK_MANUAL_PIP_RIGHT_X, STOCK_MANUAL_PIP_Y,
                           STOCK_MANUAL_PIP_SIZE, STOCK_MANUAL_PIP_SIZE, STOCK_MANUAL_PIP_RGBA);
    }
}

void mnRemixTwelveCBStockManualUpdate(s32 player, MNRemixTwelveCBStockControls controls, f32 panel_x, f32 card_width, f32 digits_x,
                                      void *arrows_file)
{
    StockManualSprites *sprites;
    sb32 is_blink_on;

    if ((player < 0) || (player >= STOCK_MANUAL_PLAYERS))
    {
        return;
    }
    sprites = &sSprites[player];
    is_blink_on = (sBlinkTimer < STOCK_MANUAL_BLINK_ON_FRAMES) ? TRUE : FALSE;
    sprites->are_arrows_active = (controls == MNREMIX_TCB_STOCK_CONTROLS_ARROWS) ? TRUE : FALSE;

    if (controls == MNREMIX_TCB_STOCK_CONTROLS_ARROWS)
    {
        showArrows(sprites, panel_x, card_width, is_blink_on, arrows_file);
    }
    else
    {
        setHidden(sprites->arrow_left, TRUE);
        setHidden(sprites->arrow_right, TRUE);
    }
    if (controls == MNREMIX_TCB_STOCK_CONTROLS_DPAD)
    {
        showDpad(sprites, player, digits_x, is_blink_on);
    }
    else
    {
        setHidden(sprites->dpad, TRUE);
    }
    if (player == (STOCK_MANUAL_PLAYERS - 1))
    {
        sBlinkTimer = (sBlinkTimer + 1) % STOCK_MANUAL_BLINK_PERIOD;
    }
}

static sb32 isPointInArrow(GObj *arrow, f32 hot_x, f32 hot_y, f32 left_margin, f32 right_margin)
{
    SObj *sobj;
    f32 left;
    f32 right;

    if (arrow == NULL)
    {
        return FALSE;
    }
    sobj = SObjGetStruct(arrow);
    left = sobj->pos.x - left_margin;
    right = sobj->pos.x + sobj->sprite.width + right_margin;

    return ((hot_x >= left) && (hot_x <= right) && (hot_y >= sobj->pos.y) && (hot_y <= (sobj->pos.y + sobj->sprite.height))) ? TRUE : FALSE;
}

s32 mnRemixTwelveCBStockManualArrowAt(s32 player, f32 hot_x, f32 hot_y)
{
    StockManualSprites *sprites;

    if ((player < 0) || (player >= STOCK_MANUAL_PLAYERS))
    {
        return 0;
    }
    sprites = &sSprites[player];

    if (sprites->are_arrows_active == FALSE)
    {
        return 0;
    }
    if (isPointInArrow(sprites->arrow_left, hot_x, hot_y, 0.0F, STOCK_MANUAL_ARROW_MARGIN) != FALSE)
    {
        return -1;
    }
    return (isPointInArrow(sprites->arrow_right, hot_x, hot_y, STOCK_MANUAL_ARROW_MARGIN, 0.0F) != FALSE) ? 1 : 0;
}

#endif
