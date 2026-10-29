/*
 * ftremixaicombat.c - combat hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm evade_fix_, charge_less_often,
 * remix_ignore_projectiles_). Pure rules live in ftremixaicombatrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmremixtoggle.h>
#include <it/itremix/itremixfranklinbadge.h>
#include <sys/utils.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixaicombat.h"
#include "ft/ftremix/ftremixaicombatrules.h"

/**
 * evade_fix_ (AI.asm): the improved AI skips the evade check while the CPU has Starman or respawn invincibility.
 * Remix 1P also forces the improved AI in asm; that part is REMIX-TODO(AI) until the Remix 1P flag is ported.
 */
sb32 ftRemixAiCombatEvadeIsSuppressed(FTStruct *fp)
{
    int invulnerable = ftRemixAiCombatIsInvulnerable(fp->star_invincible_tics, fp->invincible_tics);

    return ftRemixAiCombatEvadeSuppressed(gmRemixToggle(improved_ai), invulnerable);
}

/**
 * charge_less_often (AI.asm): only the roll is random, and it is rolled only for level 10 so lower levels leave the
 * RNG sequence untouched.
 */
sb32 ftRemixAiCombatChargeIsSkipped(FTStruct *fp)
{
    s32 roll = 0;

    if (fp->level >= FTREMIX_AI_COMBAT_CHARGE_LEVEL_MIN)
    {
        roll = syUtilsRandIntRange(FTREMIX_AI_COMBAT_CHARGE_ROLL_MAX);
    }
    return !ftRemixAiCombatChargeAllowed(fp->level, roll);
}

/**
 * remix_ignore_projectiles_ (AI.asm): MLuigi and GBowser always ignore projectiles, any other fighter ignores them
 * while it wears the Franklin Badge.
 */
sb32 ftRemixAiCombatProjectileIgnored(FTStruct *fp)
{
    int ignored_fkind = (fp->fkind == REMIX_ID_MLUIGI) || (fp->fkind == REMIX_ID_GBOWSER);

    return ftRemixAiCombatIgnoresProjectile(ignored_fkind, itRemixFranklinBadgeIsActive(fp));
}

#endif
