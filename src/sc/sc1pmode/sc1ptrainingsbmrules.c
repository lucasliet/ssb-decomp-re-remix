/**
 * sc1ptrainingsbmrules.c - state-free rule of the Training gate of Single Button Mode, see sc1ptrainingsbmrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1ptrainingsbmrules.h"

int sc1PTrainingSbmGetToggleMenu(int game_status, int is_custom_menu_open)
{
    return (is_custom_menu_open != 0) ? SC1PTRAININGSBM_MENU_CUSTOM_UP : game_status;
}

int sc1PTrainingSbmIsMenuUp(int toggle_menu)
{
    return (toggle_menu >= SC1PTRAININGSBM_MENU_SSB_UP) ? 1 : 0;
}

#endif
