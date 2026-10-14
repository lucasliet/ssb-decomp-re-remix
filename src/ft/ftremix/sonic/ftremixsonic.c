/**
 * ftremixsonic.c - C ports of Smash Remix Sonic routines
 * (.toolchain/pkgs/smashremix-src/src/Sonic/).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"

/* Sonic.classic_table (Sonic.asm, Remix RAM 0x80563BC0): one byte per port,
 * TRUE when that player picked the classic Sonic model. Read by the Spin Dash
 * charge FGM choice (SonicDSP, SonicSpecial.asm 2133/2219) and the Spring
 * Jump spring image/FGM choice (SonicUSP, spring_stage_setting_).
 *
 * In Remix the table is written by the save/load/reset_classic_* patches
 * (Sonic.asm 739-870, 1001-1032), which hook character-select, 1p-save and
 * training code owned by other workers. Those ports should call
 * ftRemixSonicSetClassic; until one lands the table stays 0, which matches
 * the previous never-written per-file statics. */
static u8 sFTRemixSonicIsClassic[MAXCONTROLLERS];

/* TRUE when `player` is using the classic Sonic model. */
sb32 ftRemixSonicIsClassic(s32 player)
{
    return sFTRemixSonicIsClassic[player] != FALSE;
}

/* Sets `player`'s classic-model flag (the save/load/reset_classic_* port side). */
void ftRemixSonicSetClassic(s32 player, sb32 is_classic)
{
    sFTRemixSonicIsClassic[player] = (is_classic != FALSE);
}

/* Remix address -> port routine for every Sonic routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixSonicProcs[] =
{
    { 0, NULL }
};

const s32 dFTRemixSonicProcsNum = 0;

#endif
