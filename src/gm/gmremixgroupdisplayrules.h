#ifndef _GMREMIXGROUPDISPLAYRULES_H_
#define _GMREMIXGROUPDISPLAYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <stdint.h>

/*
 * Display group flag rules (Render.asm toggle_group_display_, SpecialZoom.asm temp_toggle_group_display_), pure so the
 * headless unit tests (tests/remix) can run them. A "group" is one GObj link list (gGCCommonLinks, Render.GROUP_TABLE).
 */

/**
 * New GObj flags word of Render.toggle_group_display_: the asm stores the hide argument as the whole word (sw a1, 0x7C).
 *
 * @param hide 1 hides the GObj, 0 shows it.
 */
extern uint32_t gmRemixGroupDisplayRulesSetFlags(uint32_t hide);

/**
 * New GObj flags word of SpecialZoom.temp_toggle_group_display_: hiding saves the old word in the upper half
 * ((flags << 16) | hide), showing restores it (flags >> 16).
 *
 * @param flags Current flags word.
 * @param hide Nonzero hides and saves, 0 restores.
 */
extern uint32_t gmRemixGroupDisplayRulesTempFlags(uint32_t flags, uint32_t hide);

#endif

#endif
