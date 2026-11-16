#ifndef _ITREMIXTURNIPRULES_H_
#define _ITREMIXTURNIPRULES_H_

#include <stdint.h>

/* Pure rules of the Turnip ground timer (Turnip.asm falling_initial_). Ints only, so the headless tests can run them. */

#define ITREMIX_TURNIP_RULES_GROUND_TIMER 0x030C /* Turnip.asm GROUND_TIMER, stored with "sh" at 0x02D2 of the item struct */

/* The item fields that share the N64 halfword at 0x02D2: pickup_wait in the top 12 bits, then three flags and a pad bit. */
typedef struct ITRemixTurnipRulesGroundFields
{
    uint16_t pickup_wait;
    uint8_t is_allow_knockback;
    uint8_t is_unused_item_bool;
    uint8_t is_static_damage;
} ITRemixTurnipRulesGroundFields;

/**
 * Splits a halfword stored at 0x02D2 of the N64 item struct into the item fields it covers.
 *
 * @param halfword Value of the asm's "sh" (ITREMIX_TURNIP_RULES_GROUND_TIMER for the Turnip).
 * @return pickup_wait (halfword >> 4), is_allow_knockback (bit 3), is_unused_item_bool (bit 2), is_static_damage (bit 1).
 */
ITRemixTurnipRulesGroundFields itRemixTurnipRulesSplitGroundTimer(uint16_t halfword);

/**
 * Turnip.asm falling_initial_ 284-286: clears mask 0x02 of N64 0x2CE ("don't randomly despawn"), the low bit of the
 * port's times_thrown.
 *
 * @param times_thrown Current times_thrown of the item.
 * @return times_thrown with its low bit cleared.
 */
uint8_t itRemixTurnipRulesClearDespawnGuard(uint8_t times_thrown);

#endif
