/**
 * ftremixeffectsrules.c - pure rules of the fighter effect patches, see ftremixeffectsrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixeffectsrules.h"
#include "remix/remix_roster_data.h"

/* wolf_reflector_struct (Wolf.asm) and slippy_reflector_struct: the start, loop, hit and end animation joint offsets of file 7 */
static const intptr_t sReflectorAnimJointsWolf[4] = { 0x41C, 0x4C4, 0x5E8, 0x6FC };
static const intptr_t sReflectorAnimJointsSlippy[4] = { 0x320, 0x4C0, 0x3A0, 0x430 };

/* wolf_reflect_graphic_struct and slippy_reflect_graphic_struct: o_dobjsetup and the initial o_anim_joint. Slippy's initial
 * animation joint (0x340) differs from the first entry of his table (0x320) in the asm. */
#define FTREMIX_EFFECTS_REFLECTOR_WOLF_O_DOBJSETUP 0x288
#define FTREMIX_EFFECTS_REFLECTOR_WOLF_O_ANIM_JOINT 0x41C
#define FTREMIX_EFFECTS_REFLECTOR_SLIPPY_O_DOBJSETUP 0x2B0
#define FTREMIX_EFFECTS_REFLECTOR_SLIPPY_O_ANIM_JOINT 0x340

float ftRemixEffectsRulesInhaleWindOffsetX(int fkind)
{
    if (fkind == REMIX_ID_DEDEDE)
    {
        return FTREMIX_EFFECTS_INHALE_WIND_X_DEDEDE;
    }
    return FTREMIX_EFFECTS_INHALE_WIND_X_KIRBY;
}

int ftRemixEffectsRulesInhaleBreakoutWait(int absorber_fkind, int captured_percent, int vanilla_wait)
{
    if (absorber_fkind != REMIX_ID_DEDEDE)
    {
        return vanilla_wait;
    }
    return FTREMIX_EFFECTS_INHALE_BREAKOUT_BASE_DEDEDE + captured_percent;
}

FTRemixEffectsPunchGfx ftRemixEffectsRulesPunchGfx(int power_kind)
{
    switch (power_kind)
    {
    case REMIX_ID_GND:
    case REMIX_ID_JFALCON:
    case REMIX_ID_DRAGONKING:
        return nFTRemixEffectsPunchGfxOwnFile8;

    case REMIX_ID_WOLF:
        return nFTRemixEffectsPunchGfxWolfSlash;

    default:
        return nFTRemixEffectsPunchGfxFalcon;
    }
}

int ftRemixEffectsRulesPunchAnnounces(int power_kind)
{
    return (power_kind != REMIX_ID_GND) && (power_kind != REMIX_ID_WOLF) && (power_kind != REMIX_ID_MTWO);
}

int ftRemixEffectsRulesReflectorUsesOwnFile(int fkind)
{
    return (fkind == REMIX_ID_WOLF) || (fkind == REMIX_ID_SLIPPY);
}

intptr_t ftRemixEffectsRulesReflectorDObjSetup(int fkind, intptr_t vanilla_offset)
{
    if (fkind == REMIX_ID_WOLF)
    {
        return FTREMIX_EFFECTS_REFLECTOR_WOLF_O_DOBJSETUP;
    }
    if (fkind == REMIX_ID_SLIPPY)
    {
        return FTREMIX_EFFECTS_REFLECTOR_SLIPPY_O_DOBJSETUP;
    }
    return vanilla_offset;
}

intptr_t ftRemixEffectsRulesReflectorFirstAnimJoint(int fkind, intptr_t vanilla_offset)
{
    if (fkind == REMIX_ID_WOLF)
    {
        return FTREMIX_EFFECTS_REFLECTOR_WOLF_O_ANIM_JOINT;
    }
    if (fkind == REMIX_ID_SLIPPY)
    {
        return FTREMIX_EFFECTS_REFLECTOR_SLIPPY_O_ANIM_JOINT;
    }
    return vanilla_offset;
}

intptr_t ftRemixEffectsRulesReflectorAnimJoint(int fkind, int anim_id, intptr_t vanilla_offset)
{
    if ((anim_id < 0) || (anim_id > 3))
    {
        return vanilla_offset;
    }
    if (fkind == REMIX_ID_WOLF)
    {
        return sReflectorAnimJointsWolf[anim_id];
    }
    if (fkind == REMIX_ID_SLIPPY)
    {
        return sReflectorAnimJointsSlippy[anim_id];
    }
    return vanilla_offset;
}

int ftRemixEffectsRulesPsiMagnetUsesFile6(int fkind)
{
    return (fkind == REMIX_ID_JNESS) || (fkind == REMIX_ID_LUCAS) || (fkind == REMIX_ID_NLUCAS);
}

#endif
