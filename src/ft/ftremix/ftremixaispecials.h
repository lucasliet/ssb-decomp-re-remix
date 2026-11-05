#ifndef _FTREMIXAISPECIALS_H_
#define _FTREMIXAISPECIALS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Charged NSP hooks of the Remix AI (AI.asm improve_remix_charged_NSP_1 and _2). */

/** improve_remix_charged_NSP_1: presses Z to cancel a charging NSP for DSamus, Sheik and Mewtwo. TRUE when Z was pressed. */
sb32 ftRemixAiSpecialsCancelChargeWithZ(FTStruct *fp);

/**
 * improve_remix_specials (AI.asm _bowser): in the grounded USP the CPU zeroes its stick X. Returns nothing; the caller keeps
 * the vanilla "no charge" result.
 */
void ftRemixAiSpecialsBowserUspStickReset(FTStruct *fp);

/**
 * improve_remix_specials (Sheik and Peppy): charge decision of ftComputerCheckTryChargeSpecialN. Presses B (command 0xB) while
 * the ammo is below 6 and no charge action is running. Returns the value of the vanilla function: TRUE when it keeps or starts a
 * charge, FALSE when it stops.
 */
sb32 ftRemixAiSpecialsNeedleCharge(FTStruct *fp);

/** improve_remix_charged_NSP_3: TRUE when fp (fkind is its copied power) is already in a charge shot, so the far-range NSP returns FALSE. */
sb32 ftRemixAiSpecialsFarRangeIsCharging(FTStruct *fp, s32 fkind);

/** improve_remix_charged_NSP_3: TRUE when the far-range check runs the vanilla Samus action list for fkind (Samus clones and Mewtwo). */
sb32 ftRemixAiSpecialsFarRangeUsesSamusActions(s32 fkind);

/** improve_remix_charged_NSP_2: TRUE when the opponent blocks the Samus NSP (Fox-like always, Ness-like unless Sheik). */
sb32 ftRemixAiSamusNspIsBlocked(FTStruct *fp, FTStruct *opponent);

#endif

#endif
