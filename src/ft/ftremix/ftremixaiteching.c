/*
 * ftremixaiteching.c - teching and Z-cancel hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm
 * random_teching_ and z_cancel_). Pure rules live in ftremixaiteachingrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <port_log.h>
#include <gm/gmremixtoggle.h>
#include <sc/scdef.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include <sys/utils.h>
#include "sc/sc1pmode/sc1premix1p.h"
#include "mn/mnvsmode/mnremixvsstats.h"
#include "remix/remix_training.h"
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

/** Single Button Mode toggle value (the asm reads Toggles.entry_single_button_mode). */
static s32 ftRemixAiSingleButtonMode(void)
{
    return gmRemixToggle(single_button_mode);
}

/** Remix 1P game (SinglePlayerModes.singleplayer_mode_flag == 4). */
static sb32 ftRemixAiIsRemix1P(void)
{
    return sc1PRemix1PIsActive();
}

/*
 * Off-stage flag of the asm (`lh 0x018C` and mask 0x0004): the low byte of the halfword is FTStruct 0x018D, whose mask 0x04 is
 * is_magnify_show, the flag the camera sets for a fighter shown by the magnifying glass because it is off screen.
 */
static sb32 ftRemixAiIsOffStage(FTStruct *fp)
{
    return (fp->is_magnify_show != FALSE) ? TRUE : FALSE;
}

/** Vanilla tech checks, the same two calls the replaced chain used, short-circuited the same way. */
static sb32 ftRemixAiTechVanilla(GObj *fighter_gobj)
{
    return (ftCommonPassiveStandCheckInterruptDamage(fighter_gobj) != FALSE) ||
           (ftCommonPassiveCheckInterruptDamage(fighter_gobj) != FALSE);
}

/** Counts a CPU tech that did not happen (AI.asm random_teching_ _fail, VsStats.tech_miss_tracker). */
static void ftRemixAiTechCountMiss(FTStruct *fp)
{
    mnRemixVSStatsCountTracker(fp->player, nMNRemixVSStatsTrackerTechMiss);
}

/** Starts a tech roll in the direction (forward or backward) the fighter faces. TRUE as the fighter teched. */
static sb32 ftRemixAiTechStartRoll(GObj *fighter_gobj, s32 status_id)
{
    ftCommonPassiveStandSetStatus(fighter_gobj, status_id);

    return TRUE;
}

/**
 * Tech chosen by the Training menu (Training.entry_tech_behavior, AI.asm tech_option_table): 1 backward, 2 forward, 3 in place,
 * 4 roll left and 5 roll right (relative to the facing, a fighter facing right rolls backward to the left), 6 none.
 */
static sb32 ftRemixAiTechByMenu(GObj *fighter_gobj, FTStruct *fp, s32 option)
{
    port_log("SSB64: training tech: port %d option %d lr %d\n", (int)fp->player, (int)option, (int)fp->lr);

    switch (option)
    {
    case FTREMIX_AI_TECH_MENU_BACKWARD:
        return ftRemixAiTechStartRoll(fighter_gobj, nFTCommonStatusPassiveStandB);

    case FTREMIX_AI_TECH_MENU_FORWARD:
        return ftRemixAiTechStartRoll(fighter_gobj, nFTCommonStatusPassiveStandF);

    case FTREMIX_AI_TECH_MENU_IN_PLACE:
        ftCommonPassiveSetStatus(fighter_gobj);
        return TRUE;

    case FTREMIX_AI_TECH_MENU_LEFT:
        return ftRemixAiTechStartRoll(fighter_gobj, (fp->lr == +1) ? nFTCommonStatusPassiveStandB : nFTCommonStatusPassiveStandF);

    case FTREMIX_AI_TECH_MENU_RIGHT:
        return ftRemixAiTechStartRoll(fighter_gobj, (fp->lr == +1) ? nFTCommonStatusPassiveStandF : nFTCommonStatusPassiveStandB);

    default:
        ftRemixAiTechCountMiss(fp);
        return FALSE;
    }
}

/** Rolls the improved tech and sets the status of the outcome. TRUE when the fighter teched. */
static sb32 ftRemixAiTechRoll(GObj *fighter_gobj, FTStruct *fp)
{
    s32 roll = syUtilsRandIntRange(FTREMIX_AI_TECH_ROLL_MAX);

    switch (ftRemixAiTechRollOutcome(roll, ftRemixAiIsOffStage(fp)))
    {
    case FTREMIX_AI_TECH_OUTCOME_FORWARD:
        return ftRemixAiTechStartRoll(fighter_gobj, nFTCommonStatusPassiveStandF);

    case FTREMIX_AI_TECH_OUTCOME_BACKWARD:
        return ftRemixAiTechStartRoll(fighter_gobj, nFTCommonStatusPassiveStandB);

    case FTREMIX_AI_TECH_OUTCOME_IN_PLACE:
        ftCommonPassiveSetStatus(fighter_gobj);
        return TRUE;
    }
    ftRemixAiTechCountMiss(fp);

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
 * A CPU in Training follows the menu tech option unless it is Random.
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
    if (ftRemixAiIsTrainingScreen() && (remix_training_get(REMIX_TRAINING_TECH) != 0))
    {
        return ftRemixAiTechByMenu(fighter_gobj, fp, remix_training_get(REMIX_TRAINING_TECH));
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
