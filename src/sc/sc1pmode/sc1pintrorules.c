/**
 * sc1pintrorules.c - state-free rules of the 1P title cards, see sc1pintrorules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1pintrorules.h"

#define SC1PINTRO_RULES_ALLY_POSITION_HUMAN_ALONE 0
#define SC1PINTRO_RULES_ALLY_POSITION_HUMAN_ONE_ALLY 1
#define SC1PINTRO_RULES_ALLY_POSITION_HUMAN_TWO_ALLIES 3

int sc1PIntroRulesHumanSidePort(int card_anim_frame_id, int human_port)
{
    switch (card_anim_frame_id)
    {
    case SC1PINTRO_RULES_ALLY_POSITION_HUMAN_ALONE:
    case SC1PINTRO_RULES_ALLY_POSITION_HUMAN_ONE_ALLY:
    case SC1PINTRO_RULES_ALLY_POSITION_HUMAN_TWO_ALLIES:
        return human_port;

    default:
        return sc1PIntroRulesOpponentPort(human_port);
    }
}

int sc1PIntroRulesOpponentPort(int human_port)
{
    return (human_port == 0) ? 1 : 0;
}

/** TRUE for the original polygons (Polygon Mario to Polygon Ness), the only ones that take the alternate costume as opponents. */
static int sc1PIntroRulesIsOriginalPolygon(int fkind)
{
    return ((fkind >= REMIX_ID_NMARIO) && (fkind <= REMIX_ID_NNESS)) ? 1 : 0;
}

/** TRUE for every polygon: the original ones and the Remix polygons, which have ids from Polygon Wario on. */
static int sc1PIntroRulesIsPolygon(int fkind)
{
    return ((sc1PIntroRulesIsOriginalPolygon(fkind) != 0) || (fkind >= REMIX_ID_NWARIO)) ? 1 : 0;
}

/** TRUE when a human with costume 0 shares the look of this opponent: a polygon against an original polygon, Giant DK against DK. */
static int sc1PIntroRulesIsLookAlikeMatch(int cpu_fkind, int human_fkind)
{
    if (sc1PIntroRulesIsPolygon(human_fkind) != 0)
    {
        return sc1PIntroRulesIsOriginalPolygon(cpu_fkind);
    }
    return ((human_fkind == REMIX_ID_GDONKEY) && (cpu_fkind == REMIX_ID_DONKEY)) ? 1 : 0;
}

int sc1PIntroRulesOpponentCostumeIndex(int cpu_fkind, int human_fkind, int human_costume, int is_remix_1p, int stage)
{
    if (human_costume != 0)
    {
        return 0;
    }
    if (sc1PIntroRulesIsLookAlikeMatch(cpu_fkind, human_fkind) != 0)
    {
        return 1;
    }
    if ((is_remix_1p != 0) && (stage == SC1PINTRO_RULES_STAGE_DONKEY) && (cpu_fkind == human_fkind))
    {
        return 1;
    }
    return 0;
}

int sc1PIntroRulesDuoMarioSlotCostumeIndex(int is_remix_1p, int stage, int slot_fkind, int human_fkind, int human_costume)
{
    if ((is_remix_1p == 0) || (stage != SC1PINTRO_RULES_STAGE_MMARIO) || (slot_fkind != REMIX_ID_METAL))
    {
        return 1;
    }
    if (human_fkind != REMIX_ID_METAL)
    {
        return 0;
    }
    return (human_costume == 0) ? 1 : 0;
}

int sc1PIntroRulesNameWidthApplies(int stage)
{
    return ((stage == SC1PINTRO_RULES_STAGE_YOSHI) || (stage == SC1PINTRO_RULES_STAGE_DONKEY)) ? 1 : 0;
}

int sc1PIntroRulesTinyTeamCostume(int member_index, int fkind)
{
    return (fkind > 0) ? (member_index % fkind) : 0;
}

#endif
