/**
 * ftremixlucasusp.c - Smash Remix Lucas PK Thunder multi-hit (src/Lucas/Lucas.asm usp_thunder_head_, usp_thunder_tail_
 * and usp_multi_hit_).
 *
 * The head joins the hit group of the trails (the asm "projectile instance id"), so a hit on any part is shared, and
 * every HIT_RATE frames the group forgets its victims so PK Thunder hits again. Only the controlled PK Thunder is
 * covered: the hooks of the reflected PK Thunder are commented out in the asm.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixnessshared.h"
#include "ft/ftremix/ftremixnessrules.h"
#include "ft/ftremix/ftremix.h"
#include <sys/objman.h>

/** True if `wp` was fired by Lucas. */
static sb32 ftRemixLucasUSPIsLucasWeapon(WPStruct *wp)
{
    if (wp->owner_gobj == NULL)
    {
        return FALSE;
    }
    return ftGetStruct(wp->owner_gobj)->fkind == REMIX_ID_LUCAS;
}

/** First weapon of the weapon list that has the hit group `group_id`, or NULL. */
static GObj* ftRemixLucasUSPFindFirstOfGroup(u32 group_id)
{
    GObj *weapon_gobj = gGCCommonLinks[nGCCommonLinkIDWeapon];

    while (weapon_gobj != NULL)
    {
        if (wpGetStruct(weapon_gobj)->group_id == group_id)
        {
            return weapon_gobj;
        }
        weapon_gobj = weapon_gobj->link_next;
    }
    return NULL;
}

/** Forgets the victims recorded by `wp` (asm: stores 0 in the four hit object pointers). */
static void ftRemixLucasUSPClearVictims(WPStruct *wp)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(wp->attack_coll.attack_records); i++)
    {
        wp->attack_coll.attack_records[i].victim_gobj = NULL;
    }
}

void ftRemixLucasUSPInitHead(WPStruct *head_wp)
{
    if (!ftRemixLucasUSPIsLucasWeapon(head_wp))
    {
        return;
    }
    head_wp->group_id = wpManagerGetGroupID();
}

u32 ftRemixLucasUSPTrailGroupID(WPStruct *head_wp)
{
    if (!ftRemixLucasUSPIsLucasWeapon(head_wp))
    {
        return wpManagerGetGroupID();
    }
    return head_wp->group_id;
}

void ftRemixLucasUSPRefreshHits(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    GObj *other_gobj;
    WPStruct *other_wp;

    if (!ftRemixLucasUSPIsLucasWeapon(wp) || (wp->group_id == 0))
    {
        return;
    }
    if (ftRemixLucasUSPFindFirstOfGroup(wp->group_id) != weapon_gobj)
    {
        return;
    }
    if (!ftRemixNessRulesPKThunderHitRefreshDue(wp->lifetime))
    {
        return;
    }
    for (other_gobj = gGCCommonLinks[nGCCommonLinkIDWeapon]; other_gobj != NULL; other_gobj = other_gobj->link_next)
    {
        other_wp = wpGetStruct(other_gobj);

        if ((other_wp->group_id == wp->group_id) && ftRemixLucasUSPIsLucasWeapon(other_wp))
        {
            ftRemixLucasUSPClearVictims(other_wp);
        }
    }
}

#endif
