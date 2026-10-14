#ifndef _GRREMIXCACODEMONFACTORY_H_
#define _GRREMIXCACODEMONFACTORY_H_

/*
 * Cacodemon spawner of the Spawned Fear hazard (.toolchain/pkgs/smashremix-src/src/Hazards.asm
 * cacodemon_factory_). It runs as a stage process and spawns one Cacodemon at a time.
 */

/** Creates the factory process; its timer and spawn lock start cleared. */
void grRemixCacodemonFactoryMake(void);

/** Clears the one-at-a-time lock. Called when the Cacodemon is destroyed or leaves the blast zone. */
void grRemixCacodemonFactoryAllowSpawn(void);

#endif
