#ifndef _SC1PREMIXMODE_H_
#define _SC1PREMIXMODE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "sc/sc1pmode/sc1premixmodename.h"

/** Mode picked in the Remix Modes page of the 1P menu (SC1PREMIX_MODE_*); cleared when the 1P menu is left. */
extern s32 gSC1PRemixMode;

/**
 * Mode of the current 1P run: the menu choice, or SSB64_REMIX_1P_MODE (standard, bonus3, multiman, cruel, remix1p, allstar,
 * hrc) for the whole run so the smoke tests can start a mode without the menu.
 */
extern s32 sc1PRemixModeGet(void);

/** TRUE when the current 1P run is the Remix 1P mode `mode` (SC1PREMIX_MODE_*). */
extern sb32 sc1PRemixModeIs(s32 mode);

/**
 * Whether the crowd cheer at the start of a Bonus stage or a 1P game run is skipped (skip start cheer toggle set to Bonus; see
 * sc1PRemixModeSkipsBonusStartCheer). Reads the toggle, the Remix 1P mode and the scene the run came from.
 */
extern sb32 sc1PRemixIsBonusStartCheerSkipped(void);

#endif

#endif
