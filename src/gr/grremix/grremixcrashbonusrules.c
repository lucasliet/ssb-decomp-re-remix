#include "gr/grremix/grremixcrashbonusrules.h"
#include "gr/grremix/grremixids.h"

#include <stddef.h>

static const GRRemixCrashBonusOneUp dGRRemixCrashBonusBttOneUp = { 9, 0x14 / 4 };
static const GRRemixCrashBonusOneUp dGRRemixCrashBonusBtpOneUp = { 0x16, 0x3C / 4 };

const GRRemixCrashBonusOneUp* grRemixCrashBonusRulesFind(int32_t gkind)
{
    if (gkind == nGRRemixStage_BTT_CRASH)
    {
        return &dGRRemixCrashBonusBttOneUp;
    }
    if (gkind == nGRRemixStage_BTP_CRASH)
    {
        return &dGRRemixCrashBonusBtpOneUp;
    }
    return NULL;
}

int32_t grRemixCrashBonusRulesIsCollected(const GRRemixCrashBonusOneUp *one_up, int32_t floor_line_id)
{
    return (floor_line_id == one_up->line_id) ? 1 : 0;
}
