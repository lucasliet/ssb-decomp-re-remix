/**
 * grremixpiratecannonball.c - Pirate Land cannonball weapon. See grremixpiratecannonball.h.
 */

#ifdef PORT

#include "grremixpiratecannonball.h"
#include <ft/fighter.h>
#include <wp/weapon.h>
#include <ef/efmanager.h>
#include <gr/ground.h>
#include <port_log.h>

#define GRREMIX_PIRATE_CANNONBALL_KIND 0x28 /* Projectile.id.PIRATELAND_CANNONBALL, marina/ftremixmarinadsp.c */
#define GRREMIX_PIRATE_CANNONBALL_LIFETIME 999 /* properties duration; the animation end explodes it first */
#define GRREMIX_PIRATE_CANNONBALL_SMOKE_INTERVAL 9
#define GRREMIX_PIRATE_CANNONBALL_JOINT_ROTATION_Y 1.5707963F
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
}

/** Plays the joint animation track that places the barrel of the shot (asm: 0x8000BD1C). */
static void grRemixPirateCannonballApplyTrack(GObj *weapon_gobj, AObjEvent32 *anim_track)
{
    gcAddDObjAnimJoint(DObjGetStruct(weapon_gobj), anim_track, 0.0F);
}

/**
 * Creates a cannonball at spawn_pos. Returns NULL when the weapon pool is full (asm: 0x801655C8 returns 0).
 * The cannonball has no launch velocity: wpManagerMakeWeapon (0x801655C8) zeroes vel_air and the asm never writes it.
 * cannonball_properties_struct (speed 100, angle 270, gravity 0) is dead data: its only reference is the file pointer
 * store at pirate_land_setup (Hazards.asm 3736-3737), and cannonball_stage_setting_ passes only
 * cannonball_projectile_struct to 0x801655C8 (4171-4175), which has no field that points to it. The ball is moved by
 * the barrel joint animation applied at the end of this function.
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
    wp->is_camera_follow = TRUE;
    /* Hazards.asm 4185-4193: the first joint is turned 90 degrees before the barrel animation is applied. The RAM float
       at 0x8019CA80 is taken to be pi/2 from the asm comment; its bit pattern was not read. */
    DObjGetStruct(weapon_gobj)->rotate.vec.f.y = GRREMIX_PIRATE_CANNONBALL_JOINT_ROTATION_Y;
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

/** Writes the explosion hit record values the asm stores before the Samus bomb setup (cannonball_explosion_). */
static void grRemixPirateCannonballSetExplosionHitbox(WPStruct *wp)
{
    wp->attack_coll.attack_state = 1;
    wp->attack_coll.size = GRREMIX_PIRATE_CANNONBALL_SIZE;
    wp->attack_coll.damage = GRREMIX_PIRATE_CANNONBALL_DAMAGE;
    wp->attack_coll.element = GRREMIX_PIRATE_CANNONBALL_ELEMENT_FIRE;
    wp->attack_coll.angle = GRREMIX_PIRATE_CANNONBALL_ANGLE;
    wp->attack_coll.knockback_scale = GRREMIX_PIRATE_CANNONBALL_KB_SCALE;
    wp->attack_coll.knockback_base = GRREMIX_PIRATE_CANNONBALL_KB_BASE;
    wp->attack_coll.fgm_id = GRREMIX_PIRATE_CANNONBALL_HIT_FGM;
}

/** Forgets the victims of the flight and removes the owner (asm: "refresh hitbox" and "remove owner" stores). */
static void grRemixPirateCannonballRefreshHitbox(WPStruct *wp)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(wp->attack_coll.attack_records); i++)
    {
        wp->attack_coll.attack_records[i].victim_gobj = NULL;
    }
    wp->owner_gobj = NULL;
}

/**
 * Turns the cannonball into its explosion and starts the explosion timer (asm: cannonball_explosion_).
 * The asm copies the first 14 and the next 10 instructions of the vanilla Samus bomb explosion setup
 * (ROM 0xE396C / 0xE39A8 = RAM 0x80168F2C..0x80168F60 and 0x80168F68..0x80168F8C, ROM to RAM delta 0x800855C0),
 * skips the size store at 0x80168F64 (sets 180.0F) and the final jr, and adds sw r0, 0x290(v0). That is every
 * statement of wpSamusBombExplodeInitVars except the size line: lifetime 6, can_rehit_item set, can_hop clear, zero
 * vel_air, hidden model, proc_update = wpSamusBombExplodeProcUpdate, and all other procs cleared. The hitbox values
 * are written after the call so the size stays 360, and the lifetime is then overwritten with 15.
 */
static void grRemixPirateCannonballExplode(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    Vec3f *pos = &DObjGetStruct(weapon_gobj)->translate.vec.f;

    port_log("[remix-stage] pirate cannonball explode x=%.0f y=%.0f frame=%.1f\n", pos->x, pos->y, weapon_gobj->anim_frame);
    efManagerSparkleWhiteMultiExplodeMakeEffect(pos);
    wpSamusBombExplodeInitVars(weapon_gobj);
    grRemixPirateCannonballSetExplosionHitbox(wp);
    func_800269C0_275C0(GRREMIX_PIRATE_CANNONBALL_EXPLODE_FGM);
    efManagerQuakeMakeEffect(GRREMIX_PIRATE_CANNONBALL_SHAKE_SEVERITY);

    wp->lifetime = GRREMIX_PIRATE_CANNONBALL_EXPLOSION_TIMER;
    grRemixPirateCannonballRefreshHitbox(wp);
}

/**
 * Per-frame process of the cannonball: flies and explodes when its model animation ends (asm reads the GObj frame at
 * 0x78). The explosion replaces this process with wpSamusBombExplodeProcUpdate, which expires the weapon. The lifetime
 * only guards against an animation that never ends. Returns TRUE to destroy the weapon.
 */
static sb32 grRemixPirateCannonballProcUpdate(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);

    if (sGRRemixPirateCannonball.is_first_frame)
    {
        sGRRemixPirateCannonball.is_first_frame = FALSE;
        return FALSE;
    }
    if ((weapon_gobj->anim_frame <= 0.0F) || wpMainDecLifeCheckExpire(wp))
    {
        grRemixPirateCannonballExplode(weapon_gobj);
        return FALSE;
    }
    grRemixPirateCannonballUpdateSmoke(weapon_gobj);

    return FALSE;
}

#endif
