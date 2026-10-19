/**
 * ftremixperfectshield.c - Smash Remix "Perfect Shielding" setting (PerfectShield/PerfectShield.asm, toggle entry_perfect_shield).
 * A shield raised in its first two frames reflects the projectiles and items that hit it, and a shield hit in that window
 * causes no shield stun. Every entry point is a no-op while the setting is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ftremixreflect.h"
#include <ft/fighter.h>
#include <it/item.h>
#include <wp/weapon.h>
#include <gm/gmcollision.h>
#include <ef/efmanager.h>
#include <sys/objman.h>
#include <sys/objdef.h>
#include "gm/gmremixtoggle.h"

#define FTREMIX_PERFECT_SHIELD_REFLECT_ID 0
#define FTREMIX_PERFECT_SHIELD_WINDOW_TICS 2
#define FTREMIX_PERFECT_SHIELD_STALE_CAP 0.5F
#define FTREMIX_PERFECT_SHIELD_JOINT 4
#define FTREMIX_PERFECT_SHIELD_HITBOX_SIZE 350.0F
#define FTREMIX_PERFECT_SHIELD_HITBOX_Y 60.0F
#define FTREMIX_PERFECT_SHIELD_DAMAGE_RESIST 0x18000000

/* PerfectShield.asm reflect_hitbox_struct: the CUSTOM reflect hitbox the shield takes over while a perfect shield reflects.
 * Custom id 0 runs the first Reflect.custom_reflect_table routine, as in the Remix source. */
static FTSpecialColl sFTRemixPerfectShieldColl =
{
    (FTREMIX_PERFECT_SHIELD_REFLECT_ID << 16) | FTREMIX_REFLECT_TYPE_CUSTOM,
    FTREMIX_PERFECT_SHIELD_JOINT,
    { 0.0F, FTREMIX_PERFECT_SHIELD_HITBOX_Y, 0.0F },
    { FTREMIX_PERFECT_SHIELD_HITBOX_SIZE, FTREMIX_PERFECT_SHIELD_HITBOX_SIZE, FTREMIX_PERFECT_SHIELD_HITBOX_SIZE },
    FTREMIX_PERFECT_SHIELD_DAMAGE_RESIST
};

/** TRUE when gobj is a fighter, that is, an element of the fighter list gGCCommonLinks[nGCCommonLinkIDFighter]. */
static sb32 ftRemixPerfectShieldIsFighter(GObj *gobj)
{
    GObj *fighter_gobj;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        if (fighter_gobj == gobj)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** The fighter of the first attack record whose victim is a fighter, or NULL. The first match decides, as in the Remix loop. */
static FTStruct* ftRemixPerfectShieldFindVictim(GMAttackRecord *records)
{
    s32 i;

    for (i = 0; i < GMATTACKREC_NUM_MAX; i++)
    {
        if (ftRemixPerfectShieldIsFighter(records[i].victim_gobj))
        {
            return ftGetStruct(records[i].victim_gobj);
        }
    }
    return NULL;
}

/** TRUE when the shield of fp is up and still within the perfect shield window. */
static sb32 ftRemixPerfectShieldIsInWindow(FTStruct *fp)
{
    return (fp->status_id == nFTCommonStatusGuardOn) && (fp->status_total_tics < FTREMIX_PERFECT_SHIELD_WINDOW_TICS);
}

/** Clears the victims of all attack records, so the reflected object does not hit the shield again. */
static void ftRemixPerfectShieldClearRecords(GMAttackRecord *records)
{
    s32 i;

    for (i = 0; i < GMATTACKREC_NUM_MAX; i++)
    {
        records[i].victim_gobj = NULL;
    }
}

/** Halves the damage multiplier (stale), capped at FTREMIX_PERFECT_SHIELD_STALE_CAP. */
static f32 ftRemixPerfectShieldHalveStale(f32 stale)
{
    f32 halved = 0.5F * stale;

    if (halved > FTREMIX_PERFECT_SHIELD_STALE_CAP)
    {
        return FTREMIX_PERFECT_SHIELD_STALE_CAP;
    }
    return halved;
}

/** PerfectShield.asm fighter_gfx: the yellow swirl (catch effect) at the shield joint, the origin of the shield. */
static void ftRemixPerfectShieldMakeGfx(FTStruct *fp)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };

    gmCollisionGetFighterPartsWorldPosition(fp->joints[nFTPartsJointYRotN], &pos);
    efManagerCatchSwirlMakeEffect(&pos);
}

/** PerfectShield.asm shield_stun_action_change_skip: TRUE when a perfect shield took the place of the shield stun. */
sb32 ftRemixPerfectShieldHandlesGuardSetOff(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!gmRemixToggle(perfect_shield))
    {
        return FALSE;
    }
    if (fp->status_id != nFTCommonStatusGuardOn)
    {
        return FALSE;
    }
    fp->hitlag_tics = 0;
    fp->shield_damage_total = 0;
    fp->shield_damage = 0;
    ftRemixPerfectShieldMakeGfx(fp);

    return TRUE;
}

/** PerfectShield.asm shield_reflect_projectiles: TRUE when the shield reflects the projectile; the caller then runs its reflect routine. */
sb32 ftRemixPerfectShieldWeapon(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    FTStruct *fp;

    if (!gmRemixToggle(perfect_shield))
    {
        return FALSE;
    }
    fp = ftRemixPerfectShieldFindVictim(wp->attack_coll.attack_records);

    if ((fp == NULL) || !ftRemixPerfectShieldIsInWindow(fp))
    {
        return FALSE;
    }
    ftRemixPerfectShieldClearRecords(wp->attack_coll.attack_records);
    fp->special_coll = &sFTRemixPerfectShieldColl;
    wp->owner_gobj = fp->fighter_gobj;
    wp->attack_coll.stale = ftRemixPerfectShieldHalveStale(wp->attack_coll.stale);

    return TRUE;
}

/** PerfectShield.asm shield_reflect_items: TRUE when the shield reflects the item; the caller then runs its reflect routine. */
sb32 ftRemixPerfectShieldItem(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    FTStruct *fp;

    if (!gmRemixToggle(perfect_shield))
    {
        return FALSE;
    }
    fp = ftRemixPerfectShieldFindVictim(ip->attack_coll.attack_records);

    if ((fp == NULL) || !ftRemixPerfectShieldIsInWindow(fp))
    {
        return FALSE;
    }
    ftRemixPerfectShieldClearRecords(ip->attack_coll.attack_records);
    fp->special_coll = &sFTRemixPerfectShieldColl;

    /* PerfectShield.asm keeps the owner of a Poke Ball (Hazards.standard.POKEBALL, nITKindMBall) unchanged */
    if (ip->kind != nITKindMBall)
    {
        ip->owner_gobj = fp->fighter_gobj;
    }
    ip->attack_coll.stale = ftRemixPerfectShieldHalveStale(ip->attack_coll.stale);

    return TRUE;
}

#endif
