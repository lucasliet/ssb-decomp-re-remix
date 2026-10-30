#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixailongrangerules.h"

/** TRUE for every fighter kind except the vanilla ids 0x00-0x0D and 0x1A-0x1D, which keep the vanilla switch. */
int ftRemixAiLongRangeRulesUsesTable(int fkind)
{
    if (fkind < FTREMIX_AI_LONG_RANGE_NMARIO)
    {
        return 0;
    }
    if ((fkind >= 0x1A) && (fkind <= 0x1D))
    {
        return 0;
    }
    return 1;
}

/** TRUE for the NMario..NNess variants and the Remix polygons, from the roster variant POLYGON. */
int ftRemixAiLongRangeRulesIsPolygon(int fkind)
{
    if ((fkind >= FTREMIX_AI_LONG_RANGE_NMARIO) && (fkind <= FTREMIX_AI_LONG_RANGE_NNESS))
    {
        return 1;
    }
    return (fkind >= FTREMIX_AI_LONG_RANGE_NWARIO) && (fkind <= FTREMIX_AI_LONG_RANGE_NCRASH);
}

/** Matches the three vanilla LONG_RANGE routine addresses; any other value is not a routine this port knows. */
int ftRemixAiLongRangeRulesRoutineKind(unsigned int routine_addr)
{
    if (routine_addr == FTREMIX_AI_LONG_RANGE_ROUTINE_NSP_SHOOT)
    {
        return FTREMIX_AI_LONG_RANGE_KIND_NSP_SHOOT;
    }
    if (routine_addr == FTREMIX_AI_LONG_RANGE_ROUTINE_PHYSICAL_SHOOT)
    {
        return FTREMIX_AI_LONG_RANGE_KIND_PHYSICAL_SHOOT;
    }
    if (routine_addr == FTREMIX_AI_LONG_RANGE_ROUTINE_NONE)
    {
        return FTREMIX_AI_LONG_RANGE_KIND_NONE;
    }
    return FTREMIX_AI_LONG_RANGE_KIND_UNKNOWN;
}

/** Table command 0 means no custom move for this fighter, so the default command stays. */
int ftRemixAiLongRangeRulesCustomCommand(int table_command, int default_command)
{
    if (table_command != 0)
    {
        return table_command;
    }
    return default_command;
}

#endif
