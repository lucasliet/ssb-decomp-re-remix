#ifndef _FTREMIXAIPOSTUSP_H_
#define _FTREMIXAIPOSTUSP_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* USP steering entries of the Remix AI post process (Character.cpu_post_process table). Pure rules live in ftremixaipostrules.c. */

/** Bowser and Giga Bowser (Bowser/AI/Attacks.asm, GBowser/AI/Attacks.asm): the grounded USP points at the target. */
void ftRemixAiPostProcessBowser(FTStruct *fp);

/** Dragon King (DragonKing/AI/Attacks.asm): both USP actions point at the target while above ground. */
void ftRemixAiPostProcessDragonKing(FTStruct *fp);

/** Ganondorf (Ganondorf/AI/Attacks.asm): the Warlock Dive turns toward the target, then points at it. */
void ftRemixAiPostProcessGanondorf(FTStruct *fp);

/** Captain Falcon and J Falcon (captainshared.asm cpu_post_process_falcon): the Falcon Dive, level 10 only. */
void ftRemixAiPostProcessCaptain(FTStruct *fp);

/** Fox and J Fox (foxshared.asm): the Fire Fox dives to the ground, level 10 only. */
void ftRemixAiPostProcessFox(FTStruct *fp);

/** Pikachu family (pikashared.asm): the Quick Attack zips down to the ground, level 10 only. */
void ftRemixAiPostProcessPikachu(FTStruct *fp);

#endif

#endif
