/**
 * mnremixcssbonus.c - bonus bookend of the Remix VS character select, see mnremixcssbonus.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <reloc_data.h>
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include "remix/remix_css_data.h"
#include "if/ifremixsprite.h"
#include "mn/mnplayers/mnremixcssbonus.h"
#include "mn/mnplayers/mnremixcssrules.h"
#include "mn/mnplayers/mnremixcssslide.h"

#define MNREMIX_CSS_BONUS_PLAYERS 4
#define MNREMIX_CSS_HINT_PANEL_X 22
#define MNREMIX_CSS_HINT_PANEL_STRIDE 69
#define MNREMIX_CSS_HINT_GOBJ_LINK 22
#define MNREMIX_CSS_HINT_DL_LINK 28
#define MNREMIX_CSS_HINT_BLINK_ON_BELOW 11
#define MNREMIX_CSS_HINT_BLINK_PERIOD 20
#define MNREMIX_CSS_HINT_BUTTON_TINT 0x848484FF
#define MNREMIX_CSS_HINT_BUTTON_PALETTE 0x303030FF
#define MNREMIX_CSS_HINT_ARROW_TINT 0xFF0000FF

typedef enum MNRemixCssHintSprite
{
    MNREMIX_CSS_HINT_ARROW_LEFT,
    MNREMIX_CSS_HINT_ARROW_RIGHT,
    MNREMIX_CSS_HINT_BUTTON_Z,
    MNREMIX_CSS_HINT_BUTTON_R,
    MNREMIX_CSS_HINT_SPRITE_COUNT
} MNRemixCssHintSprite;

/* Offsets from the left edge of a panel (22 + 69 per panel) and tops of the hint sprites, CharacterSelect.asm 2800-2870. */
static const f32 sMNRemixCssHintX[MNREMIX_CSS_HINT_SPRITE_COUNT] = { 4.0F, 56.0F, 4.0F, 49.0F };
static const f32 sMNRemixCssHintY[MNREMIX_CSS_HINT_SPRITE_COUNT] = { 172.0F, 172.0F, 154.0F, 156.0F };

static f32 sMNRemixCssBonusHintPanelX = MNREMIX_CSS_HINT_PANEL_X;
static f32 sMNRemixCssBonusHintPanelStride = MNREMIX_CSS_HINT_PANEL_STRIDE;
static s32 sMNRemixCssBonusIndex[MNREMIX_CSS_BONUS_PLAYERS];
static s32 sMNRemixCssBonusTimer[MNREMIX_CSS_BONUS_PLAYERS] = { -1, -1, -1, -1 };
static GObj *sMNRemixCssBonusHintGObjs[MNREMIX_CSS_BONUS_PLAYERS][MNREMIX_CSS_HINT_SPRITE_COUNT];
static s32 sMNRemixCssBonusHintTimer[MNREMIX_CSS_BONUS_PLAYERS];

void mnRemixCssBonusReset(void)
{
    s32 player;
    s32 sprite;

    sMNRemixCssBonusHintPanelX = MNREMIX_CSS_HINT_PANEL_X;
    sMNRemixCssBonusHintPanelStride = MNREMIX_CSS_HINT_PANEL_STRIDE;

    for (player = 0; player < MNREMIX_CSS_BONUS_PLAYERS; player++)
    {
        sMNRemixCssBonusIndex[player] = 0;
        sMNRemixCssBonusTimer[player] = MNREMIX_CSS_BONUS_REPEAT_IDLE;
        sMNRemixCssBonusHintTimer[player] = 0;

        for (sprite = 0; sprite < MNREMIX_CSS_HINT_SPRITE_COUNT; sprite++)
        {
            sMNRemixCssBonusHintGObjs[player][sprite] = NULL;
        }
    }
}

void mnRemixCssBonusMakeBookend(void *css_images_file, s32 gobj_link, s32 dl_link)
{
    GObj *gobj = gcMakeGObjSPAfter(0, NULL, gobj_link, GOBJ_PRIORITY_DEFAULT);
    SObj *sobj;

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, dl_link, GOBJ_PRIORITY_DEFAULT, ~0);

    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, css_images_file, REMIX_CSS_BONUS_BOOKEND_OFFSET));
    sobj->pos.x = MNREMIX_CSS_BOOKEND_X;
    sobj->pos.y = MNREMIX_CSS_BOOKEND_Y;
    sobj->sprite.scalex = MNREMIX_CSS_BONUS_BOOKEND_SCALE_X;
    sobj->sprite.scaley = MNREMIX_CSS_BONUS_BOOKEND_SCALE_Y;
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    mnRemixCssSlideAttach(gobj, MNREMIX_CSS_BONUS_BOOKEND_SLOT);
}

void mnRemixCssBonusMakeFlash(GObj *gobj, void *css_images_file)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, css_images_file, REMIX_CSS_BONUS_BOOKEND_OFFSET + MNREMIX_CSS_BONUS_FLASH_DELTA));

    sobj->pos.x = MNREMIX_CSS_BOOKEND_X;
    sobj->pos.y = MNREMIX_CSS_BOOKEND_Y;
    sobj->sprite.scalex = MNREMIX_CSS_BONUS_BOOKEND_SCALE_X;
    sobj->sprite.scaley = MNREMIX_CSS_BONUS_BOOKEND_SCALE_Y;
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
}

s32 mnRemixCssBonusGetFighterKind(s32 token)
{
    if ((token < 0) || (token >= MNREMIX_CSS_BONUS_PLAYERS))
    {
        return -1;
    }
    return kRemixCssBonusIds[sMNRemixCssBonusIndex[token]];
}

