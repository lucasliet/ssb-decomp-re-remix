/**
 * mnremixstagepager.c - page strip of the Remix stage select (src/Stages.asm setup_ lines 2845-2856).
 *
 * Under the 3 x 6 grid Remix draws, from left to right: an arrow and the Z button, "Page N", the R button and an arrow. The
 * arrows are sprites of the players common file, the buttons are the pause interface sprites and the text goes through the HUD
 * overlay (remix_hud_text).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <reloc_data.h>
#include "if/ifremixsprite.h"
#include "mn/mnmaps/mnremixstagegridrules.h"
#include "mn/mnmaps/mnremixstagepager.h"
#include "remix/remix_hud_text.h"

/* Text slots 126 and 127 of remix_hud_text (stage select scene), next to the panel slots 118..125. */
#define PAGER_TEXT_SLOT_PAGE 126
#define PAGER_TEXT_SLOT_NUMBER 127

/* The sprites share the GObj link 4 and the name and emblem camera (dl link 2) with the stage panel. */
#define PAGER_GOBJ_LINK 4
#define PAGER_DL_LINK 2

/* Stages.asm setup_ draw_texture_at_offset arguments. */
#define PAGER_ARROW_LEFT_X 29.0F
#define PAGER_ARROW_RIGHT_X 276.0F
#define PAGER_ARROW_Y 117.0F
#define PAGER_ARROW_SCALE 0.625F
#define PAGER_ARROW_TINT 0xC0CC00FF
#define PAGER_ARROW_PALETTE 0x000000FF
#define PAGER_Z_BUTTON_X 36.0F
#define PAGER_Z_BUTTON_Y 113.0F
#define PAGER_R_BUTTON_X 257.0F
#define PAGER_R_BUTTON_Y 114.5F
#define PAGER_BUTTON_SCALE 1.0F
#define PAGER_BUTTON_TINT 0x848484FF
#define PAGER_BUTTON_PALETTE 0x303030FF

/* Stages.asm setup_ draw_string and draw_number_adjusted arguments: white, scale 0.875, left aligned. */
#define PAGER_TEXT_LABEL_X 136.0F
#define PAGER_TEXT_NUMBER_X 168.0F
#define PAGER_TEXT_Y 115.5F
#define PAGER_TEXT_SCALE 0.875F
#define PAGER_TEXT_COLOR 0xFFFFFFFF

void mnRemixStagePagerMake(void *arrows_file)
{
    ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowLSprite, PAGER_GOBJ_LINK, PAGER_DL_LINK, PAGER_ARROW_LEFT_X, PAGER_ARROW_Y,
                              PAGER_ARROW_TINT, PAGER_ARROW_PALETTE, PAGER_ARROW_SCALE);
    ifRemixSpriteMakeFromFile(arrows_file, llMNPlayersCommonArrowRSprite, PAGER_GOBJ_LINK, PAGER_DL_LINK, PAGER_ARROW_RIGHT_X, PAGER_ARROW_Y,
                              PAGER_ARROW_TINT, PAGER_ARROW_PALETTE, PAGER_ARROW_SCALE);
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_Z, PAGER_GOBJ_LINK, PAGER_DL_LINK, PAGER_Z_BUTTON_X, PAGER_Z_BUTTON_Y, PAGER_BUTTON_TINT,
                      PAGER_BUTTON_PALETTE, PAGER_BUTTON_SCALE);
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_R, PAGER_GOBJ_LINK, PAGER_DL_LINK, PAGER_R_BUTTON_X, PAGER_R_BUTTON_Y, PAGER_BUTTON_TINT,
                      PAGER_BUTTON_PALETTE, PAGER_BUTTON_SCALE);
}

void mnRemixStagePagerUpdate(s32 page)
{
    remix_hud_text_put(PAGER_TEXT_SLOT_PAGE, "Page", PAGER_TEXT_LABEL_X, PAGER_TEXT_Y, PAGER_TEXT_COLOR, PAGER_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_putf(PAGER_TEXT_SLOT_NUMBER, PAGER_TEXT_NUMBER_X, PAGER_TEXT_Y, PAGER_TEXT_COLOR, PAGER_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT, "%d",
                        (int)mnRemixStageGridRulesGetPageNumber(page));
}

#endif
