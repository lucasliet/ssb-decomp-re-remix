#ifndef _SC1PREMIXALLSTARRESTPORTRAITS_H_
#define _SC1PREMIXALLSTARRESTPORTRAITS_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Portraits of the next opponents in the All-Star Rest Area (SinglePlayerModes.asm rest_area_setup, _portrait): translucent pictures
 * of the fighters of the coming match, drawn in the room itself (Render.draw_stage_texture_), not on the screen.
 */

/** Makes the portraits of the coming match; no-op outside the Rest Area. */
extern void sc1PRemixAllstarRestMakePortraits(void);

#endif

#endif
