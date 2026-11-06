#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1pbonusstagerules.h"

int sc1PBonusStageRulesIsNewRecord(int is_record_complete, unsigned int record_time, unsigned int time_passed)
{
    if (is_record_complete == 0)
    {
        return 0;
    }
    return (time_passed < record_time) ? 1 : 0;
}

int sc1PBonusStageRulesGkindFor1PGame(int fkind, int first_added_fkind, int default_stage_id, int no_stage_id, int vanilla_gkind)
{
    if ((fkind < first_added_fkind) || (default_stage_id == no_stage_id))
    {
        return vanilla_gkind;
    }
    return default_stage_id;
}

int sc1PBonusStageRulesUsesCountUpTimer(int is_bonus3, int is_hrc, int is_practice)
{
    return ((is_bonus3 != 0) || (is_hrc != 0) || (is_practice != 0)) ? 1 : 0;
}

int sc1PBonusStageRulesHrcTimerTics(unsigned int time_passed)
{
    if (time_passed > SC1PBONUSSTAGE_HRC_TIMER_TICS)
    {
        return -1;
    }
    return SC1PBONUSSTAGE_HRC_TIMER_TICS - (int)time_passed;
}

#endif
