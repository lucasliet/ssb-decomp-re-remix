/**
 * mnremixcssindicators.c - variant indicators of the Remix VS character select panels, see mnremixcssindicators.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include "if/ifremixspriterules.h"
#include "mn/mnplayers/mnremixcssindicators.h"
#include "mn/mnplayers/mnremixcpulevelrules.h"
#include "mn/mnplayers/mnremixcssrules.h"
#include "mn/mnplayers/mnremixvariants.h"
#include "remix/remix_css_data.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_roster_data.h"

#define MNREMIX_CSS_INDICATOR_GOBJ_LINK 22
#define MNREMIX_CSS_INDICATOR_DL_LINK 28
#define MNREMIX_CSS_FLAG_X_OFFSET (-8)
#define MNREMIX_CSS_FLAG_Y 191
#define MNREMIX_CSS_MARK_RECT_SLOT_FIRST 34
#define MNREMIX_CSS_MARK_SIZE 2.0F
#define MNREMIX_CSS_MARK_RGBA 0xFFFF00FF
/* render_10_._handicap: the icon sits 33 units right of the HANDICAP label at y 199, the level bar under it at y 210. The bar is
 * the 8x29 rectangle at 0x330 of the common file with `sh 3, 0x3C(v0)`: that is Sprite.bmheight, the texel rows drawn (lbCommonDrawSObjAttr
 * takes the quad height from it), so the bar is the first 3 rows of the rectangle at scale 1, not the whole rectangle squeezed to 3. */
#define MNREMIX_CSS_HANDICAP_ICON_X_OFFSET 33.0F
#define MNREMIX_CSS_HANDICAP_ICON_Y 199.0F
#define MNREMIX_CSS_HANDICAP_BAR_Y 210.0F
#define MNREMIX_CSS_HANDICAP_BAR_ROWS 3
#define MNREMIX_CSS_HANDICAP_BAR_OFFSET 0x330

static GObj *sMNRemixCssDpadGObjs[MNREMIX_CSS_INDICATOR_PANELS];
static GObj *sMNRemixCssIconGObjs[MNREMIX_CSS_INDICATOR_PANELS];
static GObj *sMNRemixCssFlagGObjs[MNREMIX_CSS_INDICATOR_PANELS];
static s32 sMNRemixCssIconBase[MNREMIX_CSS_INDICATOR_PANELS];
static s32 sMNRemixCssFlagFkind[MNREMIX_CSS_INDICATOR_PANELS];

void mnRemixCssIndicatorsReset(void)
{
    s32 panel;

    for (panel = 0; panel < MNREMIX_CSS_INDICATOR_PANELS; panel++)
    {
        sMNRemixCssDpadGObjs[panel] = NULL;
        sMNRemixCssIconGObjs[panel] = NULL;
        sMNRemixCssFlagGObjs[panel] = NULL;
        sMNRemixCssIconBase[panel] = nFTKindNull;
        sMNRemixCssFlagFkind[panel] = nFTKindNull;
    }
}

static sb32 mnRemixCssIsFighterKind(s32 fkind)
{
    return ((fkind >= 0) && (fkind < REMIX_CSS_NUM_CHARACTERS) && (fkind != nFTKindNull)) ? TRUE : FALSE;
}

/**
 * The fighter whose D-pad variants a hovered fighter belongs to (Character.variant_original.table): a fighter of the grid is
 * its own original, a variant maps to the grid fighter it varies, and any other fighter (the bonus ones) to itself.
 */
static s32 mnRemixCssGetVariantOriginal(s32 fkind)
{
    s32 slot;
    s32 variant_slot;

    for (slot = 0; slot < REMIX_CSS_NUM_SLOTS; slot++)
    {
        if (kRemixCssSlotIds[slot] == fkind)
        {
            return fkind;
        }
    }
    for (slot = 0; slot < REMIX_CSS_NUM_SLOTS; slot++)
    {
        for (variant_slot = 1; variant_slot <= MNREMIX_VARIANT_SLOT_COUNT; variant_slot++)
        {
            if (mnRemixVariantLookup(kRemixCssSlotIds[slot], variant_slot) == fkind)
            {
                return kRemixCssSlotIds[slot];
            }
        }
    }
    return fkind;
}

