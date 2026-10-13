/**
 * ftremixbanjo.c - C ports of Smash Remix Banjo routines
 * (.toolchain/pkgs/smashremix-src/src/Banjo/).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"

/* Remix address -> port routine for every Banjo routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixBanjoProcs[] =
{
    { 0, NULL }
};

const s32 dFTRemixBanjoProcsNum = 0;

#endif
