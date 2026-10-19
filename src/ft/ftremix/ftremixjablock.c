/*
 * ftremixjablock.c - Jab Locking toggle (Smash Remix JabLock.asm). A pre-knockdown hit that lands during
 * DownBounceD/U restarts that action instead of playing the damage status, and the lock it sets makes the
 * bounce end in DownStandD/U. Every helper is a no-op while the toggle is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"

#define FTREMIX_JABLOCK_LOCK_OFFSET 0x0B28

/** Clears the lock when a DownBounce starts (JabLock.asm down_initial_, 0x80144504). */
void ftRemixJabLockClear(FTStruct *fp)
{
    if (gmRemixToggle(jab_lock))
    {
        *ftRemixFighterWord(fp, FTREMIX_JABLOCK_LOCK_OFFSET) = FALSE;
    }
}

/**
 * Turns a pre-knockdown damage status into a restart of the current DownBounce and sets the lock
 * (JabLock.asm handle_lock_, 0x8014139C). status_id_set is the damage status about to be set.
 */
void ftRemixJabLockDamageStatus(FTStruct *fp, s32 *status_id_set)
{
    if (!gmRemixToggle(jab_lock))
    {
        return;
    }
    if ((fp->status_id != nFTCommonStatusDownBounceD) && (fp->status_id != nFTCommonStatusDownBounceU))
    {
        return;
    }
    if (*status_id_set >= nFTCommonStatusDamageE2)
    {
        return;
    }
    *status_id_set = fp->status_id;
    *ftRemixFighterWord(fp, FTREMIX_JABLOCK_LOCK_OFFSET) = TRUE;
}

/**
 * Ends a locked DownBounce in DownStandD/U and returns TRUE when the action changed
 * (JabLock.asm handle_reset_, 0x8014494C).
 */
sb32 ftRemixJabLockInterruptDownStand(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!gmRemixToggle(jab_lock))
    {
        return FALSE;
    }
    if (*ftRemixFighterWord(fp, FTREMIX_JABLOCK_LOCK_OFFSET) == FALSE)
    {
        return FALSE;
    }
    ftCommonDownStandSetStatus(fighter_gobj);

    return TRUE;
}

#endif
