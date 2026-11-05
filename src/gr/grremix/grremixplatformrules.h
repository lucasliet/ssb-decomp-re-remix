#ifndef _GRREMIXPLATFORMRULES_H_
#define _GRREMIXPLATFORMRULES_H_

#include <stdint.h>

/* Pure rule of the platform movement gate (Hazards.asm platform_movement_). Ints only, so the headless tests can run it. */

/** Stages.asm Hazards.type bit: the stage has hazards. */
#define GRREMIX_PLATFORM_HAZARD_TYPE_HAZARDS 0x01

/** Stages.asm Hazards.type bit: the stage has moving platforms (the inverse of the asm dont_freeze_stage flag). */
#define GRREMIX_PLATFORM_HAZARD_TYPE_MOVEMENT 0x02

/**
 * TRUE when the platform movement of a stage may run. Onett, Tal Tal and Tal Tal Remix treat their moving platform as a
 * hazard and follow the hazard toggle. Every other stage runs it unless its Hazards.type has the MOVEMENT bit and the
 * movement toggle is off.
 *
 * @param gkind Remix stage id.
 * @param hazard_type Stages.stage_hazard_table entry of the stage.
 * @param hazards_on TRUE when the hazard toggle is on.
 * @param movement_on TRUE when the movement toggle is on.
 */
int32_t grRemixPlatformRulesMovementIsOn(int32_t gkind, int32_t hazard_type, int32_t hazards_on, int32_t movement_on);

#endif
