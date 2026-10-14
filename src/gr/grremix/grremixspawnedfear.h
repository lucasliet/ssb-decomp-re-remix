#ifndef _GRREMIXSPAWNEDFEAR_H_
#define _GRREMIXSPAWNEDFEAR_H_

/*
 * Spawned Fear hazards (.toolchain/pkgs/smashremix-src/src/Hazards.asm spawned_fear_acid_setup_).
 * The setup reads the Cacodemon and Thunderball files from the stage header, creates the acid
 * routine and the Cacodemon factory.
 */

#include <ssb_types.h>

/** spawned_fear_acid_setup_: runs once per battle when the stage's ground maker is called. */
void grRemixSpawnedFearSetup(void);

#endif
