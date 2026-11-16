/**
 * ftremixmarina.c - C ports of Smash Remix Marina routines
 * (.toolchain/pkgs/smashremix-src/src/Marina/).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/marina/ftremixmarinarules.h"
#include "remix/remix_symbols.h"

/** True for Marina and N. Marina, the two fighters the Remix jump patches treat as Marina. */
static sb32 ftRemixMarinaIsJumpFighter(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_MARINA) || (fp->fkind == REMIX_ID_NMARINA);
}

/**
 * Marina.asm jump_height_patch_ (vanilla 0x8013FE58): stick term of the midair jump height.
 *
 * @param fp Fighter starting a midair jump, before jumps_used is incremented.
 * @param stick_range_y Vanilla stick term (80).
 * @return jump_height_table[jumps_used] for (N)Marina, else stick_range_y.
 */
s32 ftRemixMarinaJumpAerialStickRange(FTStruct *fp, s32 stick_range_y)
{
    if (ftRemixMarinaIsJumpFighter(fp) == FALSE)
    {
        return stick_range_y;
    }
    return ftRemixMarinaJumpStickRange(fp->jumps_used);
}

/**
 * Marina.asm jump_timing_patch_ (vanilla 0x80140310): refuses a midair jump out of JumpAerialF/B until the jump
 * moveset sets temp variable 2; a jump that starts clears the variable.
 *
 * @param fp Fighter about to start a midair jump.
 * @return TRUE when the jump is refused. Always FALSE for other fighters.
 */
sb32 ftRemixMarinaJumpAerialIsLocked(FTStruct *fp)
{
    if (ftRemixMarinaIsJumpFighter(fp) == FALSE)
    {
        return FALSE;
    }
    if (ftRemixMarinaJumpIsLocked(fp->status_id, fp->motion_vars.flags.flag1) != 0)
    {
        return TRUE;
    }
    fp->motion_vars.flags.flag1 = 0;
    return FALSE;
}

/* Remix address -> port routine for every Marina routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixMarinaProcs[] =
{
    { 0, NULL }
};

const s32 dFTRemixMarinaProcsNum = 0;

#endif
