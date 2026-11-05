/*
 * ftremixairecovery.c - recovery hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm recovery_fix_ and
 * fix_remix_recovery). Pure rules live in ftremixairecoveryrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <gm/gmremixtoggle.h>
#include <mp/mpdef.h>
#include <mp/map.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixairecovery.h"
#include "ft/ftremix/ftremixairecoveryrules.h"

#define FTREMIX_AI_RECOVERY_WOLF_TARGET_Y 4000.0F
#define FTREMIX_AI_RECOVERY_NO_CHARGE_ACTION -1
#define FTREMIX_AI_RECOVERY_PRESS_B_COMMAND 0x09

/**
 * Classic mode is the vanilla 1P/RTTF game type (AI.asm reads Global.match_info, GAMEMODE.CLASSIC). See the deviation
 * note in docs/remix/ai.md.
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
 * fix_target_pos_x (AI.asm, improved AI only): clamps the walk target X inside the stage map bounds shrunk by the padding.
 * The bounds are map_bound_left and map_bound_right of the ground data (asm lh +0x7A and +0x78 of the pointer at 0x80131300).
 * Only the X of the target is clamped. Remix 1P does not enable it (the asm reads the toggle only).
 */
void ftRemixAiRecoveryClampWalkTargetX(FTStruct *fp)
{
    MPGroundData *ground = gMPCollisionGroundData;

    if ((gmRemixToggle(improved_ai) == FALSE) || (ground == NULL))
    {
        return;
    }
    fp->computer.target_pos.x = ftRemixAiRecoveryClampTargetX(fp->computer.target_pos.x, ground->map_bound_left, ground->map_bound_right);
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
 * fix_remix_recovery (AI.asm, _banjo_flight): on frame 21 of Banjo Flight the CPU presses B. With no ground below it also
 * sets the PRESS B command and skips the custom recovery. Returns TRUE when the frame is handled that way.
 */
static sb32 ftRemixAiRecoveryBanjoFlap(FTStruct *fp)
{
    if (ftRemixAiRecoveryIsBanjoFlapFrame(fp->status_id, fp->status_total_tics) == FALSE)
    {
        return FALSE;
    }
    fp->input.pl.button_tap |= B_BUTTON;
    if (ftRemixAiRecoveryBanjoNeedsPressB(fp->coll_data.floor_line_id) == FALSE)
    {
        return FALSE;
    }
    ftComputerSetCommandWaitShort(fp, FTREMIX_AI_RECOVERY_PRESS_B_COMMAND);
    return TRUE;
}

/**
 * fix_remix_recovery (AI.asm, the NSP charge cancel): a CPU in its charge action presses Z and skips the walk and the
 * custom recovery of this frame. Returns TRUE when the frame is handled that way.
 */
sb32 ftRemixAiRecoverySpecialCase(FTStruct *fp)
{
    s32 charge_action;

    if (fp->fkind == REMIX_ID_BANJO)
    {
        return ftRemixAiRecoveryBanjoFlap(fp);
    }
    charge_action = ftRemixAiRecoveryChargeActionOf(fp->fkind);

    if (ftRemixAiRecoveryIsNspChargeAction(fp->status_id, charge_action) == FALSE)
    {
        return FALSE;
    }
    ftComputerSetCommandWaitShort(fp, nFTComputerInputButtonZ1);
    return TRUE;
}

#endif
