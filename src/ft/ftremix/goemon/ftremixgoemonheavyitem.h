#ifndef _FTREMIXGOEMONHEAVYITEM_H_
#define _FTREMIXGOEMONHEAVYITEM_H_

/*
 * Goemon golden hair while he carries a heavy item (.toolchain/pkgs/smashremix-src/src/Goemon/Goemon.asm
 * heavy_item_pickup_moveset, heavy_item_hold_moveset, heavy_item_turn_moveset and their _2 flag hooks).
 * Declared here, defined in ftremixgoemonheavyitem.c.
 */

#include <ft/fighter.h>

/* ftCommonGetSetStatus tail (0x80146048): the heavy item pickup script, which plays FGM 0x86 and shows the golden hair */
void ftRemixGoemonHeavyItemPickupScript(FTStruct *fp);

/* ftCommonLiftWaitSetStatus (0x80146110): the hold script, which keeps the golden hair */
void ftRemixGoemonHeavyItemHoldScript(FTStruct *fp);

/* ftCommonLiftTurnSetStatus (0x80146230): the turn script, the same script as the hold script */
void ftRemixGoemonHeavyItemTurnScript(FTStruct *fp);

/* ftCommonLiftWaitSetStatus and ftCommonLiftTurnSetStatus (0x801460F4, 0x80146214): Goemon also preserves the model parts */
u32 ftRemixGoemonHeavyItemStatusFlags(FTStruct *fp, u32 vanilla_flags);

#endif
