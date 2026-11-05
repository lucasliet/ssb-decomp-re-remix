#ifndef _FTREMIXCAPTUREINTERRUPT_H_
#define _FTREMIXCAPTUREINTERRUPT_H_

#ifdef PORT

/*
 * custom_capture_dk_interrupt rows (Character.asm, read by CustomGrabAction.asm capture_dk_break_fix_ at 0x8014E4D4 in
 * ftCommonCaptureShoulderedProcInterrupt). The grabber's routine returns TRUE to skip the mash and break-out logic of the
 * fighter it holds. Mad Piano's row is ftRemixPianoCaptureDKInterrupt in ftremixhooks.h.
 */

#include <ft/fighter.h>

/* Wario.asm:629 capture_action_fix_ */
s32 ftRemixWarioCaptureDKInterrupt(FTStruct *grabber_fp);

/* Marina.asm:685 capture_action_fix_ */
s32 ftRemixMarinaCaptureDKInterrupt(FTStruct *grabber_fp);

#endif

#endif
