#include "gr/grremix/grremixsaffronrules.h"

#define GRREMIX_SAFFRON_QUICK_ATTACK_CLOSED_MONSTER_WAIT 15
#define GRREMIX_SAFFRON_QUICK_ATTACK_SLEEP_MONSTER_WAIT 1
#define GRREMIX_SAFFRON_INSTANT_GATE_WAIT 1

int32_t grRemixSaffronRateSleepMonsterWait(int32_t rate, int32_t base_wait)
{
    if (rate == GRREMIX_SAFFRON_RATE_DEFAULT)
    {
        return base_wait;
    }
    if (rate < GRREMIX_SAFFRON_RATE_QUICK_ATTACK)
    {
        return base_wait / 2;
    }
    return GRREMIX_SAFFRON_QUICK_ATTACK_SLEEP_MONSTER_WAIT;
}

int32_t grRemixSaffronRateClosedMonsterWait(int32_t rate, int32_t base_wait)
{
    if (rate == GRREMIX_SAFFRON_RATE_DEFAULT)
    {
        return base_wait;
    }
    if (rate < GRREMIX_SAFFRON_RATE_QUICK_ATTACK)
    {
        return base_wait / 2;
    }
    return GRREMIX_SAFFRON_QUICK_ATTACK_CLOSED_MONSTER_WAIT;
}

int32_t grRemixSaffronRateClosedGateWait(int32_t rate, int32_t base_wait)
{
    if (rate == GRREMIX_SAFFRON_RATE_DEFAULT)
    {
        return base_wait;
    }
    if (rate == GRREMIX_SAFFRON_RATE_SUPER)
    {
        return base_wait / 2;
    }
    return GRREMIX_SAFFRON_INSTANT_GATE_WAIT;
}
