#ifndef _SC1PREMIXALLSTARFINAL_H_
#define _SC1PREMIXALLSTARFINAL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objtypes.h>

/*
 * HUD of the All-Star final battle (SinglePlayerModes.asm _SC1PGameEnemyTeamSprites_expansion, _stock_loading_allstar,
 * _SC1PGameEnemyTeamSprites_allstar_yoshi_path, _allstar_final_costume_address_fix, _allstar_final_series_stock_set): the team of
 * 24 members is the last three fighters of the run, so the enemy stock row shows the own stock icon and costume of each member
 * instead of the polygon icon, and the damage panel of the CPU follows the member that spawns.
 */

/**
 * Remembers the stock sprites and series logos of the fighters of the final team (the enemy fighters already made), so the stock
 * row and the panel can use them later. Call after the enemy fighters exist; no-op outside the All-Star final.
 */
extern void sc1PRemixAllstarFinalIconsSetup(void);

/**
 * Gives an icon of the enemy stock row the stock icon of a member of the final team, with the costume palette of the member.
 *
 * @param sobj Icon of the row; its position and visibility are kept.
 * @param member Member of the team, 0 to SC1PREMIX_ALLSTAR_FINAL_MEMBERS - 1.
 * @return TRUE when the icon was set; FALSE outside the All-Star final or when the fighter has no stock icon, and the caller then
 * keeps the polygon icon.
 */
extern sb32 sc1PRemixAllstarFinalIconsSetStock(SObj *sobj, s32 member);

/**
 * The next member of the final team spawned in a CPU slot: its series logo and stock icon replace the ones the panel kept from
 * the first fighter of the slot (_allstar_final_series_stock_set). No-op outside the All-Star final.
 *
 * @param player Port of the slot.
 */
extern void sc1PRemixAllstarFinalIconsOnSpawn(s32 player);

#endif

#endif
