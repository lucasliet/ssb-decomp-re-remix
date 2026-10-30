#ifndef _FTREMIXAISPECIALS_H_
#define _FTREMIXAISPECIALS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Charged NSP hooks of the Remix AI (AI.asm improve_remix_charged_NSP_1 and _2). */

/** improve_remix_charged_NSP_1: presses Z to cancel a charging NSP for DSamus, Sheik and Mewtwo. TRUE when Z was pressed. */
sb32 ftRemixAiSpecialsCancelChargeWithZ(FTStruct *fp);

/** improve_remix_charged_NSP_2: TRUE when the opponent blocks the Samus NSP (Fox-like always, Ness-like unless Sheik). */
sb32 ftRemixAiSamusNspIsBlocked(FTStruct *fp, FTStruct *opponent);

#endif

#endif
