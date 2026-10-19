#ifndef _IFREMIXHUD_H_
#define _IFREMIXHUD_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/* Disable HUD setting (Toggles.asm entry_disable_hud), if/ifremixhud.c: TRUE when the battle interface must stay hidden */
sb32 ifRemixHudDisabledAll(void);

#endif

#endif
