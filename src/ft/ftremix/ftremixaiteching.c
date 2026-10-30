/*
 * ftremixaiteching.c - teching and Z-cancel hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm
 * random_teching_ and z_cancel_). Pure rules live in ftremixaiteachingrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmremixtoggle.h>
#include <sc/scdef.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include <sys/utils.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixaiteching.h"
#include "ft/ftremix/ftremixaiteachingrules.h"
#include "ft/ftremix/ftremixairecovery.h"

/**
 * Teching and Z-cancel are both skipped on How to Play and on the DK vs Samus intro (AI.asm screen ids 0x3C and 0x2D).
 */
static sb32 ftRemixAiIsExcludedScreen(void)
{
    return (gSCManagerSceneData.scene_curr == nSCKindExplain) || (gSCManagerSceneData.scene_curr == nSCKindOpeningJungle);
}

/** Training screen (AI.asm screen id 0x36). */
static sb32 ftRemixAiIsTrainingScreen(void)
{
    return gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode;
}

/*
 * REMIX-TODO(AI): the Single Button Mode toggle is not in the port yet (the asm reads Toggles.entry_single_button_mode).
 * Until it is, the mode is OFF, so no button mode blocks a CPU tech or Z-cancel.
 */
static s32 ftRemixAiSingleButtonMode(void)
{
    return 0;
}

/*
 * REMIX-TODO(AI): the Remix 1P flag (SinglePlayerModes.singleplayer_mode_flag == 4) has no port equivalent yet, see
 * ftremixhitbox.c. Until then Remix 1P uses the level and toggle rules only.
 */
static sb32 ftRemixAiIsRemix1P(void)
{
    return FALSE;
}

/*
 * REMIX-TODO(AI): the off-stage flag (N64 FTStruct 0x18C, bit 0x4) has no mapped port field yet. Until it is mapped
 * every CPU is treated as on stage.
 */
static sb32 ftRemixAiIsOffStage(FTStruct *fp)
{
    return FALSE;
}

/** Vanilla tech checks, the same two calls the replaced chain used, short-circuited the same way. */
static sb32 ftRemixAiTechVanilla(GObj *fighter_gobj)
{
    return (ftCommonPassiveStandCheckInterruptDamage(fighter_gobj) != FALSE) ||
           (ftCommonPassiveCheckInterruptDamage(fighter_gobj) != FALSE);
}

/** Rolls the improved tech and sets the status of the outcome. TRUE when the fighter teched. */
static sb32 ftRemixAiTechRoll(GObj *fighter_gobj, FTStruct *fp)
{
    s32 roll = syUtilsRandIntRange(FTREMIX_AI_TECH_ROLL_MAX);

    switch (ftRemixAiTechRollOutcome(roll, ftRemixAiIsOffStage(fp)))
    {
    case FTREMIX_AI_TECH_OUTCOME_FORWARD:
        ftCommonPassiveStandSetStatus(fighter_gobj, nFTCommonStatusPassiveStandF);
        return TRUE;

    case FTREMIX_AI_TECH_OUTCOME_BACKWARD:
        ftCommonPassiveStandSetStatus(fighter_gobj, nFTCommonStatusPassiveStandB);
        return TRUE;

    case FTREMIX_AI_TECH_OUTCOME_IN_PLACE:
        ftCommonPassiveSetStatus(fighter_gobj);
        return TRUE;
    }
    return FALSE;
}

/**
 * Whether the improved roll applies to this CPU. Only called for CPUs, after the sandbag and Single Button Mode checks.
 */
static sb32 ftRemixAiTechUsesRoll(FTStruct *fp)
{
    sb32 is_advanced_cpu = (fp->level >= FTREMIX_AI_TECH_LEVEL_ADVANCED) || ftRemixAiIsRemix1P();

    return ftRemixAiTechUsesImprovedRoll(ftRemixAiIsTrainingScreen(), is_advanced_cpu, ftRemixAiRecoveryIsClassicMode(),
                                         gmRemixToggle(improved_ai), ftRemixAiIsExcludedScreen());
}

/**
 * random_teching_ (AI.asm): sandbags never tech. Humans keep the vanilla checks. Single Button Mode only restricts CPUs,
 * the comment in the asm says "don't let CPU tech". CPUs without the improved path keep the vanilla checks.
 * REMIX-TODO(AI): the failed tech counter (VsStats.tech_miss_tracker) is not ported, so misses are not counted.
 * REMIX-TODO(AI): the training menu tech option (Training.entry_tech_behavior) is not ported, so training CPUs roll.
 */
sb32 ftRemixAiTechIsHandled(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->fkind == REMIX_ID_SANDBAG)
    {
        return FALSE;
    }
    if (fp->pkind != nFTPlayerKindCom)
    {
        return ftRemixAiTechVanilla(fighter_gobj);
    }
    if (ftRemixAiTechSingleButtonAllows(ftRemixAiSingleButtonMode()) == FALSE)
    {
        return FALSE;
    }
    if (ftRemixAiTechUsesRoll(fp) == FALSE)
    {
        return ftRemixAiTechVanilla(fighter_gobj);
    }
    return ftRemixAiTechRoll(fighter_gobj, fp);
}

/**
 * z_cancel_ (AI.asm): the CPU Z-cancel is a 95% roll on the improved path only. The roll is made only after every gate
 * passes, so the RNG is untouched otherwise. Its result is the Z-cancel success used by ftRemixZCancelResult.
 */
sb32 ftRemixAiShouldZCancel(FTStruct *fp)
{
    int improved_path = (fp->level >= FTREMIX_AI_TECH_LEVEL_ADVANCED) || ftRemixAiIsRemix1P() ||
                        (gmRemixToggle(improved_ai) != 0);
    int gate_open = ftRemixAiZCancelGateOpen(improved_path, ftRemixAiIsExcludedScreen(),
                                             ftRemixAiTechSingleButtonAllows(ftRemixAiSingleButtonMode()),
                                             fp->pkind == nFTPlayerKindCom, ftRemixAiRecoveryIsClassicMode());

    if (gate_open == FALSE)
    {
        return FALSE;
    }
    return ftRemixAiZCancelRollSucceeds(syUtilsRandIntRange(FTREMIX_AI_TECH_ROLL_MAX));
}

#endif
