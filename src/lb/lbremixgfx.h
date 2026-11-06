#ifndef _LBREMIXGFX_H_
#define _LBREMIXGFX_H_

/*
 * Smash Remix GFX extension (src/GFX.asm). New moveset GFX ids 0x5C..0x7C reuse the handler of a vanilla GFX id with
 * their own particle script and texture block. See lbremixgfx.c.
 */

#include <ssb_types.h>

/* GFX.asm:495 add_ground_effect_gfx(Blue/Black Ground Effect, 0x20): GFX id 0x64 is the charge shot ground wave
 * (0x800EB0A4) drawn with impact wave index 0x20 instead of 4. */
#define LBREMIXGFX_GROUND_EFFECT_ID 0x64
#define LBREMIXGFX_GROUND_EFFECT_INDEX 0x20

/** Appends the Remix scripts and texture blocks to a freshly set up particle bank when it is the common effect bank. */
void lbRemixGfxExtendBank(s32 bank_id);

/** TRUE for a GFX id introduced by Remix (0x5C and up). */
sb32 lbRemixGfxIsNewEffect(s32 effect_id);

/**
 * Selects the script of a new GFX id for the next particle spawn and returns the vanilla GFX id whose handler must run,
 * or -1 when the id has no handler (ground effect clone, bank not extended).
 */
s32 lbRemixGfxPrepareEffect(s32 effect_id);

/**
 * Makes the next particle spawn use the script of a new GFX id (asm: storing a gfx id in GFX.current_gfx_id before a
 * vanilla effect function runs). The override is consumed by the first lbParticleMakeScriptID/lbParticleMakeCommon.
 */
void lbRemixGfxSetNextEffect(s32 effect_id);

/** Drops a script override that no particle spawn consumed. */
void lbRemixGfxClearOverride(void);

/** Returns the pending script override, or script_id when there is none; the override applies once. */
s32 lbRemixGfxTakeScriptOverride(s32 script_id);

/** Starts the SSB64_REMIX_GFX_TEST harness actor (no-op without the variable). */
void lbRemixGfxDebugStart(void);

#endif
