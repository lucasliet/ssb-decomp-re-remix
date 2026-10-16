/**
 * ftremixreflect.c - Smash Remix Reflect.asm: reflect hitboxes of type CUSTOM and their routines.
 */

#ifdef REGION_REMIX

#include "ftremixreflect.h"
#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <it/item.h>
#include <wp/weapon.h>
#include <gm/gmcollision.h>
#include <sys/objman.h>
#include <sys/objdef.h>
#include <ef/efmanager.h>
#include "it/itremix/itremixfranklinbadge.h"

#define FTREMIX_REFLECT_ID_FRANKLIN_BADGE 0
#define FTREMIX_REFLECT_ID_MARINA 1
#define FTREMIX_REFLECT_ID_DEDEDE 2
#define FTREMIX_REFLECT_WP_PIRATELAND_CANNONBALL 0x28   /* Projectile.id.PIRATELAND_CANNONBALL */
#define FTREMIX_REFLECT_WP_METALLICMADNESS_BEELASER 0x29 /* Projectile.id.METALLICMADNESS_BEELASER */
#define FTREMIX_REFLECT_WP_SONIC_SPRING 0x20            /* Projectile.id.SONIC_SPRING */
#define FTREMIX_REFLECT_THUNDER_SHAKE 1
#define FTREMIX_REFLECT_THUNDER_GFX_SIZE 23

extern void wpPikachuThunderHeadSetDestroy(GObj *weapon_gobj, sb32 is_destroy);

static u32 ftRemixReflectGetKind(FTStruct *fp)
{
    return (fp->special_coll != NULL) ? (u32)fp->special_coll->kind : 0;
}

sb32 ftRemixReflectIsCustom(FTStruct *fp)
{
    return (fp->special_coll != NULL) && ((ftRemixReflectGetKind(fp) & 0xFFFF) == FTREMIX_REFLECT_TYPE_CUSTOM);
}

static s32 ftRemixReflectGetCustomID(FTStruct *fp)
{
    return (s32)(ftRemixReflectGetKind(fp) >> 16);
}

/** Custom ids whose hitbox absorbs instead of reflecting (everything but the Franklin Badge). */
static sb32 ftRemixReflectIsAbsorbing(FTStruct *fp)
{
    return ftRemixReflectIsCustom(fp) && (ftRemixReflectGetCustomID(fp) != FTREMIX_REFLECT_ID_FRANKLIN_BADGE);
}

sb32 ftRemixReflectCanReflectWeapon(FTStruct *fp, WPStruct *wp)
{
    if (fp->special_coll == NULL)
    {
        return FALSE;
    }
    if (ftRemixReflectIsCustom(fp))
    {
        switch (wp->kind)
        {
        case nWPKindSamusBomb:
        case nWPKindThunderHead:
        case nWPKindThunderTrail:
        case nWPKindIwarkRock:
        case nWPKindKamexHydro:
        case nWPKindDogasSmog:
        case FTREMIX_REFLECT_WP_PIRATELAND_CANNONBALL:
        case FTREMIX_REFLECT_WP_METALLICMADNESS_BEELASER:
            return TRUE;
        }
    }
    return wp->attack_coll.can_reflect;
}

void ftRemixReflectRunCustom(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    switch (ftRemixReflectGetCustomID(fp))
    {
    case FTREMIX_REFLECT_ID_FRANKLIN_BADGE:
        itRemixFranklinBadgeReflectInitial(fighter_gobj);
        break;

    case FTREMIX_REFLECT_ID_MARINA:
        ftRemixMarinaDSPAbsorbInitial(fighter_gobj);
        break;

    case FTREMIX_REFLECT_ID_DEDEDE:
        ftRemixDededeNSPAbsorbInitial(fighter_gobj);
        break;
    }
}

/** pikachu_thunder_absorb_: the trail absorbed by an absorb hitbox sets off the thunder head of the same owner. */
static void ftRemixReflectAbsorbThunderTrail(GObj *trail_gobj)
{
    WPStruct *trail_wp = wpGetStruct(trail_gobj);
    GObj *head_gobj = gGCCommonLinks[nGCCommonLinkIDWeapon];

    while (head_gobj != NULL)
    {
        WPStruct *head_wp = wpGetStruct(head_gobj);

        if ((head_gobj != trail_gobj) && (head_wp->kind == nWPKindThunderHead) && (head_wp->owner_gobj == trail_wp->owner_gobj))
        {
            Vec3f *pos = &DObjGetStruct(head_gobj)->translate.vec.f;

            wpPikachuThunderHeadSetDestroy(head_gobj, TRUE);
            efManagerQuakeMakeEffect(FTREMIX_REFLECT_THUNDER_SHAKE);
            efManagerDamageElectricMakeEffect(pos, FTREMIX_REFLECT_THUNDER_GFX_SIZE);
            wpMainDestroyWeapon(head_gobj);
            return;
        }
        head_gobj = head_gobj->link_next;
    }
}

FTRemixReflectResult ftRemixReflectWeapon(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    FTStruct *fp = ftGetStruct(wp->reflect_gobj);

    if (!ftRemixReflectIsAbsorbing(fp))
    {
        return nFTRemixReflectOriginal;
    }
    if (wp->kind == nWPKindThunderTrail)
    {
        ftRemixReflectAbsorbThunderTrail(weapon_gobj);

        return nFTRemixReflectHandled;
    }
    if ((wp->proc_absorb != NULL) && (wp->proc_absorb(weapon_gobj) != FALSE))
    {
        return nFTRemixReflectDestroy;
    }
    if (wp->proc_dead != NULL)
    {
        wp->proc_dead(weapon_gobj);
    }
    return nFTRemixReflectDestroy;
}

FTRemixReflectResult ftRemixReflectItem(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    FTStruct *fp = ftGetStruct(ip->reflect_gobj);

    if (!ftRemixReflectIsAbsorbing(fp))
    {
        return nFTRemixReflectOriginal;
    }
    if (ip->proc_dead != NULL)
    {
        ip->proc_dead(item_gobj);
    }
    return nFTRemixReflectDestroy;
}

sb32 ftRemixReflectInvertsDirection(FTStruct *fp)
{
    return ftRemixReflectIsCustom(fp);
}

#endif
