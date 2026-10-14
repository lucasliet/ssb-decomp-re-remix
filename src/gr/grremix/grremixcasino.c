/**
 * grremixcasino.c - Casino Night Zone side bumper bounce, the wall damage hook of Hazards.asm. See ftremixhooks.h.
 */

#ifdef PORT

#include <ft/fighter.h>
#include <mp/mpdef.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "grremixids.h"

extern void *func_800269C0_275C0(u16 id);

#define GRREMIX_CASINO_WALL_BOUNCE_SCALE 0.8F
#define GRREMIX_CASINO_BUMPER_BOUNCE_SCALE 1.1F
/* Hazards.asm 4458: surface ids above 0x16 are the side bumpers */
#define GRREMIX_CASINO_BUMPER_SURFACE_MIN 0x16
#define GRREMIX_CASINO_BUMPER_SFX 0x3D6

/**
 * Surface id of the wall the fighter hit. REMIX-TODO(surface bytes): Hazards.asm reads the bitfield at 0xCE and the
 * surface bytes at 0x11F and 0x133 of the N64 fighter struct; this port reads the MPCollData wall masks instead.
 */
static u32 grRemixCasinoWallSurface(FTStruct *fp)
{
    if (fp->coll_data.mask_curr == MAP_FLAG_LWALL)
    {
        return fp->coll_data.lwall_flags & MAP_VERTEX_MAT_MASK;
    }
    return fp->coll_data.rwall_flags & MAP_VERTEX_MAT_MASK;
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

#endif
