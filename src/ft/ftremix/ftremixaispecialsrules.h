#ifndef _FTREMIXAISPECIALSRULES_H_
#define _FTREMIXAISPECIALSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure charged NSP rules of the Remix AI (AI.asm improve_remix_charged_NSP_1 and _2), so the headless unit tests
 * (tests/remix) can run them without game state. Numbers in, numbers out. Action ids are the Remix ids of Action.asm,
 * Sheik.asm (NSPG_CHARGE 0xEB, NSPA_CHARGE 0xEE, see AI.asm improve_remix_charged_NSP_3) and Mewtwo.asm. */

/**
 * TRUE when a DSamus action is a charge that the CPU cancels with Z: the charge shot, or the Kirby copy charges
 * (AI.asm improve_remix_charged_NSP_1, dsamus).
 */
extern int ftRemixAiSpecialsDSamusCancels(int action);

/** TRUE when a Sheik action is a needle charge the CPU cancels with Z (AI.asm improve_remix_charged_NSP_1, sheik). */
extern int ftRemixAiSpecialsSheikCancels(int action);

/** TRUE when a Mewtwo action is a shadow ball charge the CPU cancels with Z (AI.asm improve_remix_charged_NSP_1, mewtwo). */
extern int ftRemixAiSpecialsMewtwoCancels(int action);

/** Result of the Sheik and Peppy charge decision (AI.asm improve_remix_specials, _check_needle and _check_revolver). */
#define FTREMIX_AI_SPECIALS_NEEDLE_STOP 0  /* no charge now: the vanilla function returns FALSE */
#define FTREMIX_AI_SPECIALS_NEEDLE_KEEP 1  /* already starting or holding the charge: returns TRUE, no new command */
#define FTREMIX_AI_SPECIALS_NEEDLE_PRESS 2 /* below the full ammo: presses B (command 0xB) and returns TRUE */

/**
 * Sheik and Peppy charge decision for the action id and the ammo word (N64 0x0AE0). A Sheik whose action matches none of
 * her own falls into the Peppy checks, as the asm does ("don't add any lines here").
 */
extern int ftRemixAiSpecialsNeedleChargeOutcome(int is_sheik, int action, int ammo);

/**
 * TRUE when a Samus-like or Sheik fighter is already in a charge shot action, so the far-range NSP must not start
 * (AI.asm improve_remix_charged_NSP_3, the own-fighter exit). fkind is the copied power for a Kirby.
 */
extern int ftRemixAiSpecialsChargeShotActive(int fkind, int action);

/** TRUE when the far-range check also runs the vanilla Samus action list for fkind (Samus, JSamus, ESamus, DSamus, Mewtwo). */
extern int ftRemixAiSpecialsUsesSamusActionList(int fkind);

/** TRUE when a Bowser or GBowser action is the grounded up special whose stick X the CPU zeroes (AI.asm improve_remix_specials). */
extern int ftRemixAiSpecialsIsBowserUspGround(int action);

/**
 * TRUE when the Samus NSP is blocked by the opponent: the opponent always blocks it, or it blocks unless the own fighter
 * is Sheik (AI.asm improve_remix_charged_NSP_2).
 */
extern int ftRemixAiSamusNspOpponentBlocks(int always_blocks, int sheik_exempt_blocks, int own_is_sheik);

#endif

#endif
