/**
 * ftremixtraining.c - fighter side of the Remix Training custom menu, see ftremixtraining.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftdef.h>
#include <ft/ftparam.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include <port_log.h>
#include "ft/ftremix/ftremixaicommands.h"
#include "ft/ftremix/ftremixtraining.h"
#include "remix/remix_training.h"
#include "sc/sc1pmode/sc1ptrainingrules.h"

#define FTREMIX_TRAINING_PORTS 4
#define FTREMIX_TRAINING_SPAM_COUNT 13
#define FTREMIX_TRAINING_RANDOM_RANGE 100
#define FTREMIX_TRAINING_DI_HITLAG 2
#define FTREMIX_TRAINING_CUSTOM_PRESS_C 1
#define FTREMIX_TRAINING_CUSTOM_RELEASE_C 2

static u8 sFTRemixTrainingShieldStatus[FTREMIX_TRAINING_PORTS];
static u8 sFTRemixTrainingSpamTimer[FTREMIX_TRAINING_PORTS];
static s32 sFTRemixTrainingAppliedPercent[FTREMIX_TRAINING_PORTS];

/* AI.asm Training Spam Mode routines (1917-2010), the same byte stream the vanilla CPU parser reads. */
static u8 sFTRemixTrainingScriptFTiltRight[] = { 0xA0, 0x28, 0xB0, 0x00, 0x01, 0xA0, 0x00, 0x10, 0xFF };
static u8 sFTRemixTrainingScriptFTiltLeft[] = { 0xA0, 0xD8, 0xB0, 0x00, 0x01, 0xA0, 0x00, 0x10, 0xFF };
static u8 sFTRemixTrainingScriptFSmashRight[] = { 0xA0, 0x38, 0xB0, 0x00, 0x01, 0xA0, 0x00, 0x10, 0xFF };
static u8 sFTRemixTrainingScriptFSmashLeft[] = { 0xA0, 0xC8, 0xB0, 0x00, 0x01, 0xA0, 0x00, 0x10, 0xFF };
static u8 sFTRemixTrainingScriptNsp[] = { 0xA0, 0x00, 0xB0, 0x00, 0x21, 0x30, 0xFF };
static u8 sFTRemixTrainingScriptUsp[] = { 0xA0, 0x00, 0xB0, 0x38, 0x21, 0xB0, 0x00, 0x30, 0xFF };
static u8 sFTRemixTrainingScriptDsp[] = { 0xA0, 0x00, 0xB0, 0xC8, 0x21, 0xB0, 0x00, 0x30, 0xFF };
static u8 sFTRemixTrainingScriptGrab[] = { 0xA0, 0x00, 0xB0, 0x00, 0x40, 0x01, 0x50, 0x10, 0xFF };
static u8 sFTRemixTrainingScriptJab[] = { 0xA0, 0x00, 0xB0, 0x00, 0x01, 0x10, 0xFF };
static u8 sFTRemixTrainingScriptFullHop[] = { 0xA0, 0x00, 0xB0, 0x78, 0x51, 0xB0, 0x00, 0xFF };
static u8 sFTRemixTrainingScriptShortHop[] =
{
    FTREMIX_AI_COMMAND_CUSTOM, FTREMIX_TRAINING_CUSTOM_PRESS_C, 0xA0, 0x00, 0xB1, 0x00, FTREMIX_AI_COMMAND_CUSTOM,
    FTREMIX_TRAINING_CUSTOM_RELEASE_C, 0xFF
};

void ftRemixTrainingStartScene(void)
{
    s32 port;

    for (port = 0; port < FTREMIX_TRAINING_PORTS; port++)
    {
        sFTRemixTrainingShieldStatus[port] = SC1PTRAININGRULES_STATUS_SHIELD;
        sFTRemixTrainingSpamTimer[port] = 0;
        sFTRemixTrainingAppliedPercent[port] = remix_training_get(REMIX_TRAINING_PERCENT(port));
    }
}

static sb32 ftRemixTrainingIsActive(void)
{
    return ((gSCManagerBattleState != NULL) && (gSCManagerBattleState->game_type == nSCBattleGameTypeTraining)) ? TRUE : FALSE;
}

static sb32 ftRemixTrainingIsValidPort(FTStruct *fp)
{
    return (fp->player < FTREMIX_TRAINING_PORTS) ? TRUE : FALSE;
}

