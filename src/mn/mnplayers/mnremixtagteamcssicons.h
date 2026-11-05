#ifndef _MNREMIXTAGTEAMCSSICONS_H_
#define _MNREMIXTAGTEAMCSSICONS_H_

#include <ssb_types.h>
#include "mn/mnplayers/mnremixtagteamcss.h"
#include "mn/mnplayers/mnremixtagteamcssrules.h"

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Makes the icon rows of the four panels and the process that keeps them up to date (setup_ and slot_icon_update_routine_).
 *
 * @param queues Queues of the four ports.
 * @param info_func Gives the state of a panel.
 */
extern void mnRemixTagTeamCssIconsMake(const MNRemixTagTeamCssQueue *queues, MNRemixTagTeamCssPortInfoFunc info_func);

#endif

#endif
