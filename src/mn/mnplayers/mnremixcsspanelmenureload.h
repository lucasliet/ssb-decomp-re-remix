#ifndef _MNREMIXCSSPANELMENURELOAD_H_
#define _MNREMIXCSSPANELMENURELOAD_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Runs the change handler of a panel menu item on the fighter shown on the CSS for a port (the onchange_handler of
 * css/ModelDisplay.asm, css/KirbyHat.asm and css/Visibility.asm): Model reloads the poly detail of the model, Kirby Hat swaps the
 * hat of a Kirby, Visibility sets the env colour override of the port. Other items do nothing. Call it after the value of the item
 * was stored; the fighter is the one the CSS made for the port, and a port with no fighter only gets the Visibility change.
 *
 * @param item MNRemixPanelMenuItem that changed.
 * @param port Port whose option changed.
 * @param value New value of the item.
 */
extern void mnRemixPanelMenuApplyFighterChange(s32 item, s32 port, s32 value);

#endif

#endif
