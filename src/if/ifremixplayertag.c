/**
 * ifremixplayertag.c - Smash Remix Player Tag option (css/PlayerTag.asm, src/PlayerTag.asm use_custom_tag_image_ and
 * always_display_custom_tag_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <gm/gmremixpanel.h>

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
 * TRUE when the port has a custom tag, which keeps the tag shown whatever its wait countdown is
 * (src/PlayerTag.asm always_display_custom_tag_).
 */
sb32 ifRemixPlayerTagAlwaysShown(s32 player)
{
    return gmRemixPanel(PLAYER_TAG, player) != 0;
}

#endif
