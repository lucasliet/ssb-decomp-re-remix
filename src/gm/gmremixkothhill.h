#ifndef _GMREMIXKOTHHILL_H_
#define _GMREMIXKOTHHILL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <PR/gbi.h>

/*
 * Hill selection of King of the Hill (KingOfTheHill.asm get_random_line_id_, set_random_hill_,
 * is_hill_outside_blast_zone_): a random floor line with a valid surface, inside the blast zone.
 */

#define GMREMIX_KOTH_HILL_TRIES 3

/**
 * Picks a random floor line of the stage that can be a hill: a surface that does not hurt (valid materials of the asm), at
 * least two vertices, active collision and inside the blast zone.
 *
 * @return The line id, or -1 when this try found none.
 */
extern s32 gmRemixKothHillPickLine(void);

/** TRUE when every vertex of the floor line `line_id` is inside the blast zone of the stage. */
extern sb32 gmRemixKothHillIsInsideBlastZone(s32 line_id);

/** TRUE when the vertices of the floor line `line_id` can be read (its collision is on and it has two vertices). */
extern sb32 gmRemixKothHillLineIsUsable(s32 line_id);

/** Number of vertices of the hill line `line_id` (read only when gmRemixKothHillLineIsUsable). */
extern s32 gmRemixKothHillGetVertexCount(s32 line_id);

/** World position of vertex `vertex` of the hill line `line_id`. */
extern void gmRemixKothHillGetVertex(s32 line_id, s32 vertex, f32 *x, f32 *y);

#endif

#endif
