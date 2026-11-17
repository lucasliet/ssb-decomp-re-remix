/**
 * ifremixplayertag.c - Smash Remix Player Tag option (css/PlayerTag.asm, src/PlayerTag.asm use_custom_tag_image_,
 * create_custom_tag_ and always_display_custom_tag_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <gm/gmremixpanel.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "if/ifremixtextlayer.h"
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

/* The battle tags are SObjs of the interface group (HUD elements, link 11) drawn by the interface camera (display link 23) */
#define IFREMIX_PLAYERTAG_BATTLE_GOBJ_LINK 11
#define IFREMIX_PLAYERTAG_BATTLE_DL_LINK 23

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
 * Makes the layer that draws the tag names of the four ports (the black copies, then the names) in the room of the tag
 * sprites, so what the scene draws over the tags (the results tints and the Match Stats page) also covers the names, as it
 * does in the asm where the characters are images of the tag. Call after the tag objects of the room are made.
 *
 * @param gobj_link GObj link (group) of the tag objects.
 * @param dl_link Display link (room) of the tag objects, drawn by a sprite camera.
 */
void ifRemixPlayerTagMakeNameLayer(s32 gobj_link, s32 dl_link)
{
    GObj *layer_gobj = ifRemixTextLayerMake(gobj_link, dl_link, GOBJ_PRIORITY_DEFAULT);

    ifRemixTextLayerBindLabels(layer_gobj, IFREMIX_PLAYERTAG_SHADOW_SLOT, (IFREMIX_PLAYERTAG_TEXT_SLOT - IFREMIX_PLAYERTAG_SHADOW_SLOT) * 2);
}

/**
 * Makes the layer of the tag names in the battle, over the tag arrows and under what the scene makes after the tags (the
 * Remix meters, the music title banner, the countdown, GO! and GAME SET), as the asm characters sort against them. Nothing is
 * made when no fighter has a custom tag, so the font file is not loaded for nothing. Call right after the tag objects.
 */
void ifRemixPlayerTagMakeBattleLayer(void)
{
    s32 player;

    for (player = 0; player < ARRAY_COUNT(gSCManagerBattleState->players); player++)
    {
        if ((gSCManagerBattleState->players[player].pkind != nFTPlayerKindNot) && (gmRemixPanel(PLAYER_TAG, player) != 0))
        {
            ifRemixPlayerTagMakeNameLayer(IFREMIX_PLAYERTAG_BATTLE_GOBJ_LINK, IFREMIX_PLAYERTAG_BATTLE_DL_LINK);
            return;
        }
    }
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
