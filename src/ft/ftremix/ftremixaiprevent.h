#ifndef _FTREMIXAIPREVENT_H_
#define _FTREMIXAIPREVENT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* PREVENT_ATTACK routines of the Remix AI (AI.asm PREVENT_ATTACK.ROUTINE), called from ftComputerCheckDetectTarget. */

/**
 * PREVENT_ATTACK (AI.asm, jump through Character.ai_attack_prevent at 0x801334D0): effect of the routine of fp->fkind on the
 * candidate attack input_kind, as FTREMIX_AI_PREVENT_VERDICT_* (ftremixaipreventrules.h). Call it in the candidate loop of
 * ftComputerCheckDetectTarget where the vanilla `switch (this_fp->fkind)` sets is_attempt_cliffcatch: CLIFF_CHECK sets that
 * flag, SKIP is `goto l_continue`, ALLOW does nothing. It covers vanilla and Remix fighters, the table holds both.
 */
s32 ftRemixAiPreventCandidate(FTStruct *fp, s32 input_kind);

#endif

#endif
