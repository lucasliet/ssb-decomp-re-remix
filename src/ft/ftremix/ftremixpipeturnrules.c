/**
 * ftremixpipeturnrules.c - pure rule of the Remix pipe_turn table, see ftremixpipeturnrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixpipeturnrules.h"
#include "remix/remix_roster_data.h"

int ftRemixPipeTurnRulesHasEntry(int fkind)
{
    switch (fkind)
    {
    case REMIX_ID_NDRM:
    case REMIX_ID_NWARIO:
    case REMIX_ID_NCONKER:
    case REMIX_ID_JMARIO:
    case REMIX_ID_JLUIGI:
    case REMIX_ID_MLUIGI:
    case REMIX_ID_WARIO:
    case REMIX_ID_GOEMON:
    case REMIX_ID_EBI:
    case REMIX_ID_DRM:
    case REMIX_ID_CONKER:
    case REMIX_ID_DRL:
        return 1;

    default:
        return 0;
    }
}

#endif
