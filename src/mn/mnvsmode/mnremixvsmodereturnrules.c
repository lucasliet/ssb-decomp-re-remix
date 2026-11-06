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

#endif
