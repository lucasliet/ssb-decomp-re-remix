#ifndef _GMREMIXACCESSIBILITY_H_
#define _GMREMIXACCESSIBILITY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>

/*
 * Accessibility.asm ParamMakeEffect and screenshake_toggle (Remix RTTF branch). The fighter of the running ftParamMakeEffect call
 * is stored on entry and cleared on every return, so a quake made inside that call knows its fighter.
 */

/** Stores `fp` as the fighter of the ftParamMakeEffect call that is starting (Accessibility.asm save_gobj, RAM 0x800EABF4). */
void gmRemixAccessibilityParamEffectBegin(FTStruct *fp);

/** Clears the stored fighter when ftParamMakeEffect returns (Accessibility.asm clear_gobj, RAM 0x800EB388). */
void gmRemixAccessibilityParamEffectEnd(void);

/** TRUE when the quake must not shake: Remix RTTF and the stored fighter is off screen (Accessibility.asm screenshake_toggle). */
sb32 gmRemixAccessibilityIsQuakeSuppressed(void);

#endif

#endif
