/**
 * grremixrainbowchomp.c - Chain Chomp hitbox weapon of Rainbow Road (Hazards.asm chainchomp_hitbox_*). See
 * grremixrainbowchomp.h.
 */

#ifdef PORT

#include "grremixrainbowchomp.h"
#include "grremixrainbow.h"
#include <gr/ground.h>
#include <sys/utils.h>
#include <wp/wpmanager.h>
#include <wp/weapon.h>

extern void *func_800269C0_275C0(u16 id);

/* Projectile.id.RAINBOWROAD_CHAINCHOMP: 0x20 + add_projectile order in .toolchain/pkgs/smashremix-src/src/Projectile.asm. */
#define GRREMIX_RAINBOW_CHOMP_KIND 0x002A
#define GRREMIX_RAINBOW_CHOMP_LIFETIME 0x0546
#define GRREMIX_RAINBOW_CHOMP_DAMAGE 25
/* Hazards.asm chainchomp_hitbox_main_: roll of Global.get_random_int_(0x50) that must return 2, during frames 0-14. */
#define GRREMIX_RAINBOW_CHOMP_SOUND_ROLL_RANGE 0x50
#define GRREMIX_RAINBOW_CHOMP_SOUND_ROLL_HIT 2
#define GRREMIX_RAINBOW_CHOMP_SOUND_FRAMES 0x0F
#define GRREMIX_RAINBOW_CHOMP_FGM 0x0401
/* Hazards.asm chainchomp_hitbox_main_ loop: three records (WPStruct 0x214, stride 8) and counters (WPStruct 0x29C). */
#define GRREMIX_RAINBOW_CHOMP_HITBOX_SLOTS 3
/* Hazards.asm chain_chomp_projectile_counter: refresh period (0x19) stored when a hit record is created. */
#define GRREMIX_RAINBOW_CHOMP_REFRESH_FRAMES 0x19

/* Per-record refresh counter. The asm keeps these bytes in the weapon_vars union (WPStruct 0x29C..0x29E). */
typedef struct GRRemixRainbowChompHitSlot
{
    u8 refresh_frames;
    sb32 is_counting;

} GRRemixRainbowChompHitSlot;

static GRRemixRainbowChompHitSlot sGRRemixRainbowChompHitSlots[GRREMIX_RAINBOW_CHOMP_HITBOX_SLOTS];

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
 * Chain chomp growl of the first frames (chainchomp_hitbox_main_): rolls every frame of frames 0-14 and plays the FGM
 * on a roll of 2. The asm reads the frame count at projectile offset 0x0E; the port derives it from the spawn lifetime,
 * since no port field is mapped to that byte.
 */
static void grRemixRainbowChompRollSound(WPStruct *wp)
{
    s32 frame = GRREMIX_RAINBOW_CHOMP_LIFETIME - wp->lifetime;
    s32 roll;

    if (frame >= GRREMIX_RAINBOW_CHOMP_SOUND_FRAMES)
    {
        return;
    }
    roll = syUtilsRandIntRange(GRREMIX_RAINBOW_CHOMP_SOUND_ROLL_RANGE);
    if (roll == GRREMIX_RAINBOW_CHOMP_SOUND_ROLL_HIT)
    {
        func_800269C0_275C0(GRREMIX_RAINBOW_CHOMP_FGM);
    }
}

/**
 * Refreshes one hit record (chainchomp_hitbox_main_ _loop). The asm patch chain_chomp_projectile_counter, inside the
 * vanilla hit record routine (0x801666A0, wpProcessSetHitInteractStats), stores 0x19 in the counter of every record the
 * chomp creates. The port has no such hook, so a record seen with no counter running is a new one and starts it here;
 * the first update after creation then decrements it, as in the asm. A counter at 0 clears only the victim pointer,
 * the flags are kept as in the asm.
 */
static void grRemixRainbowChompRefreshHitSlot(GMAttackRecord *record, GRRemixRainbowChompHitSlot *slot)
{
    if (record->victim_gobj == NULL)
    {
        slot->is_counting = FALSE;
        return;
    }
    if (!slot->is_counting)
    {
        slot->is_counting = TRUE;
        slot->refresh_frames = GRREMIX_RAINBOW_CHOMP_REFRESH_FRAMES;
    }
    if (slot->refresh_frames == 0)
    {
        record->victim_gobj = NULL;
        slot->is_counting = FALSE;
        return;
    }
    slot->refresh_frames--;
}

/** Lets the chomp hit the same fighter again 0x1A frames after a hit. The fourth record is never refreshed (asm loops 3 times). */
static void grRemixRainbowChompRefreshHitSlots(WPStruct *wp)
{
    s32 i;

    for (i = 0; i < GRREMIX_RAINBOW_CHOMP_HITBOX_SLOTS; i++)
    {
        grRemixRainbowChompRefreshHitSlot(&wp->attack_coll.attack_records[i], &sGRRemixRainbowChompHitSlots[i]);
    }
}

/** Stops the counters of a previous chomp weapon (the asm counters live in the weapon, so a new weapon starts at 0). */
static void grRemixRainbowChompResetHitSlots(void)
{
    s32 i;

    for (i = 0; i < GRREMIX_RAINBOW_CHOMP_HITBOX_SLOTS; i++)
    {
        sGRRemixRainbowChompHitSlots[i].refresh_frames = 0;
        sGRRemixRainbowChompHitSlots[i].is_counting = FALSE;
    }
}

/**
 * Per-frame hitbox routine (chainchomp_hitbox_main_). Destroys the weapon when its lifetime is spent, otherwise counts
 * it down.
 */
static sb32 grRemixRainbowChompProcUpdate(GObj *wp_gobj)
{
    WPStruct *wp = wpGetStruct(wp_gobj);

    grRemixRainbowChompSyncPosition(wp_gobj);
    grRemixRainbowChompRefreshHitSlots(wp);
    grRemixRainbowChompRollSound(wp);

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

/**
 * Blast zone callback (Hazards.asm blast_zone_destroy_prevent): the chomp weapon is never destroyed for leaving the map
 * bounds; the asm skips the whole bounds block for its projectile id.
 */
static sb32 grRemixRainbowChompProcDead(GObj *wp_gobj)
{
    return FALSE;
}

/** Applies the hitbox properties the asm writes after spawn: reflectable, lifetime, damage and the blast zone callback. */
static void grRemixRainbowChompSetProperties(WPStruct *wp)
{
    wp->proc_dead = grRemixRainbowChompProcDead;
    wp->attack_coll.can_reflect = TRUE;
    wp->lifetime = GRREMIX_RAINBOW_CHOMP_LIFETIME;
    wp->attack_coll.damage = GRREMIX_RAINBOW_CHOMP_DAMAGE;
}

GObj *grRemixRainbowChompSpawn(GObj *chomp_gobj, void **hitbox_file_slot)
{
    Vec3f spawn_pos;
    GObj *wp_gobj;

    grRemixRainbowChompSpawnPosition(chomp_gobj, &spawn_pos);
    grRemixRainbowChompResetHitSlots();
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
