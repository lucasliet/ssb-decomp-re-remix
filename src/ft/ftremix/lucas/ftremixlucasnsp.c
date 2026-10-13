/**
 * ftremixlucasnsp.c - C ports of Smash Remix Lucas neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Lucas/LucasSpecial.asm).
 *
 * The aerial PK Fire pushes Lucas horizontally while temp variable 1
 * (motion_vars.flags.flag0, set by moveset command 5400XXXX) is non-zero.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"

/* Upper halfword of X_SPEED: 0x4220 << 16 = 0x42200000 = 40.0F. */
#define LUCAS_NSP_AIR_X_SPEED 40.0F

/* LucasNSP.air_move_: subtracts X_SPEED * facing from the x velocity while temp variable 1 is set. */
static void ftRemixLucasNSPAirMove(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 kick;

    if (fp->motion_vars.flags.flag0 == 0)
    {
        return;
    }
    kick = LUCAS_NSP_AIR_X_SPEED * fp->lr;
    fp->physics.vel_air.x = fp->physics.vel_air.x - kick;
}

/* Remix address -> port routine for every Lucas NSP routine its action array references. */
const FTRemixProc dFTRemixLucasNSPProcs[] =
{
    { REMIX_SYM_LucasNSP__air_move_, ftRemixLucasNSPAirMove },
};

const s32 dFTRemixLucasNSPProcsNum = ARRAY_COUNT(dFTRemixLucasNSPProcs);

#endif
