/**
 * grremixrainbowchomp.c - Chain Chomp hitbox weapon of Rainbow Road (Hazards.asm chainchomp_hitbox_*). See
 * grremixrainbowchomp.h.
 */

#ifdef PORT

#include "grremixrainbowchomp.h"
#include "grremixrainbow.h"
#include <gr/ground.h>
#include <wp/wpmanager.h>
#include <wp/weapon.h>

/* Projectile.id.RAINBOWROAD_CHAINCHOMP: 0x20 + add_projectile order in .toolchain/pkgs/smashremix-src/src/Projectile.asm. */
#define GRREMIX_RAINBOW_CHOMP_KIND 0x002A
#define GRREMIX_RAINBOW_CHOMP_LIFETIME 0x0546
#define GRREMIX_RAINBOW_CHOMP_DAMAGE 25

/** Hurtbox, shield, shield edge and clang handler: the asm routine does nothing and never destroys the weapon. */
static sb32 grRemixRainbowChompProcConnect(GObj *wp_gobj)
{
    return FALSE;
}

/** Copies the chomp's top joint position to the hitbox weapon (wpManagerMakeWeapon always creates its DObj). */
static void grRemixRainbowChompSyncPosition(GObj *wp_gobj)
{
    DObj *wp_dobj = DObjGetStruct(wp_gobj);
    DObj *chomp_dobj = DObjGetStruct(grRemixRainbowGetChompGObj());

    wp_dobj->translate.vec.f = chomp_dobj->translate.vec.f;
}

/**
 * Per-frame hitbox routine (chainchomp_hitbox_main_). Destroys the weapon when its lifetime is spent, otherwise counts
 * it down.
 */
static sb32 grRemixRainbowChompProcUpdate(GObj *wp_gobj)
{
    WPStruct *wp = wpGetStruct(wp_gobj);

    grRemixRainbowChompSyncPosition(wp_gobj);
    /* REMIX-TODO(hitbox monitor): the asm refreshes three hitbox slots (projectile offsets 0x214 and 0x29C) and rolls a chomp sound from the frame count at 0x0E; no port equivalent is mapped. */

    if (wp->lifetime == 0)
    {
        return TRUE;
    }
    wp->lifetime--;
    return FALSE;
}

/* The asm projectile word 0x1C000000 (offset 0x10) is the transform kind nGCMatrixKindTraRotRpyRSca. */
static WPDesc sRainbowChompDesc =
{
    .flags = 0,
    .kind = GRREMIX_RAINBOW_CHOMP_KIND,
    .p_weapon = NULL,
    .o_attributes = 0,
    .transform_types = { nGCMatrixKindTraRotRpyRSca, nGCMatrixKindNull, 0 },
    .proc_update = grRemixRainbowChompProcUpdate,
    .proc_map = NULL,
    .proc_hit = grRemixRainbowChompProcConnect,
    .proc_shield = grRemixRainbowChompProcConnect,
    .proc_hop = grRemixRainbowChompProcConnect,
    .proc_setoff = grRemixRainbowChompProcConnect,
    .proc_reflector = NULL,
    .proc_absorb = NULL,
};

/** Spawn position: the chomp's top joint x and y, with z = 0 (the asm stores 0 in z, not the joint's z). */
static void grRemixRainbowChompSpawnPosition(GObj *chomp_gobj, Vec3f *spawn_pos)
{
    Vec3f *chomp_pos = &DObjGetStruct(chomp_gobj)->translate.vec.f;

    spawn_pos->x = chomp_pos->x;
    spawn_pos->y = chomp_pos->y;
    spawn_pos->z = 0.0F;
}

/** Applies the hitbox properties the asm writes after spawn: reflectable, lifetime and damage. */
static void grRemixRainbowChompSetProperties(WPStruct *wp)
{
    wp->attack_coll.can_reflect = TRUE;
    wp->lifetime = GRREMIX_RAINBOW_CHOMP_LIFETIME;
    wp->attack_coll.damage = GRREMIX_RAINBOW_CHOMP_DAMAGE;
}

GObj *grRemixRainbowChompSpawn(GObj *chomp_gobj, void **hitbox_file_slot)
{
    Vec3f spawn_pos;
    GObj *wp_gobj;

    grRemixRainbowChompSpawnPosition(chomp_gobj, &spawn_pos);
    sRainbowChompDesc.p_weapon = hitbox_file_slot;
    wp_gobj = wpManagerMakeWeapon(NULL, &sRainbowChompDesc, &spawn_pos, WEAPON_FLAG_PARENT_GROUND);
    if (wp_gobj == NULL)
    {
        return NULL;
    }
    grRemixRainbowChompSetProperties(wpGetStruct(wp_gobj));
    return wp_gobj;
}

#endif
