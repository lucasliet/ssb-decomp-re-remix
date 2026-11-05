#ifndef _FTREMIXAIPOSTNSP_H_
#define _FTREMIXAIPOSTNSP_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/**
 * Marth and Roy entry of the Remix AI post process (Marth/AI/Attacks.asm, Roy/AI/Attacks.asm nsp_logic): once a grounded
 * NSP step hits (hitlag), the CPU picks the height of the next step by the target state and damage.
 */
void ftRemixAiPostProcessSwordNsp(FTStruct *fp);

#endif

#endif
