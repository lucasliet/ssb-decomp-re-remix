/**
 * ftremixsonic.c - C ports of Smash Remix Sonic routines
 * (.toolchain/pkgs/smashremix-src/src/Sonic/).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"

/* Sonic.classic_table (Sonic.asm, Remix RAM 0x80563BC0): one byte per port,
 * TRUE when that player picked the classic Sonic model. Read by the Spin Dash
 * charge FGM choice (SonicDSP, SonicSpecial.asm 2133/2219), the Spring
 * Jump spring image/FGM choice (SonicUSP, spring_stage_setting_), the classic
 * main file, FGM swaps and shield colour (ftremixsonicclassic.c).
 *
 * In Remix the table is written by the CSS toggle (Costumes.asm select_) and the
 * save/load/reset_classic_* patches (Sonic.asm 739-997), ported in
 * mn/mnplayers/mnremixsonicclassic.c, which call ftRemixSonicSetClassic. The
 * flag of a port is cleared when a fighter other than Sonic is made for it
 * (load_classic_model_, ftRemixSonicClassicGetMainFile). */
static u8 sFTRemixSonicIsClassic[MAXCONTROLLERS];

/* TRUE when `player` is using the classic Sonic model. */
sb32 ftRemixSonicIsClassic(s32 player)
{
    if ((player < 0) || (player >= MAXCONTROLLERS))
    {
        return FALSE;
    }
    return sFTRemixSonicIsClassic[player] != FALSE;
}

/* Sets `player`'s classic-model flag (the save/load/reset_classic_* port side). */
void ftRemixSonicSetClassic(s32 player, sb32 is_classic)
{
    if ((player < 0) || (player >= MAXCONTROLLERS))
    {
        return;
    }
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
