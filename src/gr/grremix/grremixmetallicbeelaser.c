/**
 * grremixmetallicbeelaser.c - Robot Bee laser projectile of Metallic Madness.
 * Ported from Hazards.asm robot_bee_laser_*, using wpmarioremixfireball.c as the Remix weapon template.
 */

#ifdef PORT

#include "grremixmetallicbeelaser.h"
#include "grremixmetallic.h"
#include "ft/fighter.h"
#include "wp/weapon.h"
#include "wp/wpmanager.h"
#include "wp/wpmap.h"
#include "wp/wpvars.h"
#include "wp/wpfox/wpfoxblaster.h"
#include "ef/efmanager.h"
#include <math.h>

#define GRREMIX_METALLIC_LASER_KIND 0x29              /* Projectile.id.METALLICMADNESS_BEELASER (Projectile.asm) */
#define GRREMIX_METALLIC_LASER_SPAWN_FLAGS 1U         /* asm a3 = 1 (hit all players); not a WEAPON_FLAG_* name */
#define GRREMIX_METALLIC_LASER_SPAWN_OFFSET_X 194.0F  /* asm x_spawn: +194 when facing right, -194 when facing left */
#define GRREMIX_METALLIC_LASER_SPAWN_OFFSET_Y -336.0F /* asm 0xC3A80000 */
#define GRREMIX_METALLIC_LASER_SPEED_X 100.0F         /* asm 0x42C8 / 0xC2C8 */
#define GRREMIX_METALLIC_LASER_SPEED_Y -100.0F        /* asm 0xC2C8 */
#define GRREMIX_METALLIC_LASER_SCALE_STEP 3.0F        /* asm 0x4040 (3.0F) */
#define GRREMIX_METALLIC_LASER_SCALE_MIN (-WPBLASTER_CLAMP_SCALE_X) /* asm 0xC2555555, the same value as 160.0F / 3.0F */
/* DEVIATION(vanilla RAM 0x8019CAA0): asm robot_bee_laser_main_ loads the maximum X scale from this vanilla-overlay float; it lies below the Remix RAM image (0x80400000) so it cannot be read; the Fox blaster clamp is assumed. */
#define GRREMIX_METALLIC_LASER_SCALE_MAX WPBLASTER_CLAMP_SCALE_X
#define GRREMIX_METALLIC_PLAYER_OBJECT_ID nGCCommonKindFighter /* asm: object type 0x3E8 of the victim */

static sb32 grRemixMetallicLaserProcUpdate(GObj *laser_gobj);
static sb32 grRemixMetallicLaserProcMap(GObj *laser_gobj);
static sb32 grRemixMetallicLaserProcHit(GObj *laser_gobj);
static sb32 grRemixMetallicLaserProcReflector(GObj *laser_gobj);
static GObj *grRemixMetallicLaserMake(GObj *bee_gobj, Vec3f *spawn_pos);
static void grRemixMetallicLaserSetSpeed(GObj *laser_gobj);
static void grRemixMetallicLaserSetAngle(GObj *laser_gobj);
static void grRemixMetallicLaserGrowScale(GObj *laser_gobj);
static void grRemixMetallicLaserShrinkScale(GObj *laser_gobj);
static void grRemixMetallicLaserHitVictim(GObj *victim_gobj);
static sb32 grRemixMetallicLaserIsImmune(FTStruct *fp);
static f32 grRemixMetallicLaserSpawnOffsetX(void);
static f32 grRemixMetallicLaserSpeedX(void);

/**
 * Weapon descriptor of the laser (asm robot_bee_laser_projectile_struct). p_weapon is set per spawn to the laser
 * file pointer.
 * The transform word of the asm projectile struct is 0x1C000000 (Pikachu thunder value). Its top byte is the
 * XObjTransformKind, 28 = nGCMatrixKindTraRotRpyRSca (objdef.h). The Sca kind is needed because the laser grows the
 * DObj X scale.
 */
