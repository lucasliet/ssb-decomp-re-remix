#ifndef _ITREMIXSUPERMUSHROOMRULES_H_
#define _ITREMIXSUPERMUSHROOMRULES_H_

#include <stdint.h>

/* Pure rules of the mushroom size effect (SuperMushroom.asm handle_active_mushroom_). Ints only, so the headless tests can run them. */

#define ITREMIX_SUPERMUSHROOM_RULES_REVERT_FRAMES 600
#define ITREMIX_SUPERMUSHROOM_RULES_GROW_FRAMES 57
#define ITREMIX_SUPERMUSHROOM_RULES_SHRINK_FRAMES 48
#define ITREMIX_SUPERMUSHROOM_RULES_MAINTAIN_FRAMES \
    (ITREMIX_SUPERMUSHROOM_RULES_REVERT_FRAMES - ITREMIX_SUPERMUSHROOM_RULES_GROW_FRAMES - ITREMIX_SUPERMUSHROOM_RULES_SHRINK_FRAMES)

/**
 * Timer the active effect gets when a mushroom of the other kind is picked up: the frame before its second
 * transition. SuperMushroom.asm:284-288: the bnel on the new item id runs its delay slot (SHRINK - GROW) when the new
 * item is not the Poison Mushroom, which is when the active effect is the Poison one (first transition 48 frames).
 *
 * @param is_new_item_poison Non-zero when the mushroom just picked up is the Poison Mushroom.
 * @return 551 for an active Super effect (new item Poison), 542 for an active Poison effect (new item Super).
 */
int32_t itRemixSuperMushroomRulesGetSupersededTimer(int32_t is_new_item_poison);

#endif
