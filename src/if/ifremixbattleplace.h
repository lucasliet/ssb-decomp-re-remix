#ifndef _IFREMIXBATTLEPLACE_H_
#define _IFREMIXBATTLEPLACE_H_

#if defined(PORT) && defined(REGION_REMIX)

/**
 * TugOfWar.asm update_sIFCommonBattlePlace_: a fighter brought back by a Tug of War stock undoes the place step its
 * elimination took (sIFCommonBattlePlace is the place of the next fighter or team to be eliminated).
 */
extern void ifRemixBattlePlaceRestoreRevived(void);

#endif

#endif