sb32 mnRemixCssBonusScroll(s32 token, s32 cursor_player, u16 held_buttons, sb32 is_over_bookend)
{
    s32 step;

    if ((token < 0) || (token >= MNREMIX_CSS_BONUS_PLAYERS) || (cursor_player < 0) || (cursor_player >= MNREMIX_CSS_BONUS_PLAYERS))
    {
        return FALSE;
    }
    step = (is_over_bookend != FALSE) ? mnRemixCssBonusHeldStep((held_buttons & Z_TRIG) != 0, (held_buttons & R_TRIG) != 0) : 0;

    if (mnRemixCssBonusRepeat(&sMNRemixCssBonusTimer[cursor_player], (step != 0) ? 1 : 0) == 0)
    {
        return FALSE;
    }
    sMNRemixCssBonusIndex[token] = mnRemixCssBonusStep(sMNRemixCssBonusIndex[token], step);

    return TRUE;
}

static void mnRemixCssBonusMakeHint(s32 panel, void *arrows_file)
{
    f32 panel_x = sMNRemixCssBonusHintPanelX + (sMNRemixCssBonusHintPanelStride * panel);
    GObj **gobjs = sMNRemixCssBonusHintGObjs[panel];

    gobjs[MNREMIX_CSS_HINT_ARROW_LEFT] = ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowLSprite, MNREMIX_CSS_HINT_GOBJ_LINK,
        MNREMIX_CSS_HINT_DL_LINK, panel_x + sMNRemixCssHintX[MNREMIX_CSS_HINT_ARROW_LEFT], sMNRemixCssHintY[MNREMIX_CSS_HINT_ARROW_LEFT],
        MNREMIX_CSS_HINT_ARROW_TINT, MNREMIX_CSS_HINT_BUTTON_PALETTE, 1.0F);
    gobjs[MNREMIX_CSS_HINT_ARROW_RIGHT] = ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowRSprite, MNREMIX_CSS_HINT_GOBJ_LINK,
        MNREMIX_CSS_HINT_DL_LINK, panel_x + sMNRemixCssHintX[MNREMIX_CSS_HINT_ARROW_RIGHT], sMNRemixCssHintY[MNREMIX_CSS_HINT_ARROW_RIGHT],
        MNREMIX_CSS_HINT_ARROW_TINT, MNREMIX_CSS_HINT_BUTTON_PALETTE, 1.0F);
    gobjs[MNREMIX_CSS_HINT_BUTTON_Z] = ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_Z, MNREMIX_CSS_HINT_GOBJ_LINK, MNREMIX_CSS_HINT_DL_LINK,
        panel_x + sMNRemixCssHintX[MNREMIX_CSS_HINT_BUTTON_Z], sMNRemixCssHintY[MNREMIX_CSS_HINT_BUTTON_Z], MNREMIX_CSS_HINT_BUTTON_TINT,
        MNREMIX_CSS_HINT_BUTTON_PALETTE, 1.0F);
    gobjs[MNREMIX_CSS_HINT_BUTTON_R] = ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_R, MNREMIX_CSS_HINT_GOBJ_LINK, MNREMIX_CSS_HINT_DL_LINK,
        panel_x + sMNRemixCssHintX[MNREMIX_CSS_HINT_BUTTON_R], sMNRemixCssHintY[MNREMIX_CSS_HINT_BUTTON_R], MNREMIX_CSS_HINT_BUTTON_TINT,
        MNREMIX_CSS_HINT_BUTTON_PALETTE, 1.0F);
}

static void mnRemixCssBonusSetHintFlags(s32 panel, u32 arrow_flags, u32 button_flags)
{
    GObj **gobjs = sMNRemixCssBonusHintGObjs[panel];

    if (gobjs[MNREMIX_CSS_HINT_ARROW_LEFT] == NULL)
    {
        return;
    }
    gobjs[MNREMIX_CSS_HINT_ARROW_LEFT]->flags = arrow_flags;
    gobjs[MNREMIX_CSS_HINT_ARROW_RIGHT]->flags = arrow_flags;
    gobjs[MNREMIX_CSS_HINT_BUTTON_Z]->flags = button_flags;
    gobjs[MNREMIX_CSS_HINT_BUTTON_R]->flags = button_flags;
}

void mnRemixCssBonusSetHintPanelX(f32 panel_x)
{
    sMNRemixCssBonusHintPanelX = panel_x;
}

void mnRemixCssBonusSetHintPanelStride(f32 panel_stride)
{
    sMNRemixCssBonusHintPanelStride = panel_stride;
}

void mnRemixCssBonusUpdateHint(s32 panel, sb32 is_visible, void *arrows_file)
{
    s32 raw_timer;

    if ((panel < 0) || (panel >= MNREMIX_CSS_BONUS_PLAYERS))
    {
        return;
    }
    if (is_visible == FALSE)
    {
        mnRemixCssBonusSetHintFlags(panel, GOBJ_FLAG_HIDDEN, GOBJ_FLAG_HIDDEN);
        sMNRemixCssBonusHintTimer[panel] = 0;
        return;
    }
    if (sMNRemixCssBonusHintGObjs[panel][MNREMIX_CSS_HINT_ARROW_LEFT] == NULL)
    {
        mnRemixCssBonusMakeHint(panel, arrows_file);
    }
    raw_timer = sMNRemixCssBonusHintTimer[panel] + 1;

    mnRemixCssBonusSetHintFlags(panel, (raw_timer < MNREMIX_CSS_HINT_BLINK_ON_BELOW) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN, GOBJ_FLAG_NONE);

    sMNRemixCssBonusHintTimer[panel] = (raw_timer >= MNREMIX_CSS_HINT_BLINK_PERIOD) ? 0 : raw_timer;
}

#endif