static void ftRemixTrainingBuildShieldInput(FTStruct *fp, SC1PTrainingRulesShieldIn *in)
{
    in->mode = remix_training_get(REMIX_TRAINING_SHIELD_BREAK);
    in->status = sFTRemixTrainingShieldStatus[fp->player];
    in->oos_option = remix_training_get(REMIX_TRAINING_OOS);
    in->is_in_shield_stun = (fp->status_id == nFTCommonStatusGuardSetOff) ? TRUE : FALSE;
    in->is_shield_stun_ending = ((fp->status_vars.common.guard.setoff_frames - 1.0F) <= 0.0F) ? TRUE : FALSE;
    in->is_in_shield_jumpsquat = (fp->status_id == nFTCommonStatusGuardKneeBend) ? TRUE : FALSE;
    in->is_last_jumpsquat_frame = ((s32)fp->attr->kneebend_anim_length == (s32)fp->status_total_tics) ? TRUE : FALSE;
}

static void ftRemixTrainingApplyShieldOutput(FTStruct *fp, const SC1PTrainingRulesShieldOut *out)
{
    FTPlayerInput *pl = &fp->input.pl;

    if (out->max_shield != FALSE)
    {
        fp->shield_health = SC1PTRAININGRULES_MAX_SHIELD;
    }
    switch (out->action)
    {
    case SC1PTRAININGRULES_ACTION_OOS_INPUT:
        pl->button_hold = out->button_mask;
        pl->button_tap = out->button_mask;
        pl->button_release = Z_TRIG;
        pl->stick_range.x = out->stick_x;
        pl->stick_range.y = out->stick_y;
        break;

    case SC1PTRAININGRULES_ACTION_END_OOS:
        pl->button_release = out->button_mask;
        pl->stick_range.x = out->stick_x;
        pl->stick_range.y = out->stick_y;
        pl->button_hold = Z_TRIG;
        break;

    case SC1PTRAININGRULES_ACTION_FORCE_SHIELD:
        pl->button_hold = Z_TRIG;
        break;
    }
}

/*
 * Training.asm shield_break_mode_ has no controller-kind check, but its comment is "Forces CPUs to shield": run for a human it would
 * lock the player into the shield, so the port applies it to CPU fighters only.
 */
void ftRemixTrainingShieldBreakInput(FTStruct *fp)
{
    SC1PTrainingRulesShieldIn in;
    SC1PTrainingRulesShieldOut out;

    if ((ftRemixTrainingIsActive() == FALSE) || (fp->pkind != nFTPlayerKindCom) || (ftRemixTrainingIsValidPort(fp) == FALSE))
    {
        return;
    }
    ftRemixTrainingBuildShieldInput(fp, &in);
    sc1PTrainingRulesShieldStep(&in, &out);
    sFTRemixTrainingShieldStatus[fp->player] = out.new_status;
    ftRemixTrainingApplyShieldOutput(fp, &out);
}

/**
 * Script of a Spam Practice option (spam_command_table). The vanilla CPU scripts are the Remix table entries 0x17, 0x20, 0x19 and 0x22.
 *
 * @param option Spam Practice entry, 1 to FTREMIX_TRAINING_SPAM_COUNT.
 * @param is_facing_left The fighter faces left (only the tilt and smash forward routines differ).
 */
static u8 *ftRemixTrainingGetSpamScript(s32 option, sb32 is_facing_left)
{
    switch (option)
    {
    case 1: return ftComputerGetInputScript(nFTComputerInputStickTiltHiButtonA);
    case 2: return ftComputerGetInputScript(nFTComputerInputStickTiltLwButtonA);
    case 3: return (is_facing_left != FALSE) ? sFTRemixTrainingScriptFTiltLeft : sFTRemixTrainingScriptFTiltRight;
    case 4: return ftComputerGetInputScript(nFTComputerInputStickSmashHiButtonA);
    case 5: return ftComputerGetInputScript(nFTComputerInputStickSmashLwButtonA);
    case 6: return (is_facing_left != FALSE) ? sFTRemixTrainingScriptFSmashLeft : sFTRemixTrainingScriptFSmashRight;
    case 7: return sFTRemixTrainingScriptDsp;
    case 8: return sFTRemixTrainingScriptNsp;
    case 9: return sFTRemixTrainingScriptUsp;
    case 10: return sFTRemixTrainingScriptGrab;
    case 11: return sFTRemixTrainingScriptJab;
    case 12: return sFTRemixTrainingScriptShortHop;
    default: return sFTRemixTrainingScriptFullHop;
    }
}

