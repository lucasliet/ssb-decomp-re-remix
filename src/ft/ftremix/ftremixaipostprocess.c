#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmremixtoggle.h>
#include <sys/taskman.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixaipostprocess.h"
#include "ft/ftremix/ftremixaipostsheik.h"
#include "remix/remix_roster_data.h"

/**
 * Tells if the CPU is about to run the given input script: the command pointer is compared with the script of the id,
 * as the asm compares p_command with the AI.command_table entry.
 */
static sb32 ftRemixAiCommandIs(FTStruct *fp, s32 input_kind)
{
    return fp->computer.p_command == ftComputerGetInputScript(input_kind);
}

/**
 * _check_respawn (AI.asm): with Improved AI on, a dead CPU (status Sleep) mashes START to respawn.
 */
static void ftRemixAiMashStartWhenDead(FTStruct *fp)
{
    if (gmRemixToggle(improved_ai) == 0)
    {
        return;
    }
    if (fp->status_id != nFTCommonStatusSleep)
    {
        return;
    }
    fp->input.cp.button_inputs = (u16)ftRemixAiStartMashButtons(fp->input.cp.button_inputs, (int)dSYTaskmanUpdateCount);
}

/**
 * _check_if_dash_attack_input_is_possible (AI.asm): a dash attack that cannot start from the current state is cancelled
 * with the NULL routine. Not level gated.
 */
static void ftRemixAiCancelUnreadyDashAttack(FTStruct *fp)
{
    sb32 is_dash_attack = ftRemixAiCommandIs(fp, FTREMIX_AI_LV10_ROUTINE_DASH_ATTACK);
    sb32 is_idle_or_run = (fp->status_id == nFTCommonStatusWait) || (fp->status_id == nFTCommonStatusRun);

    if (ftRemixAiDashAttackCancelNeeded(is_dash_attack, is_idle_or_run))
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_NULL);
    }
}

/**
 * _dash_usmash_usp (AI.asm, level 10): while dashing or running, an up smash or USP becomes its jump-cancelled version.
 */
static void ftRemixAiJumpCancelDashCommand(FTStruct *fp)
{
    sb32 is_dashing = (fp->status_id == nFTCommonStatusDash) || (fp->status_id == nFTCommonStatusRun);

    if (is_dashing == FALSE)
    {
        return;
    }
    if (ftRemixAiCommandIs(fp, FTREMIX_AI_LV10_INPUT_USMASH))
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_JUMPCANCEL_UPSMASH);
        return;
    }
    if (ftRemixAiCommandIs(fp, FTREMIX_AI_LV10_INPUT_USP))
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_JUMPCANCEL_USP);
    }
}

/**
 * _custom_usp_command (AI.asm, level 10): the USP becomes the towards version. It compares the command after the jump
 * cancel above, so the jump-cancelled USP is not changed again.
 */
static void ftRemixAiLv10CustomUsp(FTStruct *fp)
{
    if (ftRemixAiCommandIs(fp, FTREMIX_AI_LV10_INPUT_USP))
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_USP_TOWARDS);
    }
}

/**
 * Per-fighter entries of Character.cpu_post_process (AI.asm 3867-3878), keyed by fkind. Only Sheik is ported.
 * REMIX-TODO(AI): the other entries are not ported: Falco and Fox/JFox (Falco/AI/Attacks.asm, foxshared.asm), Captain
 * Falcon/JFalcon (captainshared.asm), Link family (linkshared.asm), DK/JDK (dkshared.asm), Bowser, GBowser (Bowser/AI,
 * GBowser/AI), Wario, Marth, Dedede, Wolf, Ganondorf, Mewtwo, Roy, Marina, DragonKing, Jigglypuff and Kirby families
 * (jigglypuffkirbyshared.asm), Pikachu family (pikashared.asm:144), Yoshi/JYoshi (yoshishared.asm).
 */
static void ftRemixAiPostProcessFighter(FTStruct *fp)
{
    switch (fp->fkind)
    {
    case REMIX_ID_SHEIK:
        ftRemixAiPostProcessSheik(fp);
        break;
    }
}

/**
 * Runs the generic sub-behaviours of cpu_post_process (AI.asm) for one CPU, in the asm order.
 */
void ftRemixAiPostProcess(FTStruct *fp)
{
    ftRemixAiMashStartWhenDead(fp);
    ftRemixAiCancelUnreadyDashAttack(fp);
    if (ftRemixAiIsLevelTen(fp->level) != FALSE)
    {
        ftRemixAiJumpCancelDashCommand(fp);
        ftRemixAiLv10CustomUsp(fp);
    }
    ftRemixAiPostProcessFighter(fp);
}

#endif
