/**
 * ftremixmainrules.c - pure rules of the ftMainSetStatus and ftMainProcPhysicsMap Remix hooks, see ftremixmainrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixmainrules.h"
#include "remix/remix_roster_data.h"


int ftRemixMainIsMapStarFighter(int fkind)
{
    return (fkind == REMIX_ID_KIRBY) || (fkind == REMIX_ID_JKIRBY) || (fkind == REMIX_ID_DEDEDE);
}

int ftRemixMainMarinaThrownFile(int victim_fkind)
{
    switch (victim_fkind)
    {
    case REMIX_ID_DONKEY:
    case REMIX_ID_NDONKEY:
    case REMIX_ID_GDONKEY:
    case REMIX_ID_JDK:
        return FTREMIXMAIN_FILE_DK_THROWN_MARINA;

    case REMIX_ID_CAPTAIN:
    case REMIX_ID_NCAPTAIN:
    case REMIX_ID_GND:
    case REMIX_ID_NGND:
    case REMIX_ID_SHEIK:
    case REMIX_ID_NSHEIK:
    case REMIX_ID_DRAGONKING:
    case REMIX_ID_JFALCON:
    case REMIX_ID_SANDBAG:
        return FTREMIXMAIN_FILE_FALCON_THROWN_MARINA;

    default:
        return 0;
    }
}

/* Weapon hit: vanilla ftMainProcessHitCollisionStatsMain threshold on |vel x|; beam_knockback_direction_fix threshold on |lr| */
#define FTREMIXMAIN_WEAPON_HIT_VEL_MIN 5.0F
#define FTREMIXMAIN_BEAM_HIT_LR_MIN 0.5F

int ftRemixMainWeaponHitLr(int weapon_kind, int weapon_lr, float vel_x)
{
    float read_value = vel_x;
    float read_min = FTREMIXMAIN_WEAPON_HIT_VEL_MIN;

    if (weapon_kind == FTREMIXMAIN_WPKIND_WADDLEDOO_BEAM)
    {
        read_value = (float)weapon_lr;
        read_min = FTREMIXMAIN_BEAM_HIT_LR_MIN;
    }
    if (((read_value < 0.0F) ? -read_value : read_value) < read_min)
    {
        return 0;
    }
    return (read_value < 0.0F) ? +1 : -1;
}

#endif
