/*
 * ftremixaispecials.c - charged NSP hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm
 * improve_remix_charged_NSP_1 and improve_remix_charged_NSP_2). Pure rules live in ftremixaispecialsrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixaishared.h"
#include "ft/ftremix/ftremixaispecials.h"
#include "ft/ftremix/ftremixaispecialsrules.h"
#include "ft/ftremix/ftremixfields.h"

#define FTREMIX_AI_SPECIALS_PRESS_Z_COMMAND 0xC
#define FTREMIX_AI_SPECIALS_PRESS_B_COMMAND 0xB
#define FTREMIX_N64_OFFSET_NEEDLE_AMMO 0x0AE0

/** The vanilla cancel switch uses the copied fighter for Kirby and J Kirby, so the Remix cases do too. */
static s32 ftRemixAiSpecialsSwitchKind(FTStruct *fp)
{
    return ftRemixAiCopiedPowerKind(fp);
}

/**
 * improve_remix_charged_NSP_1 (AI.asm): DSamus, Sheik and Mewtwo press Z when their charge action can be cancelled. The
 * DK fighters are cased in the vanilla switch (see ftComputerCheckTryCancelSpecialN).
 */
sb32 ftRemixAiSpecialsCancelChargeWithZ(FTStruct *fp)
{
    sb32 is_charging = FALSE;

    switch (ftRemixAiSpecialsSwitchKind(fp))
    {
    case REMIX_ID_DSAMUS:
        is_charging = ftRemixAiSpecialsDSamusCancels(fp->status_id);
        break;

    case REMIX_ID_SHEIK:
        is_charging = ftRemixAiSpecialsSheikCancels(fp->status_id);
        break;

    case REMIX_ID_MTWO:
        is_charging = ftRemixAiSpecialsMewtwoCancels(fp->status_id);
        break;

    default:
        return FALSE;
    }
    if (is_charging == FALSE)
    {
        return FALSE;
    }
    ftComputerSetCommandWaitShort(fp, FTREMIX_AI_SPECIALS_PRESS_Z_COMMAND);
    return TRUE;
}

/**
 * improve_remix_specials (AI.asm _bowser): the grounded USP of Bowser and GBowser zeroes the CPU stick X (byte 0x1C8, the
 * cp stick range), without changing the charge decision.
 */
void ftRemixAiSpecialsBowserUspStickReset(FTStruct *fp)
{
    if (ftRemixAiSpecialsIsBowserUspGround(fp->status_id) == FALSE)
    {
        return;
    }
    fp->input.cp.stick_range.x = 0;
}

/**
 * improve_remix_specials (AI.asm _check_needle and _check_revolver): the needle (Sheik) or revolver (Peppy) ammo is the N64
 * word 0x0AE0.
 */
sb32 ftRemixAiSpecialsNeedleCharge(FTStruct *fp)
{
    s32 ammo = *ftRemixFighterWord(fp, FTREMIX_N64_OFFSET_NEEDLE_AMMO);

    switch (ftRemixAiSpecialsNeedleChargeOutcome((fp->fkind == REMIX_ID_SHEIK) ? 1 : 0, fp->status_id, ammo))
    {
    case FTREMIX_AI_SPECIALS_NEEDLE_KEEP:
        return TRUE;

    case FTREMIX_AI_SPECIALS_NEEDLE_PRESS:
        ftComputerSetCommandWaitShort(fp, FTREMIX_AI_SPECIALS_PRESS_B_COMMAND);
        return TRUE;
    }
    return FALSE;
}

sb32 ftRemixAiSpecialsFarRangeIsCharging(FTStruct *fp, s32 fkind)
{
    return ftRemixAiSpecialsChargeShotActive(fkind, fp->status_id) ? TRUE : FALSE;
}

sb32 ftRemixAiSpecialsFarRangeUsesSamusActions(s32 fkind)
{
    return ftRemixAiSpecialsUsesSamusActionList(fkind) ? TRUE : FALSE;
}

/** Opponent is in the always group (Fox-like) or the Ness group (Ness, Lucas, JNess). */
static sb32 ftRemixAiSpecialsIsAlwaysBlocking(s32 fkind)
{
    switch (fkind)
    {
    case REMIX_ID_FOX:
    case REMIX_ID_SLIPPY:
    case REMIX_ID_WOLF:
    case REMIX_ID_MARINA:
    case REMIX_ID_FALCO:
    case REMIX_ID_PIANO:
    case REMIX_ID_JFOX:
        return TRUE;
    }
    return FALSE;
}

/** Ness, Lucas and JNess block the Samus NSP unless the own fighter is Sheik. */
static sb32 ftRemixAiSpecialsIsSheikExempt(s32 fkind)
{
    return (fkind == REMIX_ID_NESS) || (fkind == REMIX_ID_LUCAS) || (fkind == REMIX_ID_JNESS);
}

/**
 * improve_remix_charged_NSP_2 (AI.asm): the Samus NSP is not fired at a reflector/absorber opponent. Called only inside
 * the vanilla random gate, in place of the vanilla Ness and Fox test. The Sheik exemption tests the own fighter id, not
 * the copied power, so a Kirby wearing the Sheik hat gets no exemption (the asm comment "misses Kirby's logic").
 */
sb32 ftRemixAiSamusNspIsBlocked(FTStruct *fp, FTStruct *opponent)
{
    return ftRemixAiSamusNspOpponentBlocks(ftRemixAiSpecialsIsAlwaysBlocking(opponent->fkind),
                                           ftRemixAiSpecialsIsSheikExempt(opponent->fkind), fp->fkind == REMIX_ID_SHEIK);
}

#endif
