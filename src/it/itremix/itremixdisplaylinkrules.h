#ifndef _ITREMIXDISPLAYLINKRULES_H_
#define _ITREMIXDISPLAYLINKRULES_H_

#include <stdint.h>

/* Pure rule of the item display link (Hazards.asm klaptrap_room_). Ints only, so the headless tests can run it. */

#define ITREMIX_DISPLAY_LINK_DEFAULT 0xB
#define ITREMIX_DISPLAY_LINK_HAZARD 0x4
#define ITREMIX_DISPLAY_LINK_CAR 0x12

/**
 * Display link (draw order group) that itManagerMakeItem registers the item with: Klap Trap (0x3A), Robot Bee (0x3B) and
 * Cacodemon (0x3F) use link 4, the Car (0x3C) link 0x12 and every other item the vanilla link 0xB.
 *
 * @param item_kind ITDesc kind of the item.
 */
int32_t itRemixDisplayLinkRulesForKind(int32_t item_kind);

#endif
