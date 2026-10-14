#ifndef _GRREMIXTURNPIKE_H_
#define _GRREMIXTURNPIKE_H_

/*
 * Toad's Turnpike hazard (Hazards.asm toads_turnpike_setup, toads_turnpike_main): a per-frame
 * light angle cycle written into every fighter's colanim.
 */

#include <ssb_types.h>

#ifdef PORT

void grRemixTurnpikeMakeGround(void);

#endif

#endif
