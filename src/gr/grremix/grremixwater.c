/**
 * grremixwater.c - Water hazard of Great Bay and Twilight City. See grremixwater.h.
 */

#ifdef PORT

#include "grremixwater.h"
#include "grremixhazards.h"
#include <gr/ground.h>
#include <ft/fighter.h>
#include <ef/efmanager.h>
#include "remix/remix_roster_data.h"
#include <port_log.h>

#define GRREMIX_WATER_PORTS_NUM 4
#define GRREMIX_WATER_SPLASH_FGM 0x3D1
#define GRREMIX_WATER_FALL_LIMIT -20.0F
#define GRREMIX_WATER_DRAG_VEL_X 0.9375F
#define GRREMIX_WATER_DRAG_VEL_Y 0.875F
#define GRREMIX_WATER_DRAG_KNOCKBACK 0.98046875F
/* Remix Action.FOX.FireFoxAir (Action.asm:734). The asm uses the same id for Falco. */
#define GRREMIX_WATER_STATUS_FIRE_FOX_AIR 0xE8

extern void* func_800269C0_275C0(u16 id);

static f32 sGRRemixWaterY;
static f32 sGRRemixWaterSplashY;
static sb32 sGRRemixWaterUnderWater[GRREMIX_WATER_PORTS_NUM];

/** Fox, Jigglypuff-Fox and Falco ignore the water while Fire Fox is in the air (asm: _fire_fox_check). */
static sb32 grRemixWaterIgnoresFighter(FTStruct *fp)
{
    sb32 is_fire_fox_user = (fp->fkind == REMIX_ID_FOX) || (fp->fkind == REMIX_ID_JFOX) || (fp->fkind == REMIX_ID_FALCO);

    return is_fire_fox_user && (fp->status_id == GRREMIX_WATER_STATUS_FIRE_FOX_AIR);
}

/** Spawns the splash sparks on both sides and plays its sound. */
static void grRemixWaterMakeSplash(f32 pos_x)
{
    Vec3f splash_pos;

    splash_pos.x = pos_x;
    splash_pos.y = sGRRemixWaterSplashY;
    splash_pos.z = 0.0F;

    efManagerDamageSpawnSparksMakeEffect(&splash_pos, 1);
    efManagerDamageSpawnSparksMakeEffect(&splash_pos, -1);
    func_800269C0_275C0(GRREMIX_WATER_SPLASH_FGM);
    port_log("[remix-stage] water splash x=%.0f y=%.0f\n", splash_pos.x, splash_pos.y);
}

/**
 * Applies the drag of the water to the aerial and knockback velocities.
 * The asm clears the bitfield byte 0x18D with mask 0x07, which also clears
 * is_absorb, absorb_lr and is_goto_attack100 (bits 0-4 in big-endian order); it is
 * ported as written.
 */
static void grRemixWaterApplyPhysics(FTStruct *fp)
{
    f32 vel_y;
    f32 kb_y;

    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;

    fp->physics.vel_air.x *= GRREMIX_WATER_DRAG_VEL_X;
    vel_y = fp->physics.vel_air.y * GRREMIX_WATER_DRAG_VEL_Y;
    if (!(GRREMIX_WATER_FALL_LIMIT <= vel_y))
    {
        fp->physics.vel_air.y = vel_y;
    }

    fp->physics.vel_damage_air.x *= GRREMIX_WATER_DRAG_KNOCKBACK;
    kb_y = fp->physics.vel_damage_air.y * GRREMIX_WATER_DRAG_KNOCKBACK;
    fp->physics.vel_damage_air.y = kb_y;
    if (!(GRREMIX_WATER_FALL_LIMIT <= kb_y))
    {
        fp->physics.vel_damage_air.y = kb_y * GRREMIX_WATER_DRAG_VEL_X;
    }
}

/** Updates the under-water state of one fighter and applies the water while it is under. */
static void grRemixWaterUpdateFighter(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f *pos;

    if (fp->player >= GRREMIX_WATER_PORTS_NUM)
    {
        return;
    }
    if (grRemixWaterIgnoresFighter(fp))
    {
        return;
    }
    if (!grRemixHazardsMatchHasStarted())
    {
        return;
    }

    pos = fp->coll_data.p_translate;
    if (pos->y > sGRRemixWaterY)
    {
        sGRRemixWaterUnderWater[fp->player] = FALSE;
        return;
    }
    if (!sGRRemixWaterUnderWater[fp->player])
    {
        sGRRemixWaterUnderWater[fp->player] = TRUE;
        grRemixWaterMakeSplash(pos->x);
    }
    grRemixWaterApplyPhysics(fp);
}

/**
 * Process of the water object, one call per frame.
 */
static void grRemixWaterProcUpdate(GObj *water_gobj)
{
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

    while (fighter_gobj != NULL)
    {
        grRemixWaterUpdateFighter(fighter_gobj);
        fighter_gobj = fighter_gobj->link_next;
    }
}

/**
 * Creates the water object and clears the under-water flags of every port.
 * asm: Hazards.great_bay_setup and Hazards.twilight_city_setup (both create the same object).
 */
void grRemixWaterMakeGround(f32 water_y, f32 splash_y)
{
    GObj *water_gobj;
    s32 port;

    sGRRemixWaterY = water_y;
    sGRRemixWaterSplashY = splash_y;
    for (port = 0; port < GRREMIX_WATER_PORTS_NUM; port++)
    {
        sGRRemixWaterUnderWater[port] = FALSE;
    }

    water_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(water_gobj, grRemixWaterProcUpdate, nGCProcessKindFunc, 4);
}

#endif
