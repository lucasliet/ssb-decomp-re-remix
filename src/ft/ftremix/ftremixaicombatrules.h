#ifndef _FTREMIXAICOMBATRULES_H_
#define _FTREMIXAICOMBATRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure combat rules of the Remix AI (AI.asm evade_fix_, charge_less_often, remix_ignore_projectiles_), so the headless
 * unit tests (tests/remix) can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_COMBAT_STAR_FRAMES_MIN 112
#define FTREMIX_AI_COMBAT_RESPAWN_FRAMES_MIN 20
#define FTREMIX_AI_COMBAT_CHARGE_LEVEL_MIN 10
#define FTREMIX_AI_COMBAT_CHARGE_ROLL_MAX 60

/**
 * TRUE when a fighter cannot be hurt by an opponent's attack: the Super Star timer or the respawn timer still has the
 * frames the asm waits for (AI.asm evade_fix_, super star 112 and respawn 20).
 */
extern int ftRemixAiCombatIsInvulnerable(int star_timer, int respawn_timer);

/**
 * TRUE when the CPU must not evade: the improved AI is on and the CPU is invulnerable.
 */
extern int ftRemixAiCombatEvadeSuppressed(int improved_ai, int invulnerable);

/**
 * TRUE when a CPU may start a charge special. Only level 10 CPUs roll: a roll of 0 passes, out of
 * FTREMIX_AI_COMBAT_CHARGE_ROLL_MAX (AI.asm charge_less_often). Other levels always pass.
 */
extern int ftRemixAiCombatChargeAllowed(int cpu_level, int roll);

/**
 * TRUE when the CPU ignores projectiles: the character is MLUIGI or GBOWSER, or the CPU wears the Franklin Badge
 * (AI.asm remix_ignore_projectiles_).
 */
extern int ftRemixAiCombatIgnoresProjectile(int ignored_fkind, int wears_franklin_badge);

#endif

#endif
