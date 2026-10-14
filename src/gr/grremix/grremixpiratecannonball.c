/**
 * grremixpiratecannonball.c - Pirate Land cannonball weapon. See grremixpiratecannonball.h.
 */

#ifdef PORT

#include "grremixpiratecannonball.h"
#include <ft/fighter.h>
#include <wp/weapon.h>
#include <ef/efmanager.h>
#include <gr/ground.h>

#define GRREMIX_PIRATE_CANNONBALL_KIND 0x28 /* Projectile.id.PIRATELAND_CANNONBALL, marina/ftremixmarinadsp.c */
#define GRREMIX_PIRATE_CANNONBALL_LIFETIME 999 /* properties duration; REMIX-TODO(animation end) below */
#define GRREMIX_PIRATE_CANNONBALL_SMOKE_INTERVAL 9
#define GRREMIX_PIRATE_CANNONBALL_EXPLOSION_TIMER 15
#define GRREMIX_PIRATE_CANNONBALL_DAMAGE 25
#define GRREMIX_PIRATE_CANNONBALL_SIZE 360.0F
#define GRREMIX_PIRATE_CANNONBALL_ANGLE 361
#define GRREMIX_PIRATE_CANNONBALL_KB_SCALE 90
#define GRREMIX_PIRATE_CANNONBALL_KB_BASE 50
#define GRREMIX_PIRATE_CANNONBALL_HIT_FGM 25
#define GRREMIX_PIRATE_CANNONBALL_ELEMENT_FIRE 1
#define GRREMIX_PIRATE_CANNONBALL_EXPLODE_FGM 1
#define GRREMIX_PIRATE_CANNONBALL_SHAKE_SEVERITY 1

extern void *func_800269C0_275C0(u16 id);

static sb32 grRemixPirateCannonballProcUpdate(GObj *weapon_gobj);

/* Stage-wide storage for the one cannonball the cannon can have in flight (asm: cannonball_hitbox_pointer). */
static void *sGRRemixPirateCannonballHitboxFile;

static struct
{
    sb32 is_first_frame;
    s32 smoke_timer;
    sb32 is_exploding;
} sGRRemixPirateCannonball;

static WPDesc sGRRemixPirateCannonballWeaponDesc =
{
    0x00,                                   /* Render flags */
    GRREMIX_PIRATE_CANNONBALL_KIND,
    &sGRRemixPirateCannonballHitboxFile,    /* Weapon data: the cannon hitbox file pointer */
    0,                                      /* Offset of the hitbox in the file (asm: cannonball_projectile_struct) */
    {
        nGCMatrixKindTra,
        0x47,
        0
    },
    grRemixPirateCannonballProcUpdate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/** Resets the per-shot state of the cannonball (asm: cannonball_stage_setting_ first frame flag and smoke timer). */
static void grRemixPirateCannonballResetState(void)
{
    sGRRemixPirateCannonball.is_first_frame = TRUE;
    sGRRemixPirateCannonball.smoke_timer = 0;
    sGRRemixPirateCannonball.is_exploding = FALSE;
}

/** Plays the joint animation track that places the barrel of the shot (asm: 0x8000BD1C). */
static void grRemixPirateCannonballApplyTrack(GObj *weapon_gobj, AObjEvent32 *anim_track)
{
    gcAddDObjAnimJoint(DObjGetStruct(weapon_gobj), anim_track, 0.0F);
}

/**
 * Creates a cannonball at spawn_pos. Returns NULL when the weapon pool is full (asm: 0x801655C8 returns 0).
 * REMIX-TODO(launch velocity): the properties struct sets speed 100 at angle 270 (ground and air), gravity 0. The
 * projectile system's angle and facing convention is not ported yet, so the cannonball starts at rest.
 * REMIX-TODO(camera): the asm sets the influence-camera flag (0x80 at 0x026C of the projectile special struct).
 * REMIX-TODO(joint rotation): the asm rotates the first joint to 90 degrees (Hazards.asm cannonball_stage_setting_).
 */
GObj* grRemixPirateCannonballMake(Vec3f *spawn_pos, void *hitbox_file, AObjEvent32 *anim_track)
{
    GObj *weapon_gobj;
    WPStruct *wp;

    sGRRemixPirateCannonballHitboxFile = hitbox_file;
    weapon_gobj = wpManagerMakeWeapon(NULL, &sGRRemixPirateCannonballWeaponDesc, spawn_pos, WEAPON_FLAG_PARENT_GROUND);
    if (weapon_gobj == NULL)
    {
        return NULL;
    }
    wp = wpGetStruct(weapon_gobj);
    wp->lifetime = GRREMIX_PIRATE_CANNONBALL_LIFETIME;
    wp->physics.vel_air.x = 0.0F;
    wp->physics.vel_air.y = 0.0F;
    grRemixPirateCannonballApplyTrack(weapon_gobj, anim_track);
    grRemixPirateCannonballResetState();

    return weapon_gobj;
}

/** Spawns a smoke puff every SMOKE_INTERVAL frames while the cannonball flies (asm: cannonball_main_ _smoke_trail). */
static void grRemixPirateCannonballUpdateSmoke(GObj *weapon_gobj)
{
    sGRRemixPirateCannonball.smoke_timer++;
    if (sGRRemixPirateCannonball.smoke_timer != GRREMIX_PIRATE_CANNONBALL_SMOKE_INTERVAL)
    {
        return;
    }
    sGRRemixPirateCannonball.smoke_timer = 0;
    efManagerDustExpandLargeMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f);
}

