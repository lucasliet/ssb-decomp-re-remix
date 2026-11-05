#ifndef _GRREMIXBARRELEXIT_H_
#define _GRREMIXBARRELEXIT_H_

/*
 * Exit of a fighter shot from a barrel on the Remix stages (Hazards.asm banjo_btp_barrel_firing_fix and congo_falls_hazard),
 * used by ftCommonTaruCannShootFighter.
 */

#include <ssb_types.h>
#include <ft/fttypes.h>

/**
 * Offset of the map header in the stage file that the throw data of the barrel is measured from: 0 on Banjo's Board the
 * Platforms, whose header starts the file, the vanilla llGRJungleMapMapHeader elsewhere (banjo_btp_barrel_firing_fix).
 */
intptr_t grRemixBarrelExitGetMapHeader(void);

/**
 * congo_falls_hazard: on Congo Falls the fighter leaves the barrel with a hitbox (size 250, 6 damage, base knockback 60, scaling
 * 150, angle 361, 50 tics), by replacing the first move script of its new status.
 *
 * @param fp Fighter that was just shot.
 */
void grRemixBarrelExitSetHitbox(FTStruct *fp);

#endif
