#ifndef _ITREMIXFRANKLINBADGERULES_H_
#define _ITREMIXFRANKLINBADGERULES_H_

#include <stdint.h>

/*
 * Pure rules of the Franklin Badge countdown (FranklinBadge.asm handle_active_franklin_badge_). Ints only, so the
 * headless tests can run them. The flags are the halfword at 0x18C of the N64 player struct, the first 16 bitfields
 * of FTStruct from is_attack_active (0x8000) to is_invisible (0x0001).
 */

#define ITREMIX_FRANKLINBADGE_RULES_FLAG_REFLECT 0x0400    /* is_reflect */
#define ITREMIX_FRANKLINBADGE_RULES_FLAGS_ABSORB 0x0880    /* is_texturepart_modify and is_absorb: the asm "absorbing" value */
#define ITREMIX_FRANKLINBADGE_RULES_FLAGS_NESS 0x0C80      /* the same with is_reflect: the asm "Ness's shield-break flag" */

/* What the countdown does with the reflect hitbox and flag on one frame. */
typedef enum ITRemixFranklinBadgeMaintain
{
    nITRemixFranklinBadgeMaintainNone,          /* another reflector or an absorber owns the hitbox: only count down */
    nITRemixFranklinBadgeMaintainReflect,       /* the badge hitbox is the player's: set the reflect flag */
    nITRemixFranklinBadgeMaintainRestore,       /* the other move ended: write the badge hitbox back and set the flag */
    nITRemixFranklinBadgeMaintainDropReflect    /* 0x0C80 becomes 0x0880: clear the reflect flag, keep the absorb one */

} ITRemixFranklinBadgeMaintain;

/**
 * handle_active_franklin_badge_, from the hitbox compare to _maintain_badge.
 *
 * @param owns_hitbox Non-zero when the player's reflect hitbox pointer is still the badge's.
 * @param flags Halfword at 0x18C of the player struct.
 * @return What to do with the hitbox and the reflect flag on this frame.
 */
ITRemixFranklinBadgeMaintain itRemixFranklinBadgeRulesGetMaintain(int32_t owns_hitbox, uint16_t flags);

/**
 * The asm t2 at _destroy_badge: whether a badge that runs out on this frame leaves the reflect flag as it is. It does
 * while another reflector owns the hitbox (its flag is not the badge's). The asm also leaves it on the frame the badge
 * takes the hitbox back, which keeps the badge reflecting after it ended; the port clears it there
 * (docs/remix/deviations.md).
 *
 * @param maintain Result of itRemixFranklinBadgeRulesGetMaintain for the same frame.
 * @param flags Halfword at 0x18C of the player struct before that frame's changes.
 * @return Non-zero when the expiry must not clear the reflect flag.
 */
int32_t itRemixFranklinBadgeRulesKeepsReflectAtExpiry(ITRemixFranklinBadgeMaintain maintain, uint16_t flags);

#endif
