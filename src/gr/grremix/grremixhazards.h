#ifndef _GRREMIXHAZARDS_H_
#define _GRREMIXHAZARDS_H_

/*
 * Hazard setups of the Remix custom stages (Hazards.asm). Stages.function_table
 * holds the Remix RAM address of the setup routine; grRemixMakeGround hands that
 * address here. Each handler returns TRUE when it recognised the address and
 * built the stage's hazards.
 */

#include <ssb_types.h>

sb32 grRemixHazardsMakeGround(s32 gkind, u32 function);

/** Toggles.entry_hazard_mode of the Remix menu; both default to on until the Remix settings menu exists. */
sb32 grRemixHazardsAreOn(void);
sb32 grRemixMovementIsOn(void);

/** Hazard intro check of the asm: always true in training mode, otherwise once the match clock has started. */
sb32 grRemixHazardsMatchHasStarted(void);

/**
 * Top of the item data file of a hazard stage (Japes KlapTrap, Metallic Madness Robot Bee, Onett Car): the item
 * pointer stored at stage header + 0xE0 minus 0x07E8, which the asm saves at 0x801313F0. Animation track offsets are
 * relative to it.
 */
void* grRemixHazardsItemFileTop(void *stage_header);

sb32 grRemixHazardsMakeGroundBayDraculaTwilight(s32 gkind, u32 function);

sb32 grRemixHazardJapes(s32 gkind, u32 function);
sb32 grRemixHazardCorneria(s32 gkind, u32 function);
sb32 grRemixHazardGbLand(s32 gkind, u32 function);
sb32 grRemixHazardPirate(s32 gkind, u32 function);
sb32 grRemixHazardCasino(s32 gkind, u32 function);
sb32 grRemixHazardMetallic(s32 gkind, u32 function);
sb32 grRemixHazardOnett(s32 gkind, u32 function);
sb32 grRemixHazardTurnpike(s32 gkind, u32 function);
sb32 grRemixHazardRainbow(s32 gkind, u32 function);
sb32 grRemixHazardMmm(s32 gkind, u32 function);
sb32 grRemixHazardBbh(s32 gkind, u32 function);
sb32 grRemixHazardSpawnedFear(s32 gkind, u32 function);
sb32 grRemixHazardRttf(s32 gkind, u32 function);
sb32 grRemixHazardTimeTwister(s32 gkind, u32 function);
sb32 grRemixHazardScuttleTown(s32 gkind, u32 function);
sb32 grRemixHazardPeachCastle2(s32 gkind, u32 function);

#endif
