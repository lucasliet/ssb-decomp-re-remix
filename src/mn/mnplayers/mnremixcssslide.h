#ifndef _MNREMIXCSSSLIDE_H_
#define _MNREMIXCSSSLIDE_H_

#include <ssb_types.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Slide-in of the portraits of the Remix VS character select (src/CharacterSelect.asm draw_portraits_): the 30 grid portraits
 * and the two bookends start off screen and slide to their cell, see mnremixcsssliderules.h.
 */

/**
 * Puts a portrait off screen and adds the process that slides it in. The portrait keeps its slot in user_data.s.
 *
 * @param gobj Portrait GObj with its portrait sprite as first SObj.
 * @param slot Portrait number: 0..29 grid slots, 30 bonus bookend, 31 Random bookend.
 */
extern void mnRemixCssSlideAttach(GObj *gobj, s32 slot);

#endif

#endif
