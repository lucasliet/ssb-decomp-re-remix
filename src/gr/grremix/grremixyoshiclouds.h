#ifndef _GRREMIXYOSHICLOUDS_H_
#define _GRREMIXYOSHICLOUDS_H_

/*
 * Yoshi's Island cloud hazards and platform movement gate of the Remix build (Hazards.asm
 * yoshis_island_clouds_, yoshis_island_cloud_anims_, platform_movement_). The vanilla files
 * (gryoster.c, grdisplay.c) call these under #ifdef REGION_REMIX.
 */

#include <ssb_types.h>

/** Hazards off: disables the clipping of the three cloud collision DObjs so they are not hidden at the stage centre. */
void grRemixYosterDisableCloudClipping(void);

/** Matrix kind of the cloud DObjs: the scale kind when the cloud scale toggle is on, otherwise the vanilla translate kind. */
u8 grRemixYosterCloudMatrixKind(void);

/** TRUE when the platform movement process of a ground display may be registered (VS/Training hazard and movement toggles). */
sb32 grRemixPlatformMovementIsOn(void);

#endif
