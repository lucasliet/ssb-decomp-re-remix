/**
 * grremixcorneria.c - Corneria Remix hazards: Great Fox laser volley. See grremixcorneria.h.
 */

#ifdef PORT

#include "grremixcorneria.h"
#include "grremixhazards.h"
#include <gr/ground.h>
#include <gr/grcommon/grsector.h>
#include <wp/weapon.h>
#include <sys/utils.h>

extern void *func_800269C0_275C0(u16 id);
extern WPDesc dGRSectorArwingWeaponLaser2DWeaponDesc;

#define GREAT_FOX_FGM_ID 0x0109
#define GREAT_FOX_RESTART_TIMER 0x107D

typedef struct RemixCorneriaState
{
    u32 great_fox_timer;

} RemixCorneriaState;

static RemixCorneriaState sCorneriaState;

static const u32 sGreatFoxShotTimers[] =
{
    0x010E, 0x097E, 0x0987, 0x0990, 0x0999, 0x09A2, 0x09AB, 0x09B4, 0x09BD, 0x09C6
};

static Vec3f sGreatFoxLeftSpawnPos = { -3100.0F, -230.0F, 200.0F };
static Vec3f sGreatFoxRightSpawnPos = { -3100.0F, -230.0F, -100.0F };

/* Hazards.asm great_fox_spawn_rotation: 0xBFC90FDB, 0xBF978C23, 0x3FC90FDB */
static const Vec3f sGreatFoxSpawnRotation = { -1.57079637F, -1.18396413F, 1.57079637F };

/**
 * Returns TRUE when the Great Fox fires on the given timer value.
 * @param timer Frames since the stage setup.
 */
static sb32 grRemixCorneriaIsGreatFoxShotTimer(u32 timer)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sGreatFoxShotTimers); i++)
    {
        if (timer == sGreatFoxShotTimers[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Spawns one Arwing-type laser from the Great Fox.
 * The asm passes the Great Fox object as parent for the right shot; the left shot passes an undefined
 * register (see REMIX-TODO below), so both use the Great Fox object.
 * REMIX-TODO(Remix header attribute 0x130): the asm reads the hitbox and graphic attributes at header offset 0x130,
 * a Remix addition to the Sector Z header. This port uses the vanilla Arwing laser descriptor, which points at
 * llGRSectorMapArwingLaser2DWeaponAttributes.
 * @return The spawned laser, or NULL when the weapon manager refused it.
 */
static GObj* grRemixCorneriaGreatFoxSpawnShot(GObj *great_fox_gobj, Vec3f *spawn_pos)
{
    GObj *shot_gobj = wpManagerMakeWeapon(great_fox_gobj, &dGRSectorArwingWeaponLaser2DWeaponDesc, spawn_pos, WEAPON_FLAG_PARENT_GROUND);
    WPStruct *wp;

    if (shot_gobj == NULL)
    {
        return NULL;
    }

    wp = wpGetStruct(shot_gobj);
    wp->physics.vel_air.x = -230.0F;
    DObjGetStruct(shot_gobj)->rotate.vec.f = sGreatFoxSpawnRotation;

    return shot_gobj;
}

/**
 * Plays the laser sound and spawns the left and right lasers. The right laser only spawns when the left one did.
 * @param great_fox_gobj The Great Fox process object.
 */
static void grRemixCorneriaGreatFoxFire(GObj *great_fox_gobj)
{
    func_800269C0_275C0(GREAT_FOX_FGM_ID);

    if (grRemixCorneriaGreatFoxSpawnShot(great_fox_gobj, &sGreatFoxLeftSpawnPos) == NULL)
    {
        return;
    }
    grRemixCorneriaGreatFoxSpawnShot(great_fox_gobj, &sGreatFoxRightSpawnPos);
}

/**
 * Per-frame process of the Great Fox (great_fox_): advances the volley timer and fires on the shot timers.
 * @param great_fox_gobj The Great Fox process object.
 */
static void grRemixCorneriaGreatFoxProcUpdate(GObj *great_fox_gobj)
{
    u32 timer = sCorneriaState.great_fox_timer;

    sCorneriaState.great_fox_timer = timer + 1;

    if (timer == GREAT_FOX_RESTART_TIMER)
    {
        sCorneriaState.great_fox_timer = 0;
        return;
    }
    if (grRemixCorneriaIsGreatFoxShotTimer(timer))
    {
        grRemixCorneriaGreatFoxFire(great_fox_gobj);
    }
}

/**
 * Builds the Corneria hazards (corneria_setup): the Sector Z ground and the Great Fox process.
 * Does nothing when the entry hazard toggle is off, as the asm skips the original routine in that case.
 */
void grRemixCorneriaSetup(void)
{
    GObj *great_fox_gobj;

    if (!grRemixHazardsAreOn())
    {
        return;
    }

    grSectorMakeGround();

    great_fox_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(great_fox_gobj, grRemixCorneriaGreatFoxProcUpdate, nGCProcessKindFunc, 4);

    sCorneriaState.great_fox_timer = 0;
}

#endif
