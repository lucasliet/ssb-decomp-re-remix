#include <mn/mncommon/mnremixtitleversion.h>

#if defined(PORT) && defined(REGION_REMIX)
#include "if/ifremixtextlayer.h"
#include "remix/remix_hud_text.h"

/**
 * Makes the layer that draws the version text in room 1 of the title (Boot.asm:160), over the logo sprites of the same
 * display link and under the black fill the title makes when it proceeds. Call once when the title starts.
 */
void mnRemixTitleVersionMakeLayer(void)
{
    GObj *layer_gobj = ifRemixTextLayerMake(MN_REMIX_TITLE_VERSION_GOBJ_LINK, MN_REMIX_TITLE_VERSION_DL_LINK, GOBJ_PRIORITY_DEFAULT);

    ifRemixTextLayerBindLabels(layer_gobj, MN_REMIX_TITLE_VERSION_SLOT, 1);
}

/**
 * Keeps the "Smash Remix v2.0.1" text of the title screen visible; call once per frame while the title runs.
 */
void mnRemixTitleVersionPut(void)
{
    remix_hud_text_put
    (
        MN_REMIX_TITLE_VERSION_SLOT,
        MN_REMIX_TITLE_VERSION_TEXT,
        MN_REMIX_TITLE_VERSION_X,
        MN_REMIX_TITLE_VERSION_Y,
        MN_REMIX_TITLE_VERSION_RGBA,
        MN_REMIX_TITLE_VERSION_SCALE,
        REMIX_HUD_ALIGN_CENTER
    );
}
#endif
