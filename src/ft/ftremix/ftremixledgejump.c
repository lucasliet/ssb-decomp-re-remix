/*
 * ftremixledgejump.c - Edge C-Jumping toggle (Smash Remix LedgeJump.asm). A C button tapped while hanging from a ledge
 * starts the cliff climb, and the Quick and Slow cliff proc updates turn that queued status into a jump. Every helper
 * is a no-op while the toggle is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"

/* LedgeJump.asm check_input_ queues this status: 3 + the Quick kind (0) and 3 + the Slow kind (3) both fall through
 * to the default case of the cliff proc updates, since the vanilla kinds are 0..2 and 3..5 */
#define FTREMIX_LEDGEJUMP_CLIFF_INPUT 3

/* LedgeJump.asm jump_initial_: 0x42A00000 (80.0F), the max stick jump height */
#define FTREMIX_LEDGEJUMP_JUMP_FORCE 80.0F

/**
 * Queues the ledge climb when a C button is tapped (LedgeJump.asm check_input_, 0x80144FE8 call). Returns TRUE when a
 * C button was tapped, and the caller then skips the fall check of CliffWait.
 */
sb32 ftRemixLedgeJumpCheckCButtons(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!gmRemixToggle(ledge_jump))
    {
        return FALSE;
    }
    if ((fp->input.pl.button_tap & (L_CBUTTONS | R_CBUTTONS | U_CBUTTONS | D_CBUTTONS)) == 0)
    {
        return FALSE;
    }
    ftCommonCliffQuickOrSlowSetStatus(fighter_gobj, FTREMIX_LEDGEJUMP_CLIFF_INPUT);

    return TRUE;
}

/**
 * Starts a stick jump from the ledge when the cliff motion holds the queued status (LedgeJump.asm jump_initial_, called
 * from the default case of the Quick and Slow cliff proc updates).
 */
void ftRemixLedgeJumpCliffMotionDefault(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!gmRemixToggle(ledge_jump))
    {
        return;
    }
    fp->status_vars.common.kneebend.jump_force = FTREMIX_LEDGEJUMP_JUMP_FORCE;
    fp->status_vars.common.kneebend.is_shorthop = FALSE;
    fp->status_vars.common.kneebend.input_source = FTCOMMON_KNEEBEND_INPUT_TYPE_STICK;
    ftCommonJumpSetStatus(fighter_gobj);
}

#endif
