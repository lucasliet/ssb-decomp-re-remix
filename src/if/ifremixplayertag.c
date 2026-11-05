/**
 * ifremixplayertag.c - Smash Remix Player Tag option (css/PlayerTag.asm, src/PlayerTag.asm use_custom_tag_image_,
 * create_custom_tag_ and always_display_custom_tag_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <gm/gmremixpanel.h>
#include "remix/remix_hud_text.h"

/* HUD text slots of the tag names of the four ports (remix_hud_text.h slot map) */
#define IFREMIX_PLAYERTAG_TEXT_SLOT 5
/* src/PlayerTag.asm MAX_WIDTH: the name is squeezed to this width */
#define IFREMIX_PLAYERTAG_MAX_WIDTH 96.0F

/* Arrow only indicator sprite of file 0x26, used by every custom tag value (src/PlayerTag.asm use_custom_tag_image_) */
#define IFREMIX_PLAYERTAG_ARROW_OFFSET 0x1158

/**
 * Returns the arrow only sprite offset when the port has a custom tag, otherwise the vanilla tag sprite offset. The
 * custom tag names are typed in Remix and are not drawn by the port.
 */
intptr_t ifRemixPlayerTagSpriteOffset(s32 player, intptr_t vanilla_offset)
{
    if (gmRemixPanel(PLAYER_TAG, player) == 0)
    {
        return vanilla_offset;
    }
    return IFREMIX_PLAYERTAG_ARROW_OFFSET;
}

/**
 * Draws the custom tag name of a port over its battle indicator (src/PlayerTag.asm create_custom_tag_): the name typed for
 * the tag, centred on the indicator sprite at its top, in the colour of the tag. Draws nothing for the default tag or an
 * unnamed tag (the asm skips a name that is only spaces).
 *
 * @param player Port index (0..3).
 * @param center_x Screen X of the middle of the indicator sprite.
 * @param top_y Screen Y of the top of the indicator sprite.
 * @param rgba Tag colour as 0xRRGGBBAA.
 */
void ifRemixPlayerTagDrawName(s32 player, f32 center_x, f32 top_y, u32 rgba)
{
    const char *name = remix_panel_tag_name(gmRemixPanel(PLAYER_TAG, player));

    if (name[0] == '\0')
    {
        return;
    }
    remix_hud_text_put_fit(IFREMIX_PLAYERTAG_TEXT_SLOT + player, name, center_x, top_y, rgba, 1.0F, REMIX_HUD_ALIGN_CENTER,
                           IFREMIX_PLAYERTAG_MAX_WIDTH);
}

/**
 * TRUE when the port has a custom tag, which keeps the tag shown whatever its wait countdown is
 * (src/PlayerTag.asm always_display_custom_tag_).
 */
sb32 ifRemixPlayerTagAlwaysShown(s32 player)
{
    return gmRemixPanel(PLAYER_TAG, player) != 0;
}

#endif
