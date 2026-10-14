#ifndef _GRREMIXRAINBOW_H_
#define _GRREMIXRAINBOW_H_

#include <ssb_types.h>
#include <sys/obj.h>

/** Rainbow Road hazards (Hazards.asm rainbow_road_setup): Chain Chomp and stage sounds. */
void grRemixRainbowSetup(void);

/** Chain Chomp ground GObj of the current match, NULL when the hazards are off. */
GObj *grRemixRainbowGetChompGObj(void);

#endif
