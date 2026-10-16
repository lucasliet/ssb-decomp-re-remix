#ifndef _GMREMIXCOLSCRIPTS_H_
#define _GMREMIXCOLSCRIPTS_H_

/*
 * Smash Remix GFX routines (src/GFXRoutine.asm): colour animations with ids from 0x56, appended to dGMColScriptsDescs.
 */

#include <ssb_types.h>

/** Loads the Remix routines into dGMColScriptsDescs once; returns TRUE when ids >= 0x56 are available. */
sb32 gmRemixColScriptsEnsureLoaded(void);

/** Remix routine that must replace the colour animation of a port when its animation ends (0 = none). */
s32 gmRemixColScriptsGetOverride(s32 port);

/** Sets the routine that is reapplied to a port when its colour animation ends (asm GFXRoutine.port_override). */
void gmRemixColScriptsSetOverride(s32 port, s32 colanim_id);

/** Clears every port override (asm clear_gfx_override_table_). */
void gmRemixColScriptsClearOverrides(void);

#endif
