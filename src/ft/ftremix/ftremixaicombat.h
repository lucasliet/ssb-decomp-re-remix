#ifndef _FTREMIXAICOMBAT_H_
#define _FTREMIXAICOMBAT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Combat hooks of the Remix AI (AI.asm evade_fix_, charge_less_often, remix_ignore_projectiles_). */

/** evade_fix_: TRUE when the improved AI stops this CPU from evading while it is invulnerable. */
sb32 ftRemixAiCombatEvadeIsSuppressed(FTStruct *fp);

/** charge_less_often: TRUE when a level 10 CPU skips this charge check (rolls 1/60 per check). */
sb32 ftRemixAiCombatChargeIsSkipped(FTStruct *fp);

/** remix_ignore_projectiles_: TRUE when the CPU ignores the projectile reaction (MLuigi, GBowser, Franklin Badge). */
sb32 ftRemixAiCombatProjectileIgnored(FTStruct *fp);

#endif

#endif
