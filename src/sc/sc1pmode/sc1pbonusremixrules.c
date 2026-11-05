/**
 * sc1pbonusremixrules.c - state-free rules of the Remix bonus stage tables, see sc1pbonusremixrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1pbonusremixrules.h"

int sc1PBonusRemixStageIndexOf(const unsigned char *stage_table, int count, int stage_id)
{
    int index;

    for (index = 0; index < count; index++)
    {
        if (stage_table[index] == stage_id)
        {
            return index;
        }
    }
    return -1;
}

int sc1PBonusRemixIsDefaultStage(int stage_id, int default_stage_id)
{
    return (stage_id == default_stage_id) ? 1 : 0;
}

int sc1PBonusRemixStageStep(int index, int direction, int count)
{
    int next = index + direction;

    if (next < 0)
    {
        return count - 1;
    }
    return (next >= count) ? 0 : next;
}

int sc1PBonusRemixCountAfterRun(int stored_count, int new_count)
{
    return (stored_count < new_count) ? new_count : stored_count;
}

unsigned int sc1PBonusRemixRecordTime(int stored_count, unsigned int stored_time)
{
    return (stored_count >= SC1PBONUS_REMIX_TASK_MAX) ? stored_time : SC1PBONUS_REMIX_TIME_UNSET;
}

int sc1PBonusRemixIsTimeBeaten(int stored_count, unsigned int stored_time, unsigned int new_time)
{
    return (new_time < sc1PBonusRemixRecordTime(stored_count, stored_time)) ? 1 : 0;
}

int sc1PBonusRemixStageSource(int is_remix_1p, int is_remix_choice, int fkind, int first_added_fkind)
{
    if (is_remix_1p != 0)
    {
        return SC1PBONUS_REMIX_SOURCE_REMIX_1P_TABLE;
    }
    if (is_remix_choice != 0)
    {
        return SC1PBONUS_REMIX_SOURCE_SELECTED;
    }
    return (fkind >= first_added_fkind) ? SC1PBONUS_REMIX_SOURCE_DEFAULT_TABLE : SC1PBONUS_REMIX_SOURCE_VANILLA;
}

int sc1PBonusRemixRulesIsPlatformStage(int is_vanilla_platforms, int is_vanilla_stage, int stage_class)
{
    if (is_vanilla_stage != 0)
    {
        return (is_vanilla_platforms != 0) ? 1 : 0;
    }
    return (stage_class == SC1PBONUS_REMIX_CLASS_BTP) ? 1 : 0;
}

int sc1PBonusRemixRulesSetStageIsVisible(int is_bonus_12, int is_remix_choice, int is_token_held, int has_fighter)
{
    return ((is_bonus_12 != 0) && (is_remix_choice != 0) && (is_token_held != 0) && (has_fighter != 0)) ? 1 : 0;
}

float sc1PBonusRemixIconX(int icon_count, int index)
{
    if (icon_count < 2)
    {
        return SC1PBONUS_REMIX_ICON_X_SINGLE;
    }
    return (index == 0) ? SC1PBONUS_REMIX_ICON_X_FIRST : SC1PBONUS_REMIX_ICON_X_SECOND;
}

#endif
