#ifndef _FTREMIXDEDEDEINHALERULES_H_
#define _FTREMIXDEDEDEINHALERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the action ids of the Dedede inhale (Dedede/DededeSpecial.asm:859-2435). Dedede and a Kirby with the Dedede
 * power run Kirby's vanilla SpecialN routines; the asm patches the 27 status changes in them so they set the Dedede
 * action (Dedede.Action.NSP_*) or Kirby's copy of it (Kirby.Action.DEDEDE_NSP_*). Only int, so the headless unit tests
 * (tests/remix) can run them.
 */

/** First hat id that sends a Kirby to the Dedede inhale actions (_kirby_hat_check: sltiu at, at, 0x0020). */
#define FTREMIX_DEDEDE_INHALE_KIRBY_HAT_MIN 0x20

/**
 * Action a status change of Kirby's SpecialN routines sets on Dedede.
 *
 * @param status_id Vanilla Kirby status the routine asks for.
 * @return The Dedede action for the SpecialN statuses 0x10D-0x11E except the two Copy ones, else status_id.
 */
extern int ftRemixDededeInhaleStatus(int status_id);

/**
 * Action a status change of Kirby's SpecialN routines sets on a Kirby (_kirby_hat_check).
 *
 * @param hat_id Hat id byte of Kirby (N64 FTStruct 0x0980, signed as `lb` reads it).
 * @param status_id Vanilla Kirby status the routine asks for.
 * @return Kirby's DEDEDE_NSP_* action when the hat id is not below FTREMIX_DEDEDE_INHALE_KIRBY_HAT_MIN (unsigned compare)
 * and the status is a SpecialN one except the two Copy ones, else status_id.
 */
extern int ftRemixDededeInhaleKirbyStatus(int hat_id, int status_id);

#endif

#endif
