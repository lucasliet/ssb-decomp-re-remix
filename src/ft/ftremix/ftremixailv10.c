#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sys/utils.h>
#include "ft/ftremix/ftremixailv10.h"
#include "ft/ftremix/ftremixailv10rules.h"

/**
 * check_invulnerable_ (AI.asm): a respawning enemy also counts as a Starman for a level 10 CPU, so the 1500 distance test
 * of the evade loop applies to it.
 */
sb32 ftRemixAiEvadeCountsEnemyInvulnerable(FTStruct *cpu, FTStruct *enemy)
{
    int star_invincible = (enemy->star_hitstatus == nGMHitStatusInvincible);

    return ftRemixAiCountsAsInvulnerableEnemy(star_invincible, enemy->invincible_tics, cpu->level);
}

/**
 * extend_cliff_attack_ (AI.asm): the normal getup is the vanilla input. Level 10 rolls once more, and the roll decides
 * CLIFF_LET_GO. The roll is taken only for level 10, so lower levels keep the RNG sequence.
 */
s32 ftRemixAiCliffGetupCommand(FTStruct *cpu)
{
    int normal_input = nFTComputerInputMoveAutoStickTiltHiReleaseZ;

    if (ftRemixAiIsLevelTen(cpu->level) == FALSE)
    {
        return normal_input;
    }
    return ftRemixAiCliffGetupInputFromRoll(syUtilsRandIntRange(2), normal_input);
}

/**
 * taunt_replace_ (AI.asm): a level 10 CPU sends no taunt. The Ness and Lucas USP branch of the asm is dead code (the
 * unconditional branch before it returns first), so it is not ported.
 */
sb32 ftRemixAiTauntIsSuppressed(FTStruct *cpu)
{
    return ftRemixAiIsLevelTen(cpu->level) != FALSE;
}

/**
 * ftComputerProcessTrait (AI.asm force_objective_attack): level 10 forces behaviour 0 and does not call the RNG.
 */
s32 ftRemixAiTraitRoll(FTStruct *cpu)
{
    if (ftRemixAiIsLevelTen(cpu->level) != FALSE)
    {
        return 0;
    }
    return syUtilsRandUShort() & 3;
}

/**
 * _fix_weird_air_drift_right and _left (AI.asm): a level 10 CPU with positive Y speed skips the block that sets the
 * target to itself plus an X offset, and goes on with the edge search.
 */
sb32 ftRemixAiSkipsUpwardDriftTarget(FTStruct *cpu)
{
    return ftRemixAiIsLevelTen(cpu->level) != FALSE;
}

#endif
