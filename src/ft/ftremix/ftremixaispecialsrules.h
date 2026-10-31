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

/** TRUE when a Bowser or GBowser action is the grounded up special whose stick X the CPU zeroes (AI.asm improve_remix_specials). */
extern int ftRemixAiSpecialsIsBowserUspGround(int action);

/**
 * TRUE when the Samus NSP is blocked by the opponent: the opponent always blocks it, or it blocks unless the own fighter
 * is Sheik (AI.asm improve_remix_charged_NSP_2).
 */
extern int ftRemixAiSamusNspOpponentBlocks(int always_blocks, int sheik_exempt_blocks, int own_is_sheik);

#endif

#endif
