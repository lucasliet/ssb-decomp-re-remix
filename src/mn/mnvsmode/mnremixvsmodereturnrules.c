#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnvsmode/mnremixvsmodereturnrules.h"

int mnRemixVSModeReturnCursor(int is_from_players_vs, int mode)
{
    if (!is_from_players_vs)
    {
        return MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR;
    }
    if ((mode < MNREMIX_VSMODE_RETURN_MODE_FIRST) || (mode > MNREMIX_VSMODE_RETURN_MODE_LAST))
    {
        return MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR;
    }
    return mode - MNREMIX_VSMODE_RETURN_MODE_FIRST;
}

int mnRemixVSModeExitSyncsTeamCostumes(int mode, int is_mode_teams)
{
    if (mode == MNREMIX_VSMODE_RETURN_MODE_SMASHKETBALL)
    {
        return 1;
    }
    if ((mode != MNREMIX_VSMODE_RETURN_MODE_DEFAULT) && (mode != MNREMIX_VSMODE_RETURN_MODE_KOTH) &&
        (mode != MNREMIX_VSMODE_RETURN_MODE_TUG_OF_WAR))
    {
        return 0;
    }
    return is_mode_teams != 0;
}

#endif
