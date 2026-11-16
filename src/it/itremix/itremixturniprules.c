#include "it/itremix/itremixturniprules.h"

ITRemixTurnipRulesGroundFields itRemixTurnipRulesSplitGroundTimer(uint16_t halfword)
{
    ITRemixTurnipRulesGroundFields fields;

    fields.pickup_wait = halfword >> 4;
    fields.is_allow_knockback = (halfword >> 3) & 1;
    fields.is_unused_item_bool = (halfword >> 2) & 1;
    fields.is_static_damage = (halfword >> 1) & 1;

    return fields;
}
