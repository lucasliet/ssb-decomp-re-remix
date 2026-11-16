#include "it/itremix/itremixspawnrules.h"

uint32_t itRemixSpawnRulesExtendedRowsMask(int standard_items_num, int vanilla_toggle_items_max)
{
    return (1U << (standard_items_num - vanilla_toggle_items_max)) - 1;
}

uint32_t itRemixSpawnRulesExtendedTogglesAllRowsOff(uint32_t extended_toggles, uint32_t rows_mask)
{
    return extended_toggles & ~rows_mask;
}
