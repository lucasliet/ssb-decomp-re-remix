/**
 * ftremixcaptureinterrupt.c - Wario and Marina custom_capture_dk_interrupt rows, see ftremixcaptureinterrupt.h.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixcaptureinterrupt.h"

#define FTREMIX_ACTION_THROW_F 0xA9 /* Action.ThrowF, Action.asm line 174 */

/**
 * Wario.asm capture_action_fix_: the break-out routine is always skipped, so a fighter held in ThrownDK cannot mash free.
 *
 * @param grabber_fp Wario's fighter struct.
 * @return TRUE.
 */
s32 ftRemixWarioCaptureDKInterrupt(FTStruct *grabber_fp)
{
    (void) grabber_fp;

    return TRUE;
}

/**
 * Marina.asm capture_action_fix_: the break-out routine is skipped only while Marina is in ThrowF.
 *
 * @param grabber_fp Marina's fighter struct.
 * @return TRUE during ThrowF, else FALSE (vanilla break-out).
 */
s32 ftRemixMarinaCaptureDKInterrupt(FTStruct *grabber_fp)
{
    return (grabber_fp->status_id == FTREMIX_ACTION_THROW_F) ? TRUE : FALSE;
}

#endif
