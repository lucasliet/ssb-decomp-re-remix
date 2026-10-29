#ifndef _FTREMIXAIPREVENT_H_
#define _FTREMIXAIPREVENT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* PREVENT_ATTACK routines of the Remix AI (AI.asm PREVENT_ATTACK.ROUTINE), called from ftComputerCheckDetectTarget. */

/**
 * PREVENT_ATTACK (AI.asm): TRUE vetoes the candidate attack input_kind before it is committed, for the Remix routine of
 * fp->fkind. Fighters whose routine is still REMIX-TODO(AI) never veto (see ftremixaiprevent.c).
 */
sb32 ftRemixAiPreventAttack(FTStruct *fp, s32 input_kind);

#endif

#endif
