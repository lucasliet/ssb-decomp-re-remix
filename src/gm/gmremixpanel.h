#ifndef _GMREMIXPANEL_H_
#define _GMREMIXPANEL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "remix/remix_player_panel.h"

/*
 * Smash Remix CSS panel (the css asm files) value of one port. `gmRemixPanel(SIZE, player)` is the Size option of that port,
 * `name` being REMIX_PANEL_<name>; the value is the one set in the port menu (Remix > Player Panel), the CVar default,
 * or the SSB64_REMIX_PANEL override of the run. The Bonus and 1P Game modes ignore the panel, as the asm does for the
 * options that clear themselves before them.
 */
#define gmRemixPanel(name, player) gmRemixPanelGet(REMIX_PANEL_##name, (player))

extern s32 gmRemixPanelGet(s32 option, s32 player);

#endif

#endif
