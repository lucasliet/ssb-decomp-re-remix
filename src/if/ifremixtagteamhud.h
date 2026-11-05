#ifndef _IFREMIXTAGTEAMHUD_H_
#define _IFREMIXTAGTEAMHUD_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

/*
 * Tag Team HUD refresh after a fighter swap (TagTeam.asm load_next_char_ _redraw_icons and "Change damage series icon"):
 * the damage HUD takes the series emblem of the new character and the stock icons are drawn again.
 */

/**
 * @param player Port whose fighter was swapped.
 * @param fighter_gobj The new fighter of the port.
 */
extern void ifRemixTagTeamRefreshHud(s32 player, GObj *fighter_gobj);

#endif

#endif
