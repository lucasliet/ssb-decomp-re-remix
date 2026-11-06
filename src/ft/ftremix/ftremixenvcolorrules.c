/**
 * ftremixenvcolorrules.c - pure rule of the Remix env colour opacity, see ftremixenvcolorrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixenvcolorrules.h"

#define FTREMIX_ENVCOLOR_ALPHA_OPAQUE 0xFFu

/** TRUE when an env colour override makes the fighter parts use the alpha render mode. */
int ftRemixEnvColorRulesForcesTranslucent(unsigned int rgba)
{
    if (rgba == 0)
    {
        return 0;
    }
    return (rgba & 0xFFu) != FTREMIX_ENVCOLOR_ALPHA_OPAQUE;
}

#endif
