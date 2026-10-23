/**
 * gmremixkothhill.c - hill selection of Remix King of the Hill (KingOfTheHill.asm get_random_line_id_, set_random_hill_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mp/map.h>
#include <mp/mpdef.h>
#include <mp/mptypes.h>
#include <mp/mpcollision.h>
#include <sys/utils.h>
#include "gm/gmremixkothhill.h"

#define GMREMIX_KOTH_MATERIAL_MASK 0x00FF
#define GMREMIX_KOTH_MIN_VERTICES 2

/* KingOfTheHill.asm get_random_line_id_: surfaces 0x00-0x06, 0x0C-0x0E, 0x11 and 0x1F-0x23 are valid, the damaging and
 * special ones (0x07-0x0B, 0x0F-0x10, 0x12-0x1E) are not. */
static sb32 gmRemixKothHillMaterialIsValid(u16 vertex_flags)
{
    s32 material = vertex_flags & GMREMIX_KOTH_MATERIAL_MASK;

    if (material <= 0x06)
    {
        return TRUE;
    }
    if ((material >= 0x0C) && (material <= 0x0E))
    {
        return TRUE;
    }
    if (material == 0x11)
    {
        return TRUE;
    }
    return ((material >= 0x1F) && (material <= 0x23)) ? TRUE : FALSE;
}

static sb32 gmRemixKothHillCollisionIsOn(s32 line_id)
{
    DObj *yakumono_dobj = gMPCollisionYakumonoDObjs->dobjs[gMPCollisionVertexInfo->vertex_info[line_id].yakumono_id];

    return (yakumono_dobj->user_data.s < nMPYakumonoStatusOff) ? TRUE : FALSE;
}

sb32 gmRemixKothHillIsInsideBlastZone(s32 line_id)
{
    s32 vertex;
    s32 count = mpCollisionGetVertexCountLineID(line_id);

    for (vertex = 0; vertex < count; vertex++)
    {
        Vec3f pos;

        mpCollisionGetVertexPositionID(line_id, vertex, &pos);

        if ((pos.x < gMPCollisionGroundData->map_bound_left) || (pos.x > gMPCollisionGroundData->map_bound_right))
        {
            return FALSE;
        }
        if ((pos.y < gMPCollisionGroundData->map_bound_bottom) || (pos.y > gMPCollisionGroundData->map_bound_top))
        {
            return FALSE;
        }
    }
    return TRUE;
}

sb32 gmRemixKothHillLineIsUsable(s32 line_id)
{
    if ((line_id < 0) || (line_id >= gMPCollisionLinesNum))
    {
        return FALSE;
    }
    if (gmRemixKothHillCollisionIsOn(line_id) == FALSE)
    {
        return FALSE;
    }
    return (mpCollisionGetVertexCountLineID(line_id) >= GMREMIX_KOTH_MIN_VERTICES) ? TRUE : FALSE;
}

s32 gmRemixKothHillPickLine(void)
{
    MPLineGroup *floor_lines = &gMPCollisionLineGroups[nMPLineKindFloor];
    s32 line_id;

    if (floor_lines->line_count == 0)
    {
        return -1;
    }
    line_id = floor_lines->line_id[syUtilsRandIntRange(floor_lines->line_count)];

    if (gmRemixKothHillLineIsUsable(line_id) == FALSE)
    {
        return -1;
    }
    if (gmRemixKothHillMaterialIsValid(mpCollisionGetVertexFlagsLineID(line_id)) == FALSE)
    {
        return -1;
    }
    return (gmRemixKothHillIsInsideBlastZone(line_id) != FALSE) ? line_id : -1;
}

s32 gmRemixKothHillGetVertexCount(s32 line_id)
{
    return mpCollisionGetVertexCountLineID(line_id);
}

void gmRemixKothHillGetVertex(s32 line_id, s32 vertex, f32 *x, f32 *y)
{
    Vec3f pos;

    mpCollisionGetVertexPositionID(line_id, vertex, &pos);
    *x = pos.x;
    *y = pos.y;
}

#endif
