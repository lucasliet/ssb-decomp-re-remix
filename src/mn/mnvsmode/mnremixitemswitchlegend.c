#include <mn/mnvsmode/mnremixitemswitchlegend.h>

#if defined(PORT) && defined(REGION_REMIX)
#include "if/ifremixsprite.h"
#include "if/ifremixtextlayer.h"
#include "remix/remix_hud_text.h"

/**
 * Makes the button and arrow sprites of the legend; call once when the screen starts, after its cameras exist.
 */
void mnRemixItemSwitchLegendMake(void)
{
    ifRemixSpriteMake
    (
        IFREMIX_SPRITE_BUTTON_R,
        MN_REMIX_ITEM_SWITCH_LEGEND_GOBJ_LINK,
        MN_REMIX_ITEM_SWITCH_LEGEND_DL_LINK,
        MN_REMIX_ITEM_SWITCH_R_X,
        MN_REMIX_ITEM_SWITCH_R_Y,
        MN_REMIX_ITEM_SWITCH_BUTTON_TINT,
        MN_REMIX_ITEM_SWITCH_BUTTON_PALETTE,
        MN_REMIX_ITEM_SWITCH_BUTTON_SCALE
    );
    ifRemixSpriteMake
    (
        IFREMIX_SPRITE_MENU_ARROW,
        MN_REMIX_ITEM_SWITCH_LEGEND_GOBJ_LINK,
        MN_REMIX_ITEM_SWITCH_LEGEND_DL_LINK,
        MN_REMIX_ITEM_SWITCH_ARROW_X,
        MN_REMIX_ITEM_SWITCH_ARROW_Y,
        MN_REMIX_ITEM_SWITCH_ARROW_TINT,
        MN_REMIX_ITEM_SWITCH_ARROW_PALETTE,
        MN_REMIX_ITEM_SWITCH_ARROW_SCALE
    );
    ifRemixSpriteMake
    (
        IFREMIX_SPRITE_BUTTON_L,
        MN_REMIX_ITEM_SWITCH_LEGEND_GOBJ_LINK,
        MN_REMIX_ITEM_SWITCH_LEGEND_DL_LINK,
        MN_REMIX_ITEM_SWITCH_L_X,
        MN_REMIX_ITEM_SWITCH_L_Y,
        MN_REMIX_ITEM_SWITCH_BUTTON_TINT,
        MN_REMIX_ITEM_SWITCH_BUTTON_PALETTE,
        MN_REMIX_ITEM_SWITCH_BUTTON_SCALE
    );
}

/**
 * Makes the layer that draws the legend text in the cursor's room (Item.asm item_switch_setup_), after the legend sprites and
 * the cursor; call once when the screen starts.
 */
void mnRemixItemSwitchLegendMakeLayer(void)
{
    GObj *layer_gobj = ifRemixTextLayerMake
    (
        MN_REMIX_ITEM_SWITCH_LEGEND_TEXT_GOBJ_LINK,
        MN_REMIX_ITEM_SWITCH_LEGEND_DL_LINK,
        GOBJ_PRIORITY_DEFAULT
    );

    ifRemixTextLayerBindLabels(layer_gobj, MN_REMIX_ITEM_SWITCH_TEXT_SLOT, 1);
}

/**
 * Keeps the legend text visible; call once per frame while the screen runs.
 */
void mnRemixItemSwitchLegendPut(void)
{
    remix_hud_text_put
    (
        MN_REMIX_ITEM_SWITCH_TEXT_SLOT,
        MN_REMIX_ITEM_SWITCH_TEXT,
        MN_REMIX_ITEM_SWITCH_TEXT_X,
        MN_REMIX_ITEM_SWITCH_TEXT_Y,
        MN_REMIX_ITEM_SWITCH_TEXT_RGBA,
        MN_REMIX_ITEM_SWITCH_TEXT_SCALE,
        REMIX_HUD_ALIGN_LEFT
    );
}
#endif
