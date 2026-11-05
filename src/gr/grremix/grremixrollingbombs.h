#ifndef _GRREMIXROLLINGBOMBS_H_
#define _GRREMIXROLLINGBOMBS_H_

/*
 * Rolling bombs of the Remix bonus stages (Hazards.asm marina_btt_setup, btp_bomb_or_barrel_setup, rolling_bomb_position,
 * rolling_bomb_explosion_1/2/3): Marina's Break the Targets and the Board the Platforms of Mewtwo and Crash drop a Race to the
 * Finish bomb every 180 frames from a fixed spot. The asm keeps the state of these bombs apart from the Race to the Finish
 * variables, which the targets and platforms of those stages use; so does this module.
 */

#include <ssb_types.h>

/** Starts the bomb spawner of the current stage (bonus stage setup); no-op on stages without one. */
void grRemixRollingBombsMakeGround(void);

/** TRUE when the current stage has the Remix bomb spawner, whose bombs read their item file from grRemixRollingBombsGetItemHeadSlot. */
sb32 grRemixRollingBombsIsStage(void);

/** Slot holding the item file of the bombs of the stage (the asm moves it to 0x8013141C): the stage header itself. */
void** grRemixRollingBombsGetItemHeadSlot(void);

#endif
