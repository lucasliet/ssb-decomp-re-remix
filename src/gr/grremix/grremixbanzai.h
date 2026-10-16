#ifndef _GRREMIXBANZAI_H_
#define _GRREMIXBANZAI_H_

/*
 * Banzai Bill factory of Peach's Castle II, ported from Hazards.asm banzai_bill_factory_ and peach_castle_II_setup_.
 * The factory rolls for a Bullet Bill item every tic once 1500 tics have passed. It only runs in training mode,
 * as the asm does (see grRemixBanzaiFactoryIsActive in grremixbanzai.c).
 */

#include <ssb_types.h>

/**
 * Stores the stage header, clears the factory timer and creates the factory process GObj.
 * `stage_header` is the Peach's Castle II stage header (gMPCollisionGroundData - 0x14).
 */
void grRemixBanzaiFactoryMake(void *stage_header);

#endif
