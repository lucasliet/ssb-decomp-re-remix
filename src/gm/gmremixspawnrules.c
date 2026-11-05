/**
 * gmremixspawnrules.c - pure rules of the Remix spawn tables, see gmremixspawnrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixspawnrules.h"

#define GMREMIX_SPAWN_RULES_TRAINING_CUSTOM_ID 4

int gmRemixSpawnRulesVsUsesNeutral(int toggle_on, int neutral_slot)
{
    return ((toggle_on != 0) && (neutral_slot >= 0)) ? 1 : 0;
}

int gmRemixSpawnRulesVsSlot(int player, int uses_neutral, int neutral_slot)
{
    return (uses_neutral != 0) ? neutral_slot : player;
}

int gmRemixSpawnRulesTrainingSlot(int spawn_id)
{
    if ((spawn_id < 0) || (spawn_id >= GMREMIX_SPAWN_RULES_TRAINING_CUSTOM_ID))
    {
        return -1;
    }
    return spawn_id;
}

int gmRemixSpawnRulesTableOffset(int stage_id, int slot)
{
    if ((stage_id < 0) || (stage_id > GMREMIX_SPAWN_RULES_STAGE_MAX))
    {
        return -1;
    }
    if ((slot < 0) || (slot >= GMREMIX_SPAWN_RULES_SLOTS_NUM))
    {
        return -1;
    }
    return (stage_id * GMREMIX_SPAWN_RULES_ROW_SIZE) + (slot * GMREMIX_SPAWN_RULES_SLOT_SIZE);
}

#endif
