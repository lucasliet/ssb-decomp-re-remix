/*
 * ftremixaishared.c - CPU hooks of the Remix family shared files (.toolchain/pkgs/smashremix-src/src dkshared.asm,
 * jigglypuffkirbyshared.asm, nessshared.asm, yoshishared.asm). They extend vanilla fkind tests to the clones. Pure rules
 * live in ftremixaisharedrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixairecovery.h"
#include "ft/ftremix/ftremixaishared.h"
#include "ft/ftremix/ftremixaisharedrules.h"
#include "ft/ftremix/ftremixfields.h"

#define FTREMIX_AI_SHARED_PKTHUNDER_VANILLA_OFFSET 100.0F

/* nessshared.asm pk_thunder_target_coords: one ledge position per port, written when a PK Thunder is made. */
static Vec2f sFTRemixAiPkThunderLedge[MAXCONTROLLERS];

static sb32 ftRemixAiIsKirbyKind(s32 fkind)
{
    return (fkind == nFTKindKirby) || (fkind == REMIX_ID_JKIRBY);
}

/*
 * Dedede is not read as a copy holder: his N64 word 0x0ADC holds a minion pointer, which equals no fkind, so only the Kirby
 * kinds contribute a copied power.
 */
s32 ftRemixAiCopiedPowerKind(FTStruct *fp)
{
    return ftRemixAiIsKirbyKind(fp->fkind) ? fp->passive_vars.kirby.copy_id : fp->fkind;
}

sb32 ftRemixAiIsMetalKind(s32 fkind)
{
    return (fkind == nFTKindMMario) || (fkind == REMIX_ID_MLUIGI);
}

sb32 ftRemixAiEvadeIsMarinaCargo(FTStruct *fp)
{
    if ((fp->fkind != REMIX_ID_MARINA) && (fp->fkind != REMIX_ID_NMARINA))
    {
        return FALSE;
    }
    return ftRemixAiSharedIsMarinaCargoStatus(fp->status_id);
}

static s32 ftRemixAiInhaleOutcome(FTStruct *fp)
{
    sb32 is_kirby_kind = ftRemixAiIsKirbyKind(fp->fkind);
    s32 hat_id = is_kirby_kind ? *ftRemixFighterKirbyHat(fp) : 0;

    return ftRemixAiSharedInhaleOutcome(is_kirby_kind, fp->fkind == REMIX_ID_DEDEDE, hat_id, fp->status_id);
}

sb32 ftRemixAiInhaleSpitsHeldVictim(FTStruct *fp)
{
    if (ftRemixAiInhaleOutcome(fp) != FTREMIX_AI_SHARED_INHALE_PRESS_NSP)
    {
        return FALSE;
    }
    ftComputerSetCommandImmediate(fp, FTREMIX_AI_SHARED_INPUT_NSP);
    return TRUE;
}

sb32 ftRemixAiInhaleUsesKirbyBlock(FTStruct *fp)
{
    return ftRemixAiInhaleOutcome(fp) == FTREMIX_AI_SHARED_INHALE_KIRBY_BLOCK;
}

/*
 * The asm also lists J Kirby (compared through t6, which holds the top joint pointer there) and N Marina and N Dedede (compared
 * through a register already overwritten), so those never match; only the four ids below take the branch.
 */
sb32 ftRemixAiWalkUsesMultiJumpBranch(FTStruct *fp)
{
    switch (fp->fkind)
    {
    case nFTKindKirby:
    case nFTKindPurin:
    case REMIX_ID_JPUFF:
    case REMIX_ID_EPUFF:
    case REMIX_ID_MARINA:
    case REMIX_ID_DEDEDE:
        return TRUE;
    }
    return FALSE;
}

sb32 ftRemixAiWalkExitsInDededeUsp(FTStruct *fp)
{
    if ((fp->ga == nMPKineticsGround) || (fp->physics.vel_air.y < 0.0F))
    {
        return FALSE;
    }
    return (fp->fkind == REMIX_ID_DEDEDE) && ftRemixAiSharedWalkExitsInDededeUsp(fp->status_id);
}

sb32 ftRemixAiUspIsPuffAttackSkipKind(s32 fkind)
{
    return (fkind == nFTKindPurin) || (fkind == REMIX_ID_JPUFF) || (fkind == REMIX_ID_EPUFF);
}

sb32 ftRemixAiIsPkThunderKind(s32 fkind)
{
    return (fkind == nFTKindNess) || (fkind == REMIX_ID_JNESS) || (fkind == REMIX_ID_LUCAS);
}

sb32 ftRemixAiRecoveryMayUseUsp(FTStruct *fp)
{
    switch (fp->fkind)
    {
    case nFTKindYoshi:
    case nFTKindPurin:
    case REMIX_ID_JYOSHI:
        return FALSE;

    case REMIX_ID_MARINA:
        return ftRemixAiSharedMarinaMayUseUsp(fp->jumps_used);
    }
    return TRUE;
}

void ftRemixAiPkThunderSaveLedge(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec2f *saved_ledge;

    if (fp->player >= MAXCONTROLLERS)
    {
        return;
    }
    saved_ledge = &sFTRemixAiPkThunderLedge[fp->player];

    *saved_ledge = (DObjGetStruct(fighter_gobj)->translate.vec.f.x < 0.0F) ? fp->computer.cliff_left_pos
                                                                          : fp->computer.cliff_right_pos;
}

/** improved_pk_thunder_ gate: level 10, Remix 1P, or the improved AI outside classic mode (the recovery_fix_ gate). */
static sb32 ftRemixAiPkThunderAdvancedAiApplies(FTStruct *fp)
{
    return ftRemixAiRecoveryAllowsUpBRepeat(fp);
}

f32 ftRemixAiPkThunderTargetY(FTStruct *fp)
{
    Vec3f *position = &fp->joints[nFTPartsJointTopN]->translate.vec.f;
    Vec2f *ledge;

    if ((fp->player >= MAXCONTROLLERS) || (ftRemixAiPkThunderAdvancedAiApplies(fp) == FALSE) ||
        (ftRemixAiSharedPkThunderAimsAtLedge(fp->status_total_tics) == FALSE))
    {
        return position->y - FTREMIX_AI_SHARED_PKTHUNDER_VANILLA_OFFSET;
    }
    ledge = &sFTRemixAiPkThunderLedge[fp->player];

    return position->y - ftRemixAiSharedPkThunderAimOffset(position->x, position->y, ledge->x, ledge->y);
}

#endif
