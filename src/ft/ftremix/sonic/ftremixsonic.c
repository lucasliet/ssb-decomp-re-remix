/**
 * ftremixsonic.c - C ports of Smash Remix Sonic routines
 * (.toolchain/pkgs/smashremix-src/src/Sonic/).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"

/* Remix address -> port routine for every Sonic routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixSonicProcs[] =
{
    { 0, NULL }
};

const s32 dFTRemixSonicProcsNum = 0;

#endif
