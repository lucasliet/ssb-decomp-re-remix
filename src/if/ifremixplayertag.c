/**
 * ifremixplayertag.c - Smash Remix Player Tag option (css/PlayerTag.asm, src/PlayerTag.asm use_custom_tag_image_,
 * create_custom_tag_ and always_display_custom_tag_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <gm/gmremixpanel.h>
#include "remix/remix_hud_text.h"

/* HUD text slots of the tag names of the four ports (remix_hud_text.h slot map): the black copies draw under the names */
#define IFREMIX_PLAYERTAG_SHADOW_SLOT 5
#define IFREMIX_PLAYERTAG_TEXT_SLOT 9
/* src/PlayerTag.asm:136-151 black copy: X offset 7 against the 8 of the coloured name, Y offset -1.65, character scale 1.25 */
#define IFREMIX_PLAYERTAG_SHADOW_OFFSET_X (-1.0F)
#define IFREMIX_PLAYERTAG_SHADOW_OFFSET_Y (-1.65F)
#define IFREMIX_PLAYERTAG_SHADOW_GLYPH_SCALE 1.25F
#define IFREMIX_PLAYERTAG_ALPHA_MASK 0xFF
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
 * the tag, centred on the indicator sprite at its top, in the colour of the tag, over a black copy 1.25 times larger. Both
 * layers take the alpha of the indicator sprite (position_custom_tag_ copies it to every character each frame), so the
 * name fades with the arrow of a cloaked or invisible fighter. Draws nothing for the default tag or an unnamed tag (the
 * asm skips a name that is only spaces).
 *
 * @param player Port index (0..3).
 * @param center_x Screen X of the middle of the indicator sprite.
 * @param top_y Screen Y of the top of the indicator sprite.
 * @param rgba Tag colour and indicator sprite alpha as 0xRRGGBBAA.
 */
void ifRemixPlayerTagDrawName(s32 player, f32 center_x, f32 top_y, u32 rgba)
{
    const char *name = remix_panel_tag_name(gmRemixPanel(PLAYER_TAG, player));

    if (name[0] == '\0')
    {
        return;
    }
    remix_hud_text_put_fit_glyph_scale(IFREMIX_PLAYERTAG_SHADOW_SLOT + player, name, center_x + IFREMIX_PLAYERTAG_SHADOW_OFFSET_X,
                                       top_y + IFREMIX_PLAYERTAG_SHADOW_OFFSET_Y, rgba & IFREMIX_PLAYERTAG_ALPHA_MASK, 1.0F,
                                       REMIX_HUD_ALIGN_CENTER, IFREMIX_PLAYERTAG_MAX_WIDTH, IFREMIX_PLAYERTAG_SHADOW_GLYPH_SCALE);
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
