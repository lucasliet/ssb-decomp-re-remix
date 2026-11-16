/**
 * ftremixenvcolorrules.c - pure rules of the Remix env colour override, see ftremixenvcolorrules.h.
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

/** TRUE when the env colour override replaces the stage light colour: there is one and no colanim colour 2 or fog colour. */
int ftRemixEnvColorRulesReplacesLight(unsigned int rgba, int is_use_color2, int is_use_fogcolor)
{
    if (rgba == 0)
    {
        return 0;
    }
    return (is_use_color2 == 0) && (is_use_fogcolor == 0);
}

#endif
