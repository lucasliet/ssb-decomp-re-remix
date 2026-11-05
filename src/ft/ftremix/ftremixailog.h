#ifndef _FTREMIXAILOG_H_
#define _FTREMIXAILOG_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/*
 * SSB64_REMIX_AI_LOG=1: logs the CPU recovery episodes ("[remix-ai] recover_begin" / "recover_end") so the level 9
 * recovery success rate can be measured (.claude/skills/remix-smoke-tests/scripts/ai_recovery.sh). One episode per port.
 */

/** Opens the recovery episode of a CPU fighter when its objective becomes Recover; no-op while the episode is open. */
void ftRemixAiLogRecoverBegin(FTStruct *fp);

/** Closes the open episode with result=ok when the fighter, after being airborne, lands on ground or holds a ledge. */
void ftRemixAiLogRecoverUpdate(FTStruct *fp);

/** Closes the open episode with result=ko when the fighter is KO'd. */
void ftRemixAiLogRecoverDead(FTStruct *fp);

#endif

#endif
