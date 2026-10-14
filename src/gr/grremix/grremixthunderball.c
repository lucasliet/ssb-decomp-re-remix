/**
 * grremixthunderball.c - C port of the Cacodemon Thunderball, the stage projectile of Spawned Fear
 * (.toolchain/pkgs/smashremix-src/src/Hazards.asm thunderball_*, check_target_).
 *
 * The Thunderball aims at the closest fighter that is not in a KO status, or flies diagonally
 * when there is none. It lives until it hits the map, a hurtbox or the stage ends.
 */

#ifdef PORT

#include <ft/fighter.h>
#include <it/itmain.h>
#include <sys/obj.h>
#include <wp/weapon.h>
#include <wp/wpmain.h>
#include <wp/wpmanager.h>
#include <wp/wpmap.h>
#include <ef/efmanager.h>
#include "gr/grremix/grremixthunderball.h"
#include "it/itremix/itremixcacodemon.h"
#include <math.h>

#define GRREMIX_THUNDERBALL_DURATION 40 /* asm 0x0028: frames before the projectile is spent */
#define GRREMIX_THUNDERBALL_SPEED 80.0F /* asm 0x42A0 */
#define GRREMIX_THUNDERBALL_DEFAULT_VEL -100.0F /* asm 0xC2C8, x and y when there is no target */
#define GRREMIX_THUNDERBALL_TARGET_MIN_STATUS 7 /* asm: statuses below 7 are KO states, never targeted */

static void *sGRRemixThunderballFile;

/** Thunderball.main_: the asm ignores the expiry result, so the projectile stays until a collision. */
static sb32 grRemixThunderballProcUpdate(GObj *weapon_gobj)
{
    wpMainDecLifeCheckExpire(wpGetStruct(weapon_gobj));

    return FALSE;
}

/** Thunderball.collision_ (proc_map): small smoke and destruction on a map collision. */
static sb32 grRemixThunderballProcMap(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);

    if (!wpMapTestAllCheckCollEnd(weapon_gobj))
    {
        return FALSE;
    }
    /* REMIX-TODO(facing): asm passes 1.0 as the second argument of 0x800FF048; the facing is passed here as the Mario fireball does. */
    efManagerDustLightMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f, wp->lr, 1.0F);

    return TRUE;
}

/** Thunderball.hit_ (proc_hit): destroys the projectile on a hurtbox hit. */
static sb32 grRemixThunderballProcHit(GObj *weapon_gobj)
{
    return TRUE;
}

/*
 * Descriptor of the Thunderball. The asm reflector and absorb routines are vanilla Fox and
 * Ness routines (0x80168748, 0x80168964) and are not ported.
 */
WPDesc dWPRemixCacodemonThunderballWeaponDesc =
{
    0x00,                                   /* Render flags */
    GRREMIX_THUNDERBALL_KIND,               /* Weapon kind */
    &sGRRemixThunderballFile,               /* Thunderball file (hitbox and graphics) */
    0,                                      /* Attributes at the file base */
    {
        nGCMatrixKindTra,                   /* Main matrix transformations */
        0,                                  /* Secondary matrix transformations */
        0
    },
    grRemixThunderballProcUpdate,           /* Proc Update */
    grRemixThunderballProcMap,              /* Proc Map */
    grRemixThunderballProcHit,              /* Proc Hit */
    NULL,                                   /* Proc Shield */
    NULL,                                   /* Proc Hop */
    NULL,                                   /* Proc Set-Off */
    NULL,                                   /* REMIX-TODO(reflector): vanilla 0x80168748 */
    NULL                                    /* REMIX-TODO(absorb): vanilla 0x80168964 */
};

void grRemixThunderballSetFile(void *file)
{
    sGRRemixThunderballFile = file;
}

/** check_target_: the fighter becomes the target when it is closer on the x axis than the current target. */
static void grRemixThunderballCheckFighter(ITRemixCacodemonVars *vars, Vec3f *spawn_pos, GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f *target_pos;
    f32 x_diff;

    if (fp->status_id < GRREMIX_THUNDERBALL_TARGET_MIN_STATUS)
    {
        return;
    }
    target_pos = &DObjGetStruct(fighter_gobj)->translate.vec.f;
    x_diff = fabsf(target_pos->x - spawn_pos->x);
    if ((vars->target_gobj != NULL) && (x_diff > vars->target_x_diff))
    {
        return;
    }
    vars->target_gobj = fighter_gobj;
    vars->target_x_diff = x_diff;
}

/** Runs the target check over every fighter; the target is kept between shots as in the asm. */
static void grRemixThunderballFindTarget(ITRemixCacodemonVars *vars, Vec3f *spawn_pos)
{
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

    while (fighter_gobj != NULL)
    {
        grRemixThunderballCheckFighter(vars, spawn_pos, fighter_gobj);
        fighter_gobj = fighter_gobj->link_next;
    }
}

/** Sets the velocity toward the target at SPEED, or the default diagonal when there is none. */
static void grRemixThunderballAim(WPStruct *wp, ITRemixCacodemonVars *vars, Vec3f *spawn_pos)
{
    Vec3f *target_pos;
    f32 angle;

    if (vars->target_gobj == NULL)
    {
        wp->physics.vel_air.x = GRREMIX_THUNDERBALL_DEFAULT_VEL;
        wp->physics.vel_air.y = GRREMIX_THUNDERBALL_DEFAULT_VEL;
        return;
    }
    target_pos = &DObjGetStruct(vars->target_gobj)->translate.vec.f;
    angle = atan2f(target_pos->y - spawn_pos->y, target_pos->x - spawn_pos->x);
    wp->physics.vel_air.x = GRREMIX_THUNDERBALL_SPEED * cosf(angle);
    wp->physics.vel_air.y = GRREMIX_THUNDERBALL_SPEED * sinf(angle);
}

/** Palette index 0 for the Thunderball model (asm sets 0.0 in the model object). */
static void grRemixThunderballSetPalette(GObj *weapon_gobj)
{
    MObj *mobj = DObjGetStruct(weapon_gobj)->mobj;

    if (mobj != NULL)
    {
        mobj->palette_id = 0.0F;
    }
}

/** thunderball_stage_setting_: makes the projectile at (x, y, 0), sets its duration, target and velocity. */
GObj* grRemixThunderballShoot(GObj *cacodemon_gobj)
{
    ITRemixCacodemonVars *vars = ITREMIX_CACODEMON_VARS(itGetStruct(cacodemon_gobj));
    Vec3f spawn_pos = DObjGetStruct(cacodemon_gobj)->translate.vec.f;
    GObj *projectile_gobj;

    spawn_pos.z = 0.0F;
    projectile_gobj = wpManagerMakeWeapon(NULL, &dWPRemixCacodemonThunderballWeaponDesc, &spawn_pos, WEAPON_FLAG_PARENT_GROUND);
    if (projectile_gobj == NULL)
    {
        return NULL;
    }
    wpGetStruct(projectile_gobj)->lifetime = GRREMIX_THUNDERBALL_DURATION;
    grRemixThunderballFindTarget(vars, &spawn_pos);
    grRemixThunderballAim(wpGetStruct(projectile_gobj), vars, &spawn_pos);
    grRemixThunderballSetPalette(projectile_gobj);

    return projectile_gobj;
}

#endif