static const WPDesc sWPMetallicBeeLaserWeaponDesc =
{
    0x00,                                       /* render flags, asm word 0x00 */
    GRREMIX_METALLIC_LASER_KIND,                /* Projectile.id.METALLICMADNESS_BEELASER */
    NULL,                                       /* set per spawn from grRemixMetallicLaserFileRef() */
    0,                                          /* asm hitbox offset 0 */
    { nGCMatrixKindTraRotRpyRSca, 0, 0 },
    grRemixMetallicLaserProcUpdate,
    grRemixMetallicLaserProcMap,
    grRemixMetallicLaserProcHit,
    NULL,                                       /* shield: asm 0 */
    NULL,                                       /* hop (edge of shield): asm 0 */
    NULL,                                       /* setoff (clang): asm 0 */
    grRemixMetallicLaserProcReflector,
    wpFoxBlasterProcHit                         /* absorb (psi magnet): vanilla 0x80168964, Hazards.asm robot_bee_laser_projectile_struct */
};

/**
 * robot_bee_laser_main_ (asm). Grows the laser toward the maximum X scale when the bee faces right and shrinks
 * it toward the minimum when the bee faces left. The laser lives until it hits a wall or a fighter.
 */
static sb32 grRemixMetallicLaserProcUpdate(GObj *laser_gobj)
{
    if (grRemixMetallicBeeFacing() == 0)
    {
        grRemixMetallicLaserGrowScale(laser_gobj);
    }
    else
    {
        grRemixMetallicLaserShrinkScale(laser_gobj);
    }
    return FALSE;
}

/** Grows the laser's X scale by one step, clamped to the maximum. */
static void grRemixMetallicLaserGrowScale(GObj *laser_gobj)
{
    f32 *scale_x = &DObjGetStruct(laser_gobj)->scale.vec.f.x;

    if (!(*scale_x < GRREMIX_METALLIC_LASER_SCALE_MAX))
    {
        return;
    }
    *scale_x += GRREMIX_METALLIC_LASER_SCALE_STEP;
    if (*scale_x > GRREMIX_METALLIC_LASER_SCALE_MAX)
    {
        *scale_x = GRREMIX_METALLIC_LASER_SCALE_MAX;
    }
}

/** Shrinks the laser's X scale by one step, clamped to the minimum. */
static void grRemixMetallicLaserShrinkScale(GObj *laser_gobj)
{
    f32 *scale_x = &DObjGetStruct(laser_gobj)->scale.vec.f.x;

    if (!(GRREMIX_METALLIC_LASER_SCALE_MIN < *scale_x))
    {
        return;
    }
    *scale_x -= GRREMIX_METALLIC_LASER_SCALE_STEP;
    if (*scale_x < GRREMIX_METALLIC_LASER_SCALE_MIN)
    {
        *scale_x = GRREMIX_METALLIC_LASER_SCALE_MIN;
    }
}

/**
 * robot_bee_laser_collision_ (asm). A map collision explodes the laser, as the vanilla laser does, and
 * destroys it.
 */
static sb32 grRemixMetallicLaserProcMap(GObj *laser_gobj)
{
    if (!wpMapTestAllCheckCollEnd(laser_gobj))
    {
        return FALSE;
    }
    efManagerFoxBlasterGlowMakeEffect(&DObjGetStruct(laser_gobj)->translate.vec.f);
    return TRUE;
}

/**
 * robot_bee_laser_hit_ (asm). Shrinks every fighter in the attack record list, skipping fighters that
 * reflect or absorb. The laser survives the hit.
 */
static sb32 grRemixMetallicLaserProcHit(GObj *laser_gobj)
{
    WPStruct *wp = wpGetStruct(laser_gobj);
    s32 i;

    for (i = 0; i < ARRAY_COUNT(wp->attack_coll.attack_records); i++)
    {
        GObj *victim_gobj = wp->attack_coll.attack_records[i].victim_gobj;

        if (victim_gobj == NULL)
        {
            break;
        }
        grRemixMetallicLaserHitVictim(victim_gobj);
    }
    return FALSE;
}

/** Shrinks one victim when it is a fighter that the laser can affect. */
static void grRemixMetallicLaserHitVictim(GObj *victim_gobj)
{
    FTStruct *fp;

    if (victim_gobj->id != GRREMIX_METALLIC_PLAYER_OBJECT_ID)
    {
        return;
    }
    fp = ftGetStruct(victim_gobj);
    if (grRemixMetallicLaserIsImmune(fp))
    {
        return;
    }
    grRemixMetallicShrinkPlayer(fp->player);
}

/**
 * The asm compares the whole FTStruct word at 0x18C with the reflect (0x04000010) and psi magnet (0x08800110)
 * patterns. REMIX-TODO(FTStruct): the port checks the reflect and absorb bitfields instead of the whole word.
 */
