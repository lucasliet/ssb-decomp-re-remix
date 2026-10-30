/*
 * ftremixairecovery.c - recovery hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm recovery_fix_ and
 * fix_remix_recovery). Pure rules live in ftremixairecoveryrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <gm/gmremixtoggle.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixairecovery.h"
#include "ft/ftremix/ftremixairecoveryrules.h"

#define FTREMIX_AI_RECOVERY_WOLF_TARGET_Y 4000.0F
#define FTREMIX_AI_RECOVERY_NO_CHARGE_ACTION -1

/**
 * Classic mode is the vanilla 1P/RTTF game type (AI.asm reads Global.match_info, GAMEMODE.CLASSIC). See the deviation
 * note in docs/remix/notes/f13_ai_port.md.
 */
sb32 ftRemixAiRecoveryIsClassicMode(void)
{
    return (gSCManagerBattleState != NULL) && (gSCManagerBattleState->game_type == nSCBattleGameType1PGame);
}

/**
 * recovery_fix_ (AI.asm): the vanilla up-B limit is skipped for CPUs at level 10 or with the improved AI, and never for
 * vanilla 1P/RTTF.
 */
sb32 ftRemixAiRecoveryAllowsUpBRepeat(FTStruct *fp)
{
    return ftRemixAiRecoveryUpBRepeatAllowed(fp->level, ftRemixAiRecoveryIsClassicMode(), gmRemixToggle(improved_ai));
}

/**
 * fix_remix_recovery (AI.asm, _pikachu): TRUE when the vanilla Pikachu recovery block runs instead of the Remix dispatch.
 */
sb32 ftRemixAiRecoveryVanillaPikachuApplies(FTStruct *fp)
{
    return ftRemixAiRecoveryKeepsPikachuVanilla(ftRemixAiRecoveryIsClassicMode(), gmRemixToggle(improved_ai));
}

/**
 * fix_remix_recovery (AI.asm, _wolf_set_target_coordinates): the Wolf USP actions aim the recovery target straight up.
 * The walk and the custom recovery run after this, as in the asm.
 */
void ftRemixAiRecoveryWolfTarget(FTStruct *fp)
{
    if (ftRemixAiRecoveryIsWolfUspAction(fp->status_id) == FALSE)
    {
        return;
    }
    fp->computer.target_pos.x = 0.0F;
    fp->computer.target_pos.y = FTREMIX_AI_RECOVERY_WOLF_TARGET_Y;
}

/**
 * NSP charge action of the fighter that cancels its charge with PRESS Z while recovering (AI.asm sheik, dsamus, peppy,
 * mewtwo). Other fighters return FTREMIX_AI_RECOVERY_NO_CHARGE_ACTION.
 */
static s32 ftRemixAiRecoveryChargeActionOf(s32 fkind)
{
    switch (fkind)
    {
    case REMIX_ID_SHEIK:
        return 0xEE;
    case REMIX_ID_DSAMUS:
        return 0xE7;
    case REMIX_ID_PEPPY:
        return 0xF2;
    case REMIX_ID_MTWO:
        return 0xE2;
    default:
        return FTREMIX_AI_RECOVERY_NO_CHARGE_ACTION;
    }
}

/**
 * fix_remix_recovery (AI.asm, the NSP charge cancel): a CPU in its charge action presses Z and skips the walk and the
 * custom recovery of this frame. Returns TRUE when the frame is handled that way.
 */
sb32 ftRemixAiRecoverySpecialCase(FTStruct *fp)
{
    s32 charge_action = ftRemixAiRecoveryChargeActionOf(fp->fkind);

    if (ftRemixAiRecoveryIsNspChargeAction(fp->status_id, charge_action) == FALSE)
    {
        return FALSE;
    }
    ftComputerSetCommandWaitShort(fp, nFTComputerInputButtonZ1);
    return TRUE;
}

#endif
