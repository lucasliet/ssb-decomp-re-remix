#ifndef _GRREMIXMETALLIC_H_
#define _GRREMIXMETALLIC_H_

/*
 * Metallic Madness hazards, ported from Hazards.asm metallic_madness_setup, bee_factory_ and shrink_timer_.
 * The factory spawns a Robot Bee item at random, the Robot Bee laser shrinks the fighters it hits, and the
 * shrink timer restores their size when the duration ends. The state is cleared at every setup.
 */

#include <ssb_types.h>
#include <sys/objdef.h>

/** Builds the bee factory and shrink timer GObjs of Metallic Madness and clears the hazard state. */
void grRemixMetallicSetup(void);

/** Address of the stage header pointer. Item descriptors use it as p_file, as the asm passes 0x801313F4. */
void **grRemixMetallicStageHeaderRef(void);

/** Address of the laser projectile file pointer. Projectile descriptors use it as p_weapon. */
void **grRemixMetallicLaserFileRef(void);

/** Robot Bee facing: 0 moves to the right, any other value moves to the left. */
s32 grRemixMetallicBeeFacing(void);

/** Shrinks the fighter of `port` for the shrink duration; called by the Robot Bee laser when it hits. */
void grRemixMetallicShrinkPlayer(s32 port);

#endif