/**
 * Sets the explosion hitbox on the cannonball and starts its explosion timer (asm: cannonball_explosion_).
 * REMIX-TODO(samus bomb): the asm copies the Samus bomb hit-setup code from ROM 0xE396C and 0xE39A8 (vanilla
 * 0x80168F2C) and resets the four hit-object pointers. The copied code is not ported, so the hit records are kept.
 */
static void grRemixPirateCannonballExplode(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    Vec3f *pos = &DObjGetStruct(weapon_gobj)->translate.vec.f;

    wp->attack_coll.attack_state = 1;
    wp->attack_coll.size = GRREMIX_PIRATE_CANNONBALL_SIZE;
    wp->attack_coll.damage = GRREMIX_PIRATE_CANNONBALL_DAMAGE;
    wp->attack_coll.element = GRREMIX_PIRATE_CANNONBALL_ELEMENT_FIRE;
    wp->attack_coll.angle = GRREMIX_PIRATE_CANNONBALL_ANGLE;
    wp->attack_coll.knockback_scale = GRREMIX_PIRATE_CANNONBALL_KB_SCALE;
    wp->attack_coll.knockback_base = GRREMIX_PIRATE_CANNONBALL_KB_BASE;
    wp->attack_coll.fgm_id = GRREMIX_PIRATE_CANNONBALL_HIT_FGM;

    efManagerSparkleWhiteMultiExplodeMakeEffect(pos);
    func_800269C0_275C0(GRREMIX_PIRATE_CANNONBALL_EXPLODE_FGM);
    efManagerQuakeMakeEffect(GRREMIX_PIRATE_CANNONBALL_SHAKE_SEVERITY);

    wp->lifetime = GRREMIX_PIRATE_CANNONBALL_EXPLOSION_TIMER;
    sGRRemixPirateCannonball.is_exploding = TRUE;
}

/**
 * Per-frame process of the cannonball: flies, explodes, then expires. Returns TRUE to destroy the weapon.
 * REMIX-TODO(animation end): the asm explodes when the cannonball model animation ends (gobj frame 0x78). The
 * animation length is not read yet, so the explosion comes when the flight lifetime runs out.
 */
static sb32 grRemixPirateCannonballProcUpdate(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);

    if (sGRRemixPirateCannonball.is_exploding)
    {
        return wpMainDecLifeCheckExpire(wp);
    }
    if (sGRRemixPirateCannonball.is_first_frame)
    {
        sGRRemixPirateCannonball.is_first_frame = FALSE;
        return FALSE;
    }
    if (wpMainDecLifeCheckExpire(wp))
    {
        grRemixPirateCannonballExplode(weapon_gobj);
        return FALSE;
    }
    grRemixPirateCannonballUpdateSmoke(weapon_gobj);

    return FALSE;
}

#endif