/** Variant of `original` in a D-pad direction, or nFTKindNull when it has none (the lookup answers `original` itself then). */
static s32 mnRemixCssGetVariantOfDirection(s32 original, s32 direction)
{
    s32 variant = mnRemixVariantLookup(original, direction + 1);

    return (variant == original) ? nFTKindNull : variant;
}

static sb32 mnRemixCssHasDpadVariants(s32 original)
{
    s32 variants[MNREMIX_CSS_DPAD_DIRECTION_COUNT];
    s32 direction;

    for (direction = 0; direction < MNREMIX_CSS_DPAD_DIRECTION_COUNT; direction++)
    {
        variants[direction] = mnRemixCssGetVariantOfDirection(original, direction);
    }
    return (mnRemixCssHasVariants(variants, nFTKindNull) != 0) ? TRUE : FALSE;
}

/**
 * Stock icon of a fighter from its main file (draw_variant_icon_ _gdk and render_10_._handicap read attribute 0x340 of the character
 * struct). NULL when the main file is not loaded.
 */
static Sprite *mnRemixCssGetStockSprite(s32 fkind)
{
    FTData *data = dFTManagerDataFiles[fkind];
    FTAttributes *attr;
    FTSprites *sprites;

    if ((data == NULL) || (data->p_file_main == NULL) || (*data->p_file_main == NULL))
    {
        return NULL;
    }
    attr = lbRelocGetFileData(FTAttributes*, *data->p_file_main, data->o_attributes);
    sprites = (FTSprites*)PORT_RESOLVE(attr->sprites);

    return (Sprite*)PORT_RESOLVE(sprites->stock_sprite);
}

static Sprite *mnRemixCssGetIconSprite(void *css_images_file, MNRemixCssVariantIcon icon)
{
    if (icon == MNREMIX_CSS_ICON_DONKEY_STOCK)
    {
        return mnRemixCssGetStockSprite(nFTKindDonkey);
    }
    return lbRelocGetFileData(Sprite*, css_images_file, mnRemixCssVariantIconOffset(icon));
}

static GObj *mnRemixCssMakeIndicatorGObj(void)
{
    GObj *gobj = gcMakeGObjSPAfter(0, NULL, MNREMIX_CSS_INDICATOR_GOBJ_LINK, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, MNREMIX_CSS_INDICATOR_DL_LINK, GOBJ_PRIORITY_DEFAULT, ~0);
    gobj->flags = GOBJ_FLAG_HIDDEN;

    return gobj;
}

static void mnRemixCssAddSprite(GObj *gobj, Sprite *sprite, f32 x, f32 y, f32 scale)
{
    SObj *sobj;

    if (sprite == NULL)
    {
        return;
    }
    sobj = lbCommonMakeSObjForGObj(gobj, sprite);
    sobj->pos.x = x;
    sobj->pos.y = y;
    sobj->sprite.scalex = scale;
    sobj->sprite.scaley = scale;
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
}

static void mnRemixCssMakeDpad(s32 panel, void *css_images_file)
{
    GObj *gobj = mnRemixCssMakeIndicatorGObj();

    mnRemixCssAddSprite(gobj, lbRelocGetFileData(Sprite*, css_images_file, ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_DPAD)->sprite_offset),
                        mnRemixCssDpadPanelX(panel), MNREMIX_CSS_DPAD_Y, 1.0F);
    sMNRemixCssDpadGObjs[panel] = gobj;
    sMNRemixCssIconGObjs[panel] = mnRemixCssMakeIndicatorGObj();
    sMNRemixCssFlagGObjs[panel] = mnRemixCssMakeIndicatorGObj();
}

