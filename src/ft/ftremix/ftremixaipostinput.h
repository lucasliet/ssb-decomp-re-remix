#ifndef _FTREMIXAIPOSTINPUT_H_
#define _FTREMIXAIPOSTINPUT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Input veto and charge entries of the Remix AI post process (Character.cpu_post_process table). Pure rules live in ftremixaipostrules.c. */

/** Mewtwo (Mewtwo/AI/Attacks.asm dsp_check): the DSP is cancelled against a target it cannot grab. */
void ftRemixAiPostProcessMewtwo(FTStruct *fp);

/** Jigglypuff family (jigglypuffkirbyshared.asm cpu_post_process): the DSP is cancelled against a shield or a low-damage target, level 10 only. */
void ftRemixAiPostProcessPuff(FTStruct *fp);

/** Kirby and J Kirby (jigglypuffkirbyshared.asm cpu_post_process_kirby): the stone is left by mashing B, level 10 only. */
void ftRemixAiPostProcessKirby(FTStruct *fp);

/** Yoshi and J Yoshi (yoshishared.asm): the NSP is cancelled against a target already in an egg, level 10 only. */
void ftRemixAiPostProcessYoshi(FTStruct *fp);

/** Link family (linkshared.asm): the NSP is cancelled while a boomerang is out, level 10 only (Young Link at any level). */
void ftRemixAiPostProcessLink(FTStruct *fp);

/** Dedede (Dedede/AI/Attacks.asm charge_check): B is held while charging the DSP far from the target. */
void ftRemixAiPostProcessDedede(FTStruct *fp);

#endif

#endif
