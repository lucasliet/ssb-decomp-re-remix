/**
 * grremixjapes.c - C port of the Jungle Japes Remix hazards
 * (.toolchain/pkgs/smashremix-src/src/Hazards.asm, jungle_japes_setup and rushing_water).
 */

#ifdef PORT

#include "grremixjapes.h"
#include "grremixhazards.h"
#include "gr/ground.h"
#include "ft/fighter.h"
#include "it/itremix/itremixklaptrap.h"
#include "it/itmanager.h"
#include "mp/map.h"
#include "sc/sctypes.h"
#include "sc/scmanager.h"
#include "sys/obj.h"
#include "sys/objman.h"

#define GRREMIX_JAPES_STAGE_HEADER_OFFSET 0x14 /* asm addiu t1, t1, -0x14: stage header is the map data minus 0x14 */
#define GRREMIX_JAPES_WATER_Y -1000.0F /* asm 0xC47A0000 */
#define GRREMIX_JAPES_WATER_SPEED 90.0F /* asm 0x42B40000 */
#define GRREMIX_JAPES_WATER_PULL 0.0006F /* asm 0x3A1D4952, same multiplier as Whispy */
/*
 * Base x of the pull (asm sub.s f2, f0, f20, Hazards.asm 1950). f20 is never written in rushing_water (1870..1955, the
 * only f20 writes in Hazards.asm are 2628, 3150 and a restore at 4358, all in other routines) and the routine does not
 * save it, so the value is whatever the caller chain left in the callee-saved register: gcRunAll -> gcRunGObjProcess
 * (objman.c, no float code, so neither saves or loads f20) -> rushing_water, running on the scene thread. That is the
 * f20 the thread had at its first FPU use, a residue of an earlier thread that no source defines. The Remix author copied
 * the routine from the Dream Land wind (0x8010595C), where f20 is the wind center x set up by the same function. The
 * port therefore uses 0.0F (center at x = 0) as an explicit choice, not as a proven value: it is not portable exactly.
 */
#define GRREMIX_JAPES_WATER_BASE_X 0.0F
#define GRREMIX_JAPES_SPLASH_FGM 0x3D1 /* asm fgm id Splash */
#define GRREMIX_JAPES_PLAYERS_NUM 4

extern void* func_800269C0_275C0(u16 id);

static sb32 sGRRemixJapesUnderWater[GRREMIX_JAPES_PLAYERS_NUM];

static void grRemixJapesRushingWaterProcUpdate(GObj *water_gobj);
static void grRemixJapesRushingWaterCheckFighter(GObj *fighter_gobj);
static f32 grRemixJapesWaterPull(f32 pos_x);

/**
 * Stage setup (asm jungle_japes_setup): binds the stage file to the KlapTrap item, creates the
 * rushing water process and spawns the KlapTrap at the origin. The asm's Remix timer at map
 * data + 0x08 is replaced by the item's own timer.
 */
void grRemixJapesMakeGround(void)
{
    GObj *water_gobj;
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };

    if (!grRemixHazardsAreOn())
    {
        return;
    }

    water_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(water_gobj, grRemixJapesRushingWaterProcUpdate, nGCProcessKindFunc, 4);

    itRemixKlapTrapSetStageFile((void*) ((uintptr_t) gMPCollisionGroundData - GRREMIX_JAPES_STAGE_HEADER_OFFSET));
    /* asm passes the object id 0x3F5 as the parent of the KlapTrap; it has no owner, so NULL is passed */
    itManagerMakeItemSetupCommon(NULL, ITREMIX_KLAPTRAP_ID, &pos, &vel, TRUE);
}

/** Rushing water process (asm rushing_water): checks every fighter in the fighter link. */
static void grRemixJapesRushingWaterProcUpdate(GObj *water_gobj)
{
    GObj *fighter_gobj;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        grRemixJapesRushingWaterCheckFighter(fighter_gobj);
    }
}

/**
 * Splash on first entry below the water line, then pulls the fighter leftward while it is in a
 * valid action (status id from Wait on). The position is FTStruct coll_data.p_translate (player + 0x78), the Vec3f
 * that Hazards.asm rushing_water reads and writes.
 */
static void grRemixJapesRushingWaterCheckFighter(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f *pos = fp->coll_data.p_translate;
    sb32 *is_under_water = &sGRRemixJapesUnderWater[fp->player];

    if (pos == NULL)
    {
        return;
    }
    if (pos->y > GRREMIX_JAPES_WATER_Y)
    {
        *is_under_water = FALSE;
        return;
    }
    if (!*is_under_water)
    {
        *is_under_water = TRUE;
        if (grRemixHazardsMatchHasStarted())
        {
            func_800269C0_275C0(GRREMIX_JAPES_SPLASH_FGM);
        }
    }
    if (!grRemixMovementIsOn())
    {
        return;
    }
    if (fp->status_id < nFTCommonStatusWait)
    {
        return;
    }
    pos->x -= grRemixJapesWaterPull(pos->x);
}

/** Leftward push of the current, speed minus a term proportional to the distance from the base. */
static f32 grRemixJapesWaterPull(f32 pos_x)
{
    f32 pull = (pos_x - GRREMIX_JAPES_WATER_BASE_X) * GRREMIX_JAPES_WATER_PULL;

    return GRREMIX_JAPES_WATER_SPEED - pull;
}

#endif
