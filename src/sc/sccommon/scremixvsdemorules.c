/**
 * scremixvsdemorules.c - state-free rules of the Remix VS demo fighter pick, see scremixvsdemorules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sccommon/scremixvsdemorules.h"

int scRemixVsDemoRepeatsPort3Fighter(int player, int fkind, int port_3_fkind)
{
    return ((player == SC_REMIX_VS_DEMO_PORT_4_PLAYER) && (fkind == port_3_fkind)) ? 1 : 0;
}

#endif
