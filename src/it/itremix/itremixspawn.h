#ifndef _ITREMIXSPAWN_H_
#define _ITREMIXSPAWN_H_

/*
 * Spawn weights and on/off switches of the Remix standard custom items
 * (.toolchain/pkgs/smashremix-src/src/Item.asm, extend_vs_item_spawning_).
 *
 * Standard item n (kind ITREMIX_KIND_START + n) is switched in bit
 * nITKindCommonEnd + 1 + n of the vanilla item_toggles. Remix stops after
 * ITREMIX_VANILLA_TOGGLE_ITEMS_MAX items because the 32-bit mask is full; the
 * rest use EXTENDED_ENABLED_BITMASK (here gITRemixExtendedToggles), bit
 * n - ITREMIX_VANILLA_TOGGLE_ITEMS_MAX. Spawn weights per stage are the bytes of
 * Stages.custom_item_spawn_rate_table in the Remix RAM image.
 */

#include <it/item.h>

#define ITREMIX_STANDARD_ITEMS_NUM 13 /* Item.NUM_STANDARD_ITEMS */
#define ITREMIX_VANILLA_TOGGLE_ITEMS_MAX 12 /* Item.MAX_NUM_STANDARD_ITEMS */
#define ITREMIX_ITEMS_NUM 31 /* Item.NUM_ITEMS: bytes per stage row of the rate table */
#define ITREMIX_EXTENDED_TOGGLES_DEFAULT 0x00000001 /* Item.EXTENDED_ENABLED_BITMASK_DEFAULT */

/** spawn_wait used while SSB64_REMIX_FORCE_ITEM is set (~1s at 60fps). */
#define ITREMIX_DEBUG_SPAWN_WAIT 60

extern u32 gITRemixExtendedToggles;

/** TRUE when standard item `index` is switched on in the current battle. */
sb32 itRemixStandardItemIsOn(s32 index);

/**
 * Spawn weight of standard item `index` on the current stage; 0 when the item is
 * off, has no port or the stage has no row in the rate table.
 */
s32 itRemixStandardItemGetWeight(s32 index);

/** TRUE when any standard item is switched on (the vanilla "any toggle" gate). */
sb32 itRemixAnyStandardItemIsOn(void);

/** Item.asm clear_active_custom_items_: scene-boundary reset of item effect state. */
void itRemixClearActiveItems(void);

/*
 * Item.asm extend_item_switch_bitmask_.save_ stores EXTENDED_ENABLED_BITMASK
 * twice: live (+0x00, gITRemixExtendedToggles) and saved (+0x04). Save is
 * called from the VS item-switch screen after any custom write; restore is
 * called from the CSS scene start.
 */
void itRemixSaveExtendedToggles(void);
void itRemixRestoreExtendedToggles(void);

#endif
