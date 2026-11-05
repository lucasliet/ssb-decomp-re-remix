/**
 * grremixcasino.c - Casino Night Zone side bumper bounce, the wall damage hook of Hazards.asm. See ftremixhooks.h.
 */

#ifdef PORT

#include <stdlib.h>
#include <ft/fighter.h>
#include <mp/map.h>
#include <mp/mpdef.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "grremixids.h"
#include "grremixcasino.h"
#include "grremixsurfacerules.h"

extern void *func_800269C0_275C0(u16 id);

#define GRREMIX_CASINO_WALL_BOUNCE_SCALE 0.8F
#define GRREMIX_CASINO_BUMPER_BOUNCE_SCALE 1.1F
/* Hazards.asm 4458: surface ids above 0x16 are the side bumpers */
#define GRREMIX_CASINO_BUMPER_SURFACE_MIN 0x16
#define GRREMIX_CASINO_SURFACE_BYTE_MASK 0xFF
#define GRREMIX_CASINO_BUMPER_SFX 0x3D6
#define GRREMIX_CASINO_LOG_ENV "SSB64_REMIX_CASINO_LOG"

/**
 * Surface id of the wall the fighter hit. Hazards.asm 4450-4455 reads the surface type byte, the low byte of the wall
 * flags word (N64 offsets 0x11F and 0x133), selected by the collision mask at 0xCE (== left wall).
 */
static u32 grRemixCasinoWallSurface(FTStruct *fp)
{
    if (fp->coll_data.mask_curr == MAP_FLAG_LWALL)
    {
        return fp->coll_data.lwall_flags & GRREMIX_CASINO_SURFACE_BYTE_MASK;
    }
    return fp->coll_data.rwall_flags & GRREMIX_CASINO_SURFACE_BYTE_MASK;
}

/**
 * Velocity scale that ftCommonWallDamageSetStatus applies on a wall hit. Side bumpers of Casino Night Zone bounce
 * harder and play the bumper sound (Hazards.asm 4436-4471).
 */
f32 grRemixCasinoWallBounceScale(FTStruct *fp)
{
    if (gSCManagerBattleState->gkind != nGRRemixStage_CASINO)
    {
        return GRREMIX_CASINO_WALL_BOUNCE_SCALE;
    }
    if (grRemixCasinoWallSurface(fp) <= GRREMIX_CASINO_BUMPER_SURFACE_MIN)
    {
        return GRREMIX_CASINO_WALL_BOUNCE_SCALE;
    }
    func_800269C0_275C0(GRREMIX_CASINO_BUMPER_SFX);
    return GRREMIX_CASINO_BUMPER_BOUNCE_SCALE;
}

/**
 * Wall surface the knockback hook reads (Hazards.asm 4478-4531): the previous and current collision masks must both be
 * exactly the left wall (word 0x00010001 at N64 0xCC) or the right wall (0x00200020).
 */
static sb32 grRemixCasinoGetBumperWallSurface(FTStruct *fp, sb32 *is_left_wall, u32 *surface)
{
    if ((fp->coll_data.mask_prev == MAP_FLAG_LWALL) && (fp->coll_data.mask_curr == MAP_FLAG_LWALL))
    {
        *is_left_wall = TRUE;
        *surface = fp->coll_data.lwall_flags & GRREMIX_CASINO_SURFACE_BYTE_MASK;
        return TRUE;
    }
    if ((fp->coll_data.mask_prev == MAP_FLAG_RWALL) && (fp->coll_data.mask_curr == MAP_FLAG_RWALL))
    {
        *is_left_wall = FALSE;
        *surface = fp->coll_data.rwall_flags & GRREMIX_CASINO_SURFACE_BYTE_MASK;
        return TRUE;
    }
    return FALSE;
}

sb32 grRemixCasinoGetBumperMaterial(FTStruct *fp, u32 *material)
{
    sb32 is_left_wall;
    u32 wall_surface;
    s32 bumper_material;

    if (gSCManagerBattleState->gkind != nGRRemixStage_CASINO)
    {
        return FALSE;
    }
    if (grRemixSurfaceRulesBumperStatusApplies(fp->status_id) == 0)
    {
        return FALSE;
    }
    if (grRemixCasinoGetBumperWallSurface(fp, &is_left_wall, &wall_surface) == FALSE)
    {
        return FALSE;
    }
    bumper_material = grRemixSurfaceRulesBumperMaterial(wall_surface, is_left_wall, fp->lr);

    if (bumper_material < 0)
    {
        return FALSE;
    }
    *material = bumper_material;

    return TRUE;
}

/** TRUE when SSB64_REMIX_CASINO_LOG is set to something other than empty or "0". */
static sb32 grRemixCasinoLogIsEnabled(void)
{
    const char *text = getenv(GRREMIX_CASINO_LOG_ENV);

    return ((text != NULL) && (text[0] != '\0') && (text[0] != '0')) ? TRUE : FALSE;
}

/** Logs the segment that starts at `vertex_index` of the wall line when its start vertex carries a bumper surface. */
static void grRemixCasinoLogBumperSegment(s32 line_id, const char *side, s32 vertex_index)
{
    MPVertexData *start = &gMPCollisionVertexData->vpos[gMPCollisionVertexIDs->vertex_id[vertex_index]];
    MPVertexData *end = &gMPCollisionVertexData->vpos[gMPCollisionVertexIDs->vertex_id[vertex_index + 1]];
    u32 surface = start->vertex_flags & GRREMIX_CASINO_SURFACE_BYTE_MASK;

    if (surface <= GRREMIX_CASINO_BUMPER_SURFACE_MIN)
    {
        return;
    }
    port_log("[remix-casino] bumper wall line=%d side=%s surface=0x%02X from=(%d,%d) to=(%d,%d)\n", line_id, side, surface,
             (s32)start->pos.x, (s32)start->pos.y, (s32)end->pos.x, (s32)end->pos.y);
}

/** Logs every segment of the wall lines of one kind (nMPLineKindLWall or nMPLineKindRWall) that carries a bumper surface. */
static void grRemixCasinoLogBumperLines(s32 line_kind, const char *side)
{
    MPLineGroup *group = &gMPCollisionLineGroups[line_kind];
    s32 i;
    s32 vertex_index;
    s32 line_id;

    for (i = 0; i < group->line_count; i++)
    {
        line_id = group->line_id[i];

        for (vertex_index = gMPCollisionVertexLinks[line_id].vertex1; vertex_index < (gMPCollisionVertexLinks[line_id].vertex1 + gMPCollisionVertexLinks[line_id].vertex2 - 1); vertex_index++)
        {
            grRemixCasinoLogBumperSegment(line_id, side, vertex_index);
        }
    }
}

void grRemixCasinoLogBumperWalls(void)
{
    if (gSCManagerBattleState->gkind != nGRRemixStage_CASINO)
    {
        return;
    }
    if (grRemixCasinoLogIsEnabled() == FALSE)
    {
        return;
    }
    port_log("[remix-casino] wall lines: left=%d right=%d (surface above 0x%02X are bumpers)\n", gMPCollisionLineGroups[nMPLineKindLWall].line_count,
             gMPCollisionLineGroups[nMPLineKindRWall].line_count, GRREMIX_CASINO_BUMPER_SURFACE_MIN);
    grRemixCasinoLogBumperLines(nMPLineKindLWall, "left");
    grRemixCasinoLogBumperLines(nMPLineKindRWall, "right");
}

#endif
