#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaicombatrules.h"

/**
 * Reads the invulnerability timers of a fighter. The asm skips the evade when either timer is at or above its limit,
 * so a timer of exactly the limit is already invulnerable.
 */
int ftRemixAiCombatIsInvulnerable(int star_timer, int respawn_timer)
{
    return (star_timer >= FTREMIX_AI_COMBAT_STAR_FRAMES_MIN) || (respawn_timer >= FTREMIX_AI_COMBAT_RESPAWN_FRAMES_MIN);
}

/** Evade is suppressed only by the improved AI, the vanilla behaviour keeps evading. */
int ftRemixAiCombatEvadeSuppressed(int improved_ai, int invulnerable)
{
    return (improved_ai != 0) && (invulnerable != 0);
}

/** Level 10 charge roll: the CPU charges only on a roll of 0. */
int ftRemixAiCombatChargeAllowed(int cpu_level, int roll)
{
    if (cpu_level < FTREMIX_AI_COMBAT_CHARGE_LEVEL_MIN)
    {
        return 1;
    }
    return roll == 0;
}

/** Projectile ignore is a plain OR of the two fighter ids and the badge. */
int ftRemixAiCombatIgnoresProjectile(int ignored_fkind, int wears_franklin_badge)
{
    return (ignored_fkind != 0) || (wears_franklin_badge != 0);
}

#endif
