/*
 * ftremixledgetrump.c - Ledge Trumping toggle (Smash Remix LedgeTrump.asm). A fighter that grabs a ledge held by another
 * fighter in CliffCatch or CliffWait knocks that fighter off, and the grab goes through. Every helper is a no-op while
 * the toggle is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"

/* LedgeTrump.asm PUSH_Y_VELOCITY and PUSH_X_VELOCITY: 0x42480000 (50.0F) */
#define FTREMIX_LEDGETRUMP_PUSH_VELOCITY 50.0F

/* LedgeTrump.asm LAG_FRAMES, written to the damage status hitstun tics */
#define FTREMIX_LEDGETRUMP_LAG_TICS 30

/** Sets the holder to DamageAir3 with an outward push away from its facing direction, when it hangs from the ledge. */
static void ftRemixLedgeTrumpPushIfHanging(GObj *holder_gobj)
{
    FTStruct *holder_fp = ftGetStruct(holder_gobj);

    if ((holder_fp->status_id != nFTCommonStatusCliffCatch) && (holder_fp->status_id != nFTCommonStatusCliffWait))
    {
        return;
    }
    ftMainSetStatus(holder_gobj, nFTCommonStatusDamageAir3, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    holder_fp->physics.vel_air.y = FTREMIX_LEDGETRUMP_PUSH_VELOCITY;
    holder_fp->physics.vel_air.x = FTREMIX_LEDGETRUMP_PUSH_VELOCITY * (f32) holder_fp->lr * -1.0F;
    holder_fp->status_vars.common.damage.hitstun_tics = FTREMIX_LEDGETRUMP_LAG_TICS;
}

/**
 * Pushes the holder off the ledge when it hangs in CliffCatch or CliffWait (LedgeTrump.asm ledge_check_, 0x800DE66C).
 * Returns TRUE when the search for the ledge holder goes on, and FALSE when the vanilla grab is refused (toggle off).
 */
sb32 ftRemixLedgeTrumpSkipsHolder(GObj *holder_gobj)
{
    if (!gmRemixToggle(ledge_trump))
    {
        return FALSE;
    }
    ftRemixLedgeTrumpPushIfHanging(holder_gobj);

    return TRUE;
}

#endif