/** Redraws the images of the variants of `original` around the D-pad of a panel. */
static void mnRemixCssMakeVariantIcons(s32 panel, s32 original, void *css_images_file)
{
    GObj *gobj = sMNRemixCssIconGObjs[panel];
    s32 direction;

    gcRemoveSObjAll(gobj);

    if (original == nFTKindNull)
    {
        return;
    }
    for (direction = 0; direction < MNREMIX_CSS_DPAD_DIRECTION_COUNT; direction++)
    {
        s32 variant = mnRemixCssGetVariantOfDirection(original, direction);
        MNRemixCssVariantIcon icon;
        s32 offset_x;
        s32 offset_y;

        if (variant == nFTKindNull)
        {
            continue;
        }
        icon = mnRemixCssVariantIconKind(kRemixRoster[variant].variant_type, variant);
        mnRemixCssDpadIconOffset(direction, &offset_x, &offset_y);
        mnRemixCssAddSprite(gobj, mnRemixCssGetIconSprite(css_images_file, icon), mnRemixCssDpadPanelX(panel) + offset_x,
                            MNREMIX_CSS_DPAD_Y + offset_y, mnRemixCssVariantIconScale(icon));
    }
}

/** The J or E flag of a selected regional fighter, drawn under the D-pad spot (update_variant_indicators_ _draw_flag). */
static void mnRemixCssMakeFlag(s32 panel, s32 fkind, void *css_images_file)
{
    MNRemixCssVariantIcon flag = mnRemixCssVariantIconKind(kRemixRoster[fkind].variant_type, fkind);

    gcRemoveSObjAll(sMNRemixCssFlagGObjs[panel]);
    mnRemixCssAddSprite(sMNRemixCssFlagGObjs[panel], mnRemixCssGetIconSprite(css_images_file, flag),
                        mnRemixCssDpadPanelX(panel) + MNREMIX_CSS_FLAG_X_OFFSET, MNREMIX_CSS_FLAG_Y, 1.0F);
}

static sb32 mnRemixCssIsRegionalFlag(s32 fkind)
{
    MNRemixCssVariantIcon flag = mnRemixCssVariantIconKind(kRemixRoster[fkind].variant_type, fkind);

    return ((flag == MNREMIX_CSS_ICON_FLAG_J) || (flag == MNREMIX_CSS_ICON_FLAG_E)) ? TRUE : FALSE;
}

static void mnRemixCssUpdateMark(s32 panel, sb32 is_dpad_shown, sb32 is_token_held)
{
    s32 slot = mnRemixVariantGetOffset(panel);
    s32 offset_x;
    s32 offset_y;

    if ((is_dpad_shown == FALSE) || (is_token_held == FALSE) || (slot == 0))
    {
        return;
    }
    mnRemixCssDpadMarkOffset(slot - 1, &offset_x, &offset_y);
    remix_hud_rect_put(MNREMIX_CSS_MARK_RECT_SLOT_FIRST + panel, mnRemixCssDpadPanelX(panel) + offset_x, MNREMIX_CSS_DPAD_Y + offset_y,
                       MNREMIX_CSS_MARK_SIZE, MNREMIX_CSS_MARK_SIZE, MNREMIX_CSS_MARK_RGBA);
}

static void mnRemixCssUpdateFlag(s32 panel, const MNRemixCssPanelState *state, void *css_images_file)
{
    sb32 is_flag_shown = ((mnRemixCssIsFighterKind(state->fkind) != FALSE) && (state->is_selected != FALSE) &&
                          (mnRemixCssIsRegionalFlag(state->fkind) != FALSE)) ? TRUE : FALSE;

    if (is_flag_shown == FALSE)
    {
        sMNRemixCssFlagGObjs[panel]->flags = GOBJ_FLAG_HIDDEN;
        sMNRemixCssFlagFkind[panel] = nFTKindNull;
        return;
    }
    if (sMNRemixCssFlagFkind[panel] != state->fkind)
    {
        mnRemixCssMakeFlag(panel, state->fkind, css_images_file);
        sMNRemixCssFlagFkind[panel] = state->fkind;
    }
    sMNRemixCssFlagGObjs[panel]->flags = GOBJ_FLAG_NONE;
}

