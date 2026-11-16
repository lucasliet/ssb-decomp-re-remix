#include "it/itremix/itremixsupermushroomrules.h"

int32_t itRemixSuperMushroomRulesGetSupersededTimer(int32_t is_new_item_poison)
{
    int32_t timer = ITREMIX_SUPERMUSHROOM_RULES_GROW_FRAMES + ITREMIX_SUPERMUSHROOM_RULES_MAINTAIN_FRAMES - 1;

    if (is_new_item_poison == 0)
    {
        timer += ITREMIX_SUPERMUSHROOM_RULES_SHRINK_FRAMES - ITREMIX_SUPERMUSHROOM_RULES_GROW_FRAMES;
    }
    return timer;
}
