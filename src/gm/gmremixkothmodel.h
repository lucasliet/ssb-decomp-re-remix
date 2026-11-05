#ifndef _GMREMIXKOTHMODEL_H_
#define _GMREMIXKOTHMODEL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * The hill model of Remix King of the Hill (KingOfTheHill.asm create_ground_gfx_, koth_ground_gfx_struct and
 * koth_indicator_gfx_struct): the Remix file 0x14F3 holds one joint tree per line segment (the strip slab with its end caps) and the
 * hill marker (a crown over an arrow). They are effect objects made from the file with the vanilla effect machinery; the strip
 * pieces are placed on the vertices of the hill line and the marker above the middle of the line.
 */

/**
 * Loads the model file and makes the effect descriptors ready. Call once at match start.
 *
 * @return TRUE when the model can be drawn, FALSE when the file is missing (the caller falls back to the flat strip).
 */
extern sb32 gmRemixKothModelMake(void);

/** Every frame: moves the pieces to the vertices of the hill line and the marker above its middle, or hides them. */
extern void gmRemixKothModelSync(void);

/**
 * KingOfTheHill.asm ai_hill_behavior_: world position of the hill marker (the top joint plus its texture joint), raised above
 * the hill line.
 *
 * @return FALSE when the marker does not exist or is hidden, then x and y are not written.
 */
extern sb32 gmRemixKothModelGetMarkerPosition(f32 *x, f32 *y);

#endif

#endif
