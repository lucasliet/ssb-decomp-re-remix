#include <mn/mncommon/mnremixtitleversion.h>

#if defined(PORT) && defined(REGION_REMIX)
#include "remix/remix_hud_text.h"

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