static sb32 grRemixMetallicLaserIsImmune(FTStruct *fp)
{
    return (fp->is_reflect != FALSE) || (fp->is_absorb != FALSE);
}

/**
 * robot_bee_laser_reflect_ (asm). A reflected laser flips its speed and turns to match it.
 */
static sb32 grRemixMetallicLaserProcReflector(GObj *laser_gobj)
{
    WPStruct *wp = wpGetStruct(laser_gobj);

    wp->physics.vel_air.x = -wp->physics.vel_air.x;
    wp->physics.vel_air.y = -wp->physics.vel_air.y;
    grRemixMetallicLaserSetAngle(laser_gobj);
    return FALSE;
}

/**
 * robot_bee_laser_ (asm). Spawns the laser from the bee's translation, offset to the side it faces and below it.
 * Returns the laser GObj, or NULL when the projectile could not be made.
 */
GObj *grRemixMetallicBeeLaserFire(GObj *bee_gobj)
{
    Vec3f spawn_pos;
    DObj *bee_joint = DObjGetStruct(bee_gobj);

    if (*grRemixMetallicLaserFileRef() == NULL)
    {
        return NULL;
    }
    spawn_pos.x = bee_joint->translate.vec.f.x + grRemixMetallicLaserSpawnOffsetX();
    spawn_pos.y = bee_joint->translate.vec.f.y + GRREMIX_METALLIC_LASER_SPAWN_OFFSET_Y;
    spawn_pos.z = 0.0F;
    return grRemixMetallicLaserMake(bee_gobj, &spawn_pos);
}

/** Spawn offset on X: +194 when the bee faces right (facing 0), -194 when it faces left. */
static f32 grRemixMetallicLaserSpawnOffsetX(void)
{
    if (grRemixMetallicBeeFacing() == 0)
    {
        return GRREMIX_METALLIC_LASER_SPAWN_OFFSET_X;
    }
    return -GRREMIX_METALLIC_LASER_SPAWN_OFFSET_X;
}

/** Speed on X: +100 when the bee faces right, -100 when it faces left. */
static f32 grRemixMetallicLaserSpeedX(void)
{
    if (grRemixMetallicBeeFacing() == 0)
    {
        return GRREMIX_METALLIC_LASER_SPEED_X;
    }
    return -GRREMIX_METALLIC_LASER_SPEED_X;
}

/**
 * robot_bee_laser_stage_setting_ (asm). Makes the projectile with the bee as parent, then sets its speed and
 * angle and plays the glow effect at the spawn point.
 */
static GObj *grRemixMetallicLaserMake(GObj *bee_gobj, Vec3f *spawn_pos)
{
    WPDesc weapon_desc = sWPMetallicBeeLaserWeaponDesc;
    GObj *laser_gobj;

    /* asm writes a 40 tic value to WPStruct 0x268; the Remix laser main never reads it, so it is not stored. */
    weapon_desc.p_weapon = grRemixMetallicLaserFileRef();
    laser_gobj = wpManagerMakeWeapon(bee_gobj, &weapon_desc, spawn_pos, GRREMIX_METALLIC_LASER_SPAWN_FLAGS);
    if (laser_gobj == NULL)
    {
        return NULL;
    }
    grRemixMetallicLaserSetSpeed(laser_gobj);
    efManagerFoxBlasterGlowMakeEffect(spawn_pos);
    return laser_gobj;
}

/** Sets the laser's X and Y speed from the bee facing and then its angle. */
static void grRemixMetallicLaserSetSpeed(GObj *laser_gobj)
{
    WPStruct *wp = wpGetStruct(laser_gobj);

    wp->physics.vel_air.x = grRemixMetallicLaserSpeedX();
    wp->physics.vel_air.y = GRREMIX_METALLIC_LASER_SPEED_Y;
    grRemixMetallicLaserSetAngle(laser_gobj);
}

/**
 * Sets the laser's Z rotation from its speed. The asm passes the X speed in f12 and the Y speed in f14 to the
 * angle routine (0x8001863C), and the port keeps that order.
 */
static void grRemixMetallicLaserSetAngle(GObj *laser_gobj)
{
    WPStruct *wp = wpGetStruct(laser_gobj);

    DObjGetStruct(laser_gobj)->rotate.vec.f.z = atan2f(wp->physics.vel_air.x, wp->physics.vel_air.y);
}

#endif
