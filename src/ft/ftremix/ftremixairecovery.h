#ifndef _FTREMIXAIRECOVERY_H_
#define _FTREMIXAIRECOVERY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Recovery hooks of the Remix AI (AI.asm recovery_fix_ and fix_remix_recovery). */

/** Vanilla 1P/RTTF (classic mode), the game type that keeps the vanilla recovery rules. */
sb32 ftRemixAiRecoveryIsClassicMode(void);

/** recovery_fix_: TRUE when the CPU may repeat its up-B recovery, skipping the vanilla "already used" check. */
sb32 ftRemixAiRecoveryAllowsUpBRepeat(FTStruct *fp);

/** fix_remix_recovery: TRUE when the vanilla Pikachu recovery block runs for fp. */
sb32 ftRemixAiRecoveryVanillaPikachuApplies(FTStruct *fp);

/** fix_target_pos_x: clamps the walk target X inside the stage bounds when the improved AI is on. */
void ftRemixAiRecoveryClampWalkTargetX(FTStruct *fp);

/** fix_remix_recovery: the Wolf USP actions aim the recovery target straight up. */
void ftRemixAiRecoveryWolfTarget(FTStruct *fp);

/** fix_remix_recovery: TRUE when the NSP charge cancel handled this frame, so the walk and the custom recovery are skipped. */
sb32 ftRemixAiRecoverySpecialCase(FTStruct *fp);

#endif

#endif
