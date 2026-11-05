/**
 * ftremixlucasdsp.c - Smash Remix Lucas PSI Magnet (src/Lucas/Lucas.asm dsp_absorption_turnaround_prevent and
 * dsp_absorption_position_fix_): the absorb does not turn Lucas around and its absorb sphere is Lucas's own
 * (psi_magnet_struct_lucas). The heal multiplier is in ftremixpianoshared.c (ftRemixAbsorbHealAmount).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixnessshared.h"
#include "ft/ftremix/ftremixnessrules.h"
#include "remix/remix_roster_data.h"

#define LUCAS_PSIMAGNET_OFFSET_Y 300.0F
#define LUCAS_PSIMAGNET_OFFSET_Z 350.0F
#define LUCAS_PSIMAGNET_SIZE 300.0F

/* psi_magnet_struct_lucas: kind 1 (absorb), joint 0, offset (0, 300, 350), size (300, 300, 300). */
static FTSpecialColl sFTRemixLucasPsiMagnetColl =
{
    nFTSpecialCollKindNessAbsorb,
    0,
    { 0.0F, LUCAS_PSIMAGNET_OFFSET_Y, LUCAS_PSIMAGNET_OFFSET_Z },
    { LUCAS_PSIMAGNET_SIZE, LUCAS_PSIMAGNET_SIZE, LUCAS_PSIMAGNET_SIZE },
    0
};

sb32 ftRemixLucasDSPKeepsFacing(FTStruct *fp)
{
    return ftRemixNessRulesPsiMagnetKeepsFacing(fp->fkind);
}

FTSpecialColl* ftRemixLucasDSPSpecialColl(FTStruct *fp, FTSpecialColl *vanilla_coll)
{
    if (fp->fkind != REMIX_ID_LUCAS)
    {
        return vanilla_coll;
    }
    return &sFTRemixLucasPsiMagnetColl;
}

#endif
