#ifndef _ITREMIX_CAR_H_
#define _ITREMIX_CAR_H_

/*
 * Smash Remix Car (item id 0x3C), ported from src/items/Car.asm and the Onett hazard in
 * src/Hazards.asm (onett_setup, car_main_, car_stage_setting_, car_blast_zone_).
 * The descriptor is dITRemixCarItemDesc in itremixcar.c.
 */

#include "it/itremix/itremix.h"

/**
 * Hazards.onett_setup: resets the car timer and spawns the car from the Onett map header.
 * stage_header is the Onett header address (the asm's ground data pointer minus 0x14).
 */
void itRemixCarHazardSetup(void *stage_header);

/** SPAWN_ITEM of Car: builds the car with its hitbox and scale. The vanilla ABI is (parent, pos, vel, flags). */
GObj* itRemixCarMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
