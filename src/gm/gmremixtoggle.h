#ifndef _GMREMIXTOGGLE_H_
#define _GMREMIXTOGGLE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "remix/remix_toggles.h"
#include "remix/remix_symbols.h"

/*
 * Smash Remix settings (Toggles.asm). `gmRemixToggle(hitstun)` is the value of Toggles.entry_hitstun, `name` being the label
 * of the toggle in Toggles.asm without the `entry_` prefix; its symbol is REMIX_SYM_Toggles__entry_<name> in
 * port/remix/remix_symbols.h. The value is the one set in the port menu (Remix section), the CVar default, or the
 * SSB64_REMIX_TOGGLES override of the run.
 */
#define gmRemixToggle(name) remix_toggle_get(REMIX_SYM_Toggles__entry_##name)

/* Toggles.single_player_guard: the Bonus and 1P Game modes ignore the toggle, as if it were off. */
#define gmRemixToggleVS(name) (gmRemixToggleModeIsGuarded() ? 0 : gmRemixToggle(name))

extern sb32 gmRemixToggleModeIsGuarded(void);

#endif

#endif
