/**
 * gmremixaccessibility.c - Accessibility.asm ParamMakeEffect store and the Remix RTTF branch of screenshake_toggle.
 * See gmremixaccessibility.h for the patch addresses.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gr/grremix/grremixids.h>
#include <sc/scene.h>
#include "gm/gmremixaccessibility.h"

/* Accessibility.asm ParamMakeEffect.gobj: the fighter of the running ftParamMakeEffect call, NULL outside it. */
static FTStruct *sGMRemixAccessibilityEffectFighter;

/** Stores `fp` as the fighter of the ftParamMakeEffect call that is starting. */
void gmRemixAccessibilityParamEffectBegin(FTStruct *fp)
{
    sGMRemixAccessibilityEffectFighter = fp;
}

/** Clears the stored fighter when ftParamMakeEffect returns. */
void gmRemixAccessibilityParamEffectEnd(void)
{
    sGMRemixAccessibilityEffectFighter = NULL;
}

/** TRUE when the quake must not shake: Remix RTTF and the stored fighter is off screen. */
sb32 gmRemixAccessibilityIsQuakeSuppressed(void)
{
    if (gSCManagerBattleState->gkind != nGRRemixStage_REMIX_RTTF)
    {
        return FALSE;
    }
    if (sGMRemixAccessibilityEffectFighter == NULL)
    {
        return FALSE;
    }
    return (sGMRemixAccessibilityEffectFighter->is_magnify_show != FALSE) ? TRUE : FALSE;
}

#endif
