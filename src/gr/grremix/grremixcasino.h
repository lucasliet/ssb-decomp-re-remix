#ifndef _GRREMIXCASINO_H_
#define _GRREMIXCASINO_H_

/*
 * Casino Night Zone side bumper knockback (Hazards.asm casino_night_side_bumper_knockback). The wall bounce scale is
 * declared in ft/ftremix/ftremixhooks.h.
 */

#include <ft/fighter.h>

/**
 * Hook of ftMainGetGroundHitObstacle at 0x800E5C40: on Casino Night Zone, a fighter outside the damage statuses that
 * touches a side bumper wall takes the knockback of that wall surface. Writes the material to use in place of the floor
 * material and returns TRUE, or returns FALSE when the vanilla floor lookup applies.
 */
sb32 grRemixCasinoGetBumperMaterial(FTStruct *fp, u32 *material);

/**
 * Test aid: with SSB64_REMIX_CASINO_LOG=1, logs at stage start every wall segment of Casino Night Zone whose surface is above
 * 0x16 (`[remix-casino] bumper wall line=<id> side=<left|right> surface=0x<nn> from=(x,y) to=(x,y)`). No-op on other stages.
 */
void grRemixCasinoLogBumperWalls(void);

#endif
