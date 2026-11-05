/**
 * ftremixvarianttunerules.c - pure rules of small per-fighter tuning patches, see ftremixvarianttunerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixvarianttunerules.h"
#include "remix/remix_roster_data.h"

float ftRemixVariantTuneRulesSpinningKongAirVelY(int fkind, float vanilla_vel_y)
{
    return (fkind == REMIX_ID_JDK) ? FTREMIX_TUNE_JDK_SPINNINGKONG_AIR_VEL_Y : vanilla_vel_y;
}

float ftRemixVariantTuneRulesFalconDiveAirAccelMul(int fkind, float vanilla_mul)
{
    return (fkind == REMIX_ID_JFALCON) ? FTREMIX_TUNE_JFALCON_FALCONDIVE_AIR_ACCEL_MUL : vanilla_mul;
}

float ftRemixVariantTuneRulesFalconDiveAirSpeedMaxMul(int fkind, float vanilla_mul)
{
    return (fkind == REMIX_ID_JFALCON) ? FTREMIX_TUNE_JFALCON_FALCONDIVE_AIR_SPEED_MAX_MUL : vanilla_mul;
}

float ftRemixVariantTuneRulesFireFoxPitchOffset(int fkind, float vanilla_offset)
{
    return (fkind == REMIX_ID_PEPPY) ? FTREMIX_TUNE_PEPPY_FIREFOX_PITCH_OFFSET : vanilla_offset;
}

int ftRemixVariantTuneRulesEntryKeepsFacing(int fkind)
{
    if (fkind >= REMIX_ID_NWARIO)
    {
        return 1;
    }
    if (fkind <= REMIX_ID_METAL)
    {
        return 0;
    }
    if (fkind < REMIX_ID_GDONKEY)
    {
        return 1;
    }
    return (fkind == REMIX_ID_SONIC) || (fkind == REMIX_ID_SSONIC);
}

#endif