static void mnRemixCssUpdatePanel(s32 panel, const MNRemixCssPanelState *state, void *css_images_file)
{
    s32 original = (mnRemixCssIsFighterKind(state->fkind) != FALSE) ? mnRemixCssGetVariantOriginal(state->fkind) : nFTKindNull;
    sb32 is_dpad_shown = ((original != nFTKindNull) && (state->is_selected == FALSE) && (mnRemixCssHasDpadVariants(original) != FALSE)) ? TRUE : FALSE;

    if (sMNRemixCssDpadGObjs[panel] == NULL)
    {
        mnRemixCssMakeDpad(panel, css_images_file);
    }
    if (sMNRemixCssIconBase[panel] != original)
    {
        mnRemixCssMakeVariantIcons(panel, original, css_images_file);
        sMNRemixCssIconBase[panel] = original;
    }
    sMNRemixCssDpadGObjs[panel]->flags = (is_dpad_shown != FALSE) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;
    sMNRemixCssIconGObjs[panel]->flags = (is_dpad_shown != FALSE) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;

    mnRemixCssUpdateFlag(panel, state, css_images_file);
    mnRemixCssUpdateMark(panel, is_dpad_shown, state->is_token_held);
}

void mnRemixCssIndicatorsUpdate(void *css_images_file, const MNRemixCssPanelState *panels, s32 panel_count)
{
    s32 panel;

    if ((css_images_file == NULL) || (panel_count > MNREMIX_CSS_INDICATOR_PANELS))
    {
        return;
    }
    for (panel = 0; panel < panel_count; panel++)
    {
        mnRemixCssUpdatePanel(panel, &panels[panel], css_images_file);
    }
}

/** Image of a handicap icon: the stock icon of a fighter, or one of the CSS images file icons. */
static Sprite *mnRemixCssGetHandicapSprite(s32 source, void *css_images_file)
{
    switch (source)
    {
    case MNREMIX_CSS_HANDICAP_SOURCE_POLYGON:
        return mnRemixCssGetIconSprite(css_images_file, MNREMIX_CSS_ICON_POLYGON);

    case MNREMIX_CSS_HANDICAP_SOURCE_METAL:
        return mnRemixCssGetIconSprite(css_images_file, MNREMIX_CSS_ICON_METAL);

    case MNREMIX_CSS_HANDICAP_SOURCE_MASTER_HAND:
        return mnRemixCssGetIconSprite(css_images_file, MNREMIX_CSS_ICON_MASTER_HAND);

    case MNREMIX_CSS_HANDICAP_SOURCE_NONE:
        return NULL;

    default:
        return mnRemixCssGetStockSprite(source);
    }
}

void mnRemixCssMakeHandicapIcon(GObj *gobj, s32 value, f32 label_x, void *css_images_file, void *common_file)
{
    s32 index = mnRemixHandicapIconIndex(value);
    s32 level = mnRemixCssHandicapIconLevel(index);
    f32 icon_x = label_x + MNREMIX_CSS_HANDICAP_ICON_X_OFFSET;
    Sprite *icon = mnRemixCssGetHandicapSprite(mnRemixCssHandicapIconSource(index), css_images_file);
    SObj *bar;

    mnRemixCssAddSprite(gobj, icon, icon_x, MNREMIX_CSS_HANDICAP_ICON_Y, 1.0F);

    if ((level == 0) || (icon == NULL))
    {
        return;
    }
    bar = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, common_file, MNREMIX_CSS_HANDICAP_BAR_OFFSET));
    bar->pos.x = icon_x;
    bar->pos.y = MNREMIX_CSS_HANDICAP_BAR_Y;
    bar->sprite.bmheight = MNREMIX_CSS_HANDICAP_BAR_ROWS;
    bar->sprite.red = mnRemixCssHandicapLevelColor(level) >> 24;
    bar->sprite.green = mnRemixCssHandicapLevelColor(level) >> 16;
    bar->sprite.blue = mnRemixCssHandicapLevelColor(level) >> 8;
    bar->sprite.alpha = mnRemixCssHandicapLevelColor(level);
    bar->sprite.attr &= ~SP_FASTCOPY;
    bar->sprite.attr |= SP_TRANSPARENT;
}

#endif
