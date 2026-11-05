/**
 * ftremixchartables.c - Remix per-character jump tables run from vanilla code.
 *
 * Character.asm keeps per-id tables that vanilla routines dispatch through:
 * grounded_script (get_grounded_script_, 0x800DE408, in
 * mpCommonSetFighterLandingParams) and on_hit (OnHit.asm, 0x800E8244, in
 * ftParamStopVoiceRunProcDamage). A new character inherits its parent's
 * grounded_script entry unless its own file patches it.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixhooks.h"
#include "ft/ftremix/ftremixgroundedrules.h"

/* 0x800DE428: the vanilla Mario/Luigi/Captain/Purin case, `sw r0, 0x0ADC(v0)` */
static void ftRemixGroundedClearPassiveWord(FTStruct *fp)
{
    fp->passive_vars.mario.is_expend_tornado = FALSE;
}

/* Samus case of the vanilla table (mpcommon.c): charge_recoil (N64 0x0AE0) = 0 */
static void ftRemixGroundedClearChargeRecoil(FTStruct *fp)
{
    fp->passive_vars.samus.charge_recoil = 0;
}

/* Ebi.asm:466 grounded_script_: both passive words are cleared before the disabled return */
static void ftRemixGroundedEbiClearWords(FTStruct *fp)
{
    fp->passive_vars.mario.is_expend_tornado = FALSE;
    fp->passive_vars.samus.charge_recoil = 0;
}

void ftRemixRunGroundedScript(FTStruct *fp)
{
    switch (ftRemixGroundedScriptKind(fp->fkind))
    {
    case nFTRemixGroundedScriptClearWord:
        ftRemixGroundedClearPassiveWord(fp);
        break;

    case nFTRemixGroundedScriptClearRecoil:
        ftRemixGroundedClearChargeRecoil(fp);
        break;

    case nFTRemixGroundedScriptEbi:
        ftRemixGroundedEbiClearWords(fp);
        break;

    case nFTRemixGroundedScriptKirbyHat:
        ftRemixJKirbyGroundedScript(fp);
        break;

    case nFTRemixGroundedScriptPeachFloat:
        ftRemixPeachGroundedScriptRestoreFloat(fp);
        break;

    default:
        break;
    }
}

static sb32 ftRemixIsBowser(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_BOWSER) || (fp->fkind == REMIX_ID_GBOWSER);
}

/* BowserUSP.ledge_grab_patch_ (BowserSpecial.asm:1060, 0x8015EC64): temp variable 2 = 1 */
void ftRemixBowserUSPLedgeGrabPatch(FTStruct *fp)
{
    if (ftRemixIsBowser(fp))
    {
        fp->motion_vars.flags.flag1 = 1;
    }
}

/* BowserDSP.skip_projectile_ (BowserSpecial.asm:1157, 0x8015EE24): no Yoshi Bomb stars */
sb32 ftRemixBowserDSPSkipsStars(FTStruct *fp)
{
    return ftRemixIsBowser(fp);
}

/* GoemonDSP.item_hurtbox_patch_ (GoemonSpecial.asm:2761, 0x80170698): the chain pull
 * (DSPGround 0xEF, DSPAir 0xF4) hits items even though it sets the catch status */
sb32 ftRemixGoemonDSPIgnoresCatchStatus(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_GOEMON) && ((fp->status_id == 0xEF) || (fp->status_id == 0xF4));
}

void ftRemixRunOnHit(FTStruct *fp)
{
    switch (fp->fkind)
    {
    case REMIX_ID_SONIC:  /* SonicUSPRefresh (SonicSpecial.asm:1184): up special is usable again */
    case REMIX_ID_SSONIC: /* SonicSpecial.asm:1194 registers the same routine */
        fp->passive_vars.mario.is_expend_tornado = FALSE;
        break;

    case REMIX_ID_EBI: /* EbiUSPRefresh (EbiSpecial.asm:700): up special is usable again */
        ftRemixEbiUSPRefresh(fp);
        break;

    default:
        break;
    }
}

#endif
