#ifndef _FTREMIXAILV10_H_
#define _FTREMIXAILV10_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Level 10 CPU hooks of the Remix AI (AI.asm level_ten scope). Pure rules live in ftremixailv10rules.c. */

/** check_invulnerable_: the evade check also treats a respawning enemy as a Starman when the CPU is level 10. */
sb32 ftRemixAiEvadeCountsEnemyInvulnerable(FTStruct *cpu, FTStruct *enemy);

/** extend_cliff_attack_: the CliffWait getup input, level 10 CPUs may let go instead of the normal getup. */
s32 ftRemixAiCliffGetupCommand(FTStruct *cpu);

/** taunt_replace_: level 10 CPUs never send the taunt command. */
sb32 ftRemixAiTauntIsSuppressed(FTStruct *cpu);

/** ftComputerProcessTrait roll: level 10 CPUs always get behaviour 0 (default) and do not roll. */
s32 ftRemixAiTraitRoll(FTStruct *cpu);

/** _fix_weird_air_drift_right and _left: level 10 CPUs with positive Y speed skip the target-to-self block. */
sb32 ftRemixAiSkipsUpwardDriftTarget(FTStruct *cpu);

#endif

#endif
