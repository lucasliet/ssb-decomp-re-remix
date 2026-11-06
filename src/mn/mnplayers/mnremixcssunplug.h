#ifndef _MNREMIXCSSUNPLUG_H_
#define _MNREMIXCSSUNPLUG_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/**
 * B tap that backs out of the 1P, Training and Bonus character selects: the tap of the player's own controller, or the tap of any
 * controller when the player's one was unplugged, so the screen cannot strand the player after a hot unplug (_1P_unplugged_b_handler).
 *
 * @param player Port of the player of the screen.
 * @return TRUE when B counts as tapped.
 */
extern sb32 mnRemixCssUnplugIsBackTapped(s32 player);

#endif

#endif