sb32 ftRemixTrainingSpamSetObjective(FTStruct *fp)
{
    s32 option = remix_training_get(REMIX_TRAINING_SPAM_PRACTICE);
    s32 timer;

    if ((ftRemixTrainingIsActive() == FALSE) || (ftRemixTrainingIsValidPort(fp) == FALSE) || (option == 0) ||
        (remix_training_get(REMIX_TRAINING_SHIELD_BREAK) != SC1PTRAININGRULES_SHIELD_BREAK_OFF))
    {
        return FALSE;
    }
    if (sFTRemixTrainingSpamTimer[fp->player] != 0)
    {
        return FALSE;
    }
    timer = sc1PTrainingRulesSpamReloadTimer(remix_training_get(REMIX_TRAINING_SPAM_RANDOM),
                                             remix_training_get(REMIX_TRAINING_SPAM_INTERVAL),
                                             (remix_training_get(REMIX_TRAINING_SPAM_RANDOM) != 0)
                                                 ? syUtilsRandIntRange(FTREMIX_TRAINING_RANDOM_RANGE) : 0);
    sFTRemixTrainingSpamTimer[fp->player] = timer;
    fp->computer.p_command = ftRemixTrainingGetSpamScript(option, (fp->lr < 0) ? TRUE : FALSE);
    fp->computer.input_wait = 1;
    port_log("SSB64: training spam: port %d option %d timer %d\n", (int)fp->player, (int)option, (int)timer);

    return TRUE;
}

static void ftRemixTrainingTickSpamTimer(FTStruct *fp)
{
    s32 timer;

    if (ftRemixTrainingIsValidPort(fp) == FALSE)
    {
        return;
    }
    timer = sFTRemixTrainingSpamTimer[fp->player];
    sFTRemixTrainingSpamTimer[fp->player] =
        sc1PTrainingRulesSpamTickTimer(timer, (fp->pkind == nFTPlayerKindCom) ? TRUE : FALSE, (fp->hitlag_tics != 0) ? TRUE : FALSE);
}

static void ftRemixTrainingApplyDiPractice(FTStruct *fp)
{
    if (fp->status_id < nFTCommonStatusWait)
    {
        return;
    }
    fp->hitlag_tics = FTREMIX_TRAINING_DI_HITLAG;
    fp->proc_lagupdate = ftCommonDamageCommonProcLagUpdate;
}

/**
 * Percent entry (Training.asm set_percent_): when the menu value of a port changes, the fighter's percent is reset to zero and the new
 * value is added, through the same damage update the asm uses (Character.add_percent_).
 */
static void ftRemixTrainingApplyPercent(FTStruct *fp)
{
    s32 wanted;

    if (ftRemixTrainingIsValidPort(fp) == FALSE)
    {
        return;
    }
    wanted = remix_training_get(REMIX_TRAINING_PERCENT(fp->player));
    if (wanted == sFTRemixTrainingAppliedPercent[fp->player])
    {
        return;
    }
    sFTRemixTrainingAppliedPercent[fp->player] = wanted;
    ftParamUpdateDamage(fp, -fp->percent_damage);
    ftParamUpdateDamage(fp, wanted);
}

void ftRemixTrainingUpdateFighters(void)
{
    GObj *fighter_gobj;
    sb32 is_spam_on = (remix_training_get(REMIX_TRAINING_SPAM_PRACTICE) != 0) ? TRUE : FALSE;
    sb32 is_di_on = (remix_training_get(REMIX_TRAINING_DI_PRACTICE) != 0) ? TRUE : FALSE;

    if (ftRemixTrainingIsActive() == FALSE)
    {
        return;
    }
    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        ftRemixTrainingApplyPercent(fp);
        if (is_di_on != FALSE)
        {
            ftRemixTrainingApplyDiPractice(fp);
        }
        if (is_spam_on != FALSE)
        {
            ftRemixTrainingTickSpamTimer(fp);
        }
    }
}

#endif
