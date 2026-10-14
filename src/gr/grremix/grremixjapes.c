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
#define GRREMIX_JAPES_WATER_BASE_X 0.0F /* REMIX-TODO(f20 never set in rushing_water): asm subtracts f20, assumed 0 */
#define GRREMIX_JAPES_SPLASH_FGM 0x3D1 /* asm fgm id Splash */
#define GRREMIX_JAPES_PLAYERS_NUM 4

extern void* func_800269C0_275C0(u16 id);

static sb32 sGRRemixJapesUnderWater[GRREMIX_JAPES_PLAYERS_NUM];

static void grRemixJapesRushingWaterProcUpdate(GObj *water_gobj);
static void grRemixJapesRushingWaterCheckFighter(GObj *fighter_gobj);
static sb32 grRemixJapesCanPlaySplash(void);
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
    /* REMIX-TODO(verify stage header base): asm loads the header from 0x80131300 - 0x14; unverified on the map data layout */
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
 * valid action (status id from Wait on). The position is the fighter DObj translate, as in grJungle.
 * REMIX-TODO(verify): the asm reads the fighter location struct (player + 0x78); unverified equivalence.
 */
static void grRemixJapesRushingWaterCheckFighter(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f *pos = &DObjGetStruct(fighter_gobj)->translate.vec.f;
    sb32 *is_under_water = &sGRRemixJapesUnderWater[fp->player];

    if (pos->y > GRREMIX_JAPES_WATER_Y)
    {
        *is_under_water = FALSE;
        return;
    }
    if (!*is_under_water)
    {
        *is_under_water = TRUE;
        if (grRemixJapesCanPlaySplash())
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

/** The splash plays in training mode, or in a match once the match clock has started (asm intro check). */
static sb32 grRemixJapesCanPlaySplash(void)
{
    if (gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode)
    {
        return TRUE;
    }
    return gSCManagerBattleState->time_passed != 0;
}

/** Leftward push of the current, speed minus a term proportional to the distance from the base. */
static f32 grRemixJapesWaterPull(f32 pos_x)
{
    f32 pull = (pos_x - GRREMIX_JAPES_WATER_BASE_X) * GRREMIX_JAPES_WATER_PULL;

    return GRREMIX_JAPES_WATER_SPEED - pull;
}

#endif
