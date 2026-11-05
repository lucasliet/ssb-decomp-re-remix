#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmremixtoggle.h>
#include <port_log.h>
#include <sys/taskman.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixaipostcommon.h"
#include "ft/ftremix/ftremixaipostedge.h"
#include "ft/ftremix/ftremixaipostinput.h"
#include "ft/ftremix/ftremixaipostmarina.h"
#include "ft/ftremix/ftremixaipostnsp.h"
#include "ft/ftremix/ftremixaipostprocess.h"
#include "ft/ftremix/ftremixaipostsheik.h"
#include "ft/ftremix/ftremixaipostusp.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define FTREMIX_AI_POST_TABLE_ENTRY_BYTES 4
#define FTREMIX_AI_POST_LOG_FKIND_MAX 64

static u64 sFtRemixAiPostLoggedFkinds;

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
    sb32 is_dash_attack = ftRemixAiPostCommandIs(fp, FTREMIX_AI_LV10_ROUTINE_DASH_ATTACK);
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
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_LV10_INPUT_USMASH))
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_JUMPCANCEL_UPSMASH);
        return;
    }
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_LV10_INPUT_USP))
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
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_LV10_INPUT_USP))
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_USP_TOWARDS);
    }
}

/**
 * Address of the Character.cpu_post_process routine of a fighter kind, 0 when the kind has none. The routines are Remix
 * code, so the port runs the C equivalent that ftRemixAiPostProcessFighter picks by kind.
 */
static u32 ftRemixAiPostGetTableEntry(s32 fkind)
{
    return remix_ram_image_read_word(REMIX_SYM_Character__cpu_post_process__table +
                                     (fkind * FTREMIX_AI_POST_TABLE_ENTRY_BYTES));
}

/**
 * Logs the first run of each fighter kind's routine, so a match shows which post process fired without flooding the log.
 * Kinds >= FTREMIX_AI_POST_LOG_FKIND_MAX are not logged.
 */
static void ftRemixAiPostLogFirstRun(FTStruct *fp, u32 routine_addr, sb32 is_ported)
{
    u64 kind_bit;

    if ((fp->fkind < 0) || (fp->fkind >= FTREMIX_AI_POST_LOG_FKIND_MAX))
    {
        return;
    }
    kind_bit = (u64)1 << fp->fkind;
    if ((sFtRemixAiPostLoggedFkinds & kind_bit) != 0)
    {
        return;
    }
    sFtRemixAiPostLoggedFkinds |= kind_bit;
    port_log("ftRemixAiPostProcess: fkind 0x%X level %d routine 0x%08X %s\n", fp->fkind, fp->level, routine_addr,
             (is_ported != FALSE) ? "first run" : "not ported");
}

/**
 * Runs the C port of the routine that a fighter kind's Character.cpu_post_process entry points to (AI.asm 3867-3878).
 * The shared files register one routine for a family: Fox/JFox, Pikachu/EPika/JPika, Jigglypuff/JPuff/EPuff, Kirby/JKirby,
 * Yoshi/JYoshi, DK/JDK, Captain/JFalcon and the four Links. Returns FALSE for a kind with no port.
 */
static sb32 ftRemixAiPostProcessFighter(FTStruct *fp)
{
    switch (fp->fkind)
    {
    case REMIX_ID_BOWSER:
    case REMIX_ID_GBOWSER:
        ftRemixAiPostProcessBowser(fp);
        return TRUE;
    case REMIX_ID_DRAGONKING:
        ftRemixAiPostProcessDragonKing(fp);
        return TRUE;
    case REMIX_ID_GND:
        ftRemixAiPostProcessGanondorf(fp);
        return TRUE;
    case REMIX_ID_CAPTAIN:
    case REMIX_ID_JFALCON:
        ftRemixAiPostProcessCaptain(fp);
        return TRUE;
    case REMIX_ID_FOX:
    case REMIX_ID_JFOX:
        ftRemixAiPostProcessFox(fp);
        return TRUE;
    case REMIX_ID_PIKACHU:
    case REMIX_ID_EPIKA:
    case REMIX_ID_JPIKA:
        ftRemixAiPostProcessPikachu(fp);
        return TRUE;
    case REMIX_ID_FALCO:
        ftRemixAiPostProcessFalco(fp);
        return TRUE;
    case REMIX_ID_WARIO:
        ftRemixAiPostProcessWario(fp);
        return TRUE;
    case REMIX_ID_WOLF:
        ftRemixAiPostProcessWolf(fp);
        return TRUE;
    case REMIX_ID_DONKEY:
    case REMIX_ID_JDK:
        ftRemixAiPostProcessDonkeyKong(fp);
        return TRUE;
    case REMIX_ID_MARTH:
    case REMIX_ID_ROY:
        ftRemixAiPostProcessSwordNsp(fp);
        return TRUE;
    case REMIX_ID_MTWO:
        ftRemixAiPostProcessMewtwo(fp);
        return TRUE;
    case REMIX_ID_JIGGLY:
    case REMIX_ID_JPUFF:
    case REMIX_ID_EPUFF:
        ftRemixAiPostProcessPuff(fp);
        return TRUE;
    case REMIX_ID_KIRBY:
    case REMIX_ID_JKIRBY:
        ftRemixAiPostProcessKirby(fp);
        return TRUE;
    case REMIX_ID_YOSHI:
    case REMIX_ID_JYOSHI:
        ftRemixAiPostProcessYoshi(fp);
        return TRUE;
    case REMIX_ID_LINK:
    case REMIX_ID_JLINK:
    case REMIX_ID_ELINK:
    case REMIX_ID_YLINK:
        ftRemixAiPostProcessLink(fp);
        return TRUE;
    case REMIX_ID_DEDEDE:
        ftRemixAiPostProcessDedede(fp);
        return TRUE;
    case REMIX_ID_MARINA:
        ftRemixAiPostProcessMarina(fp);
        return TRUE;
    case REMIX_ID_SHEIK:
        ftRemixAiPostProcessSheik(fp);
        return TRUE;
    default:
        return FALSE;
    }
}

/**
 * _post_process_function (AI.asm): runs the per-fighter routine when the kind has a Character.cpu_post_process entry.
 */
static void ftRemixAiPostProcessTableEntry(FTStruct *fp)
{
    u32 routine_addr = ftRemixAiPostGetTableEntry(fp->fkind);
    sb32 is_ported;

    if (routine_addr == 0)
    {
        return;
    }
    is_ported = ftRemixAiPostProcessFighter(fp);
    ftRemixAiPostLogFirstRun(fp, routine_addr, is_ported);
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
    ftRemixAiPostProcessTableEntry(fp);
}

#endif
