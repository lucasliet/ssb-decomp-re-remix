/**
 * ftremixhandicap.c - Smash Remix Handicap.asm: the CPU handicap override of the Remix CSS panel.
 *
 * use_handicap_ (asm 0x800D816C, ROM 0x5396C) writes the panel value into the handicap byte of the fighter description
 * of a CPU player when the value is not 0. ftManagerInitFighter copies that byte into the fighter.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmremixpanel.h>

/* The CSS handicap indicator (AI.asm 6896, ledger FT-11) draws this value for CPU players. */
u8 ftRemixHandicapGetFighterValue(FTStruct *fp, u8 vanilla_handicap)
{
    s32 panel_value;

    if (fp->pkind != nFTPlayerKindCom)
    {
        return vanilla_handicap;
    }
    panel_value = gmRemixPanel(HANDICAP, fp->player);
    if (panel_value == 0)
    {
        return vanilla_handicap;
    }
    return (u8)panel_value;
}

#endif
