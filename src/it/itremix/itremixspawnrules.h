#ifndef _ITREMIXSPAWNRULES_H_
#define _ITREMIXSPAWNRULES_H_

#include <stdint.h>

/* Pure rules of the Item Switch extended mask (Item.asm extend_item_switch_bitmask_.save_). Ints only, so the headless tests can run them. */

/**
 * Mask of the extended bits that belong to custom item rows (Item.asm NUM_STANDARD_ITEMS - MAX_NUM_STANDARD_ITEMS rows).
 *
 * @param standard_items_num Number of custom item rows (Item.NUM_STANDARD_ITEMS).
 * @param vanilla_toggle_items_max Rows held in the vanilla item_toggles word (Item.MAX_NUM_STANDARD_ITEMS).
 * @return One bit per extended row, bit 0 first.
 */
uint32_t itRemixSpawnRulesExtendedRowsMask(int standard_items_num, int vanilla_toggle_items_max);

/**
 * Extended mask after the Item Switch exit with every row off: the save_ loop clears the bit of each row that is off.
 *
 * @param extended_toggles Live extended mask.
 * @param rows_mask Result of itRemixSpawnRulesExtendedRowsMask.
 * @return The mask with every row bit cleared and the other bits kept.
 */
uint32_t itRemixSpawnRulesExtendedTogglesAllRowsOff(uint32_t extended_toggles, uint32_t rows_mask);

#endif
