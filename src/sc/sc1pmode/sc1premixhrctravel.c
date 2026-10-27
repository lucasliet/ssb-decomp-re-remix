/**
 * sc1premixhrctravel.c - projectiles and player far down the Home-Run Contest field, see sc1premixhrctravel.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/objdef.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include <sys/objtypes.h>
#include "sc/sc1pmode/sc1premixhrctravel.h"
#include "sc/sc1pmode/sc1premixhrctravelrules.h"

#define SC1PREMIX_HRC_TRAVEL_MAX 32

/** One followed object: the traveler is the object's memory in the rules, is_seen marks that it still exists this frame. */
typedef struct SC1PRemixHrcTravelEntry
{
    GObj *gobj;
    SC1PRemixHrcTraveler traveler;
    sb32 is_seen;

} SC1PRemixHrcTravelEntry;

static SC1PRemixHrcTravelEntry sSC1PRemixHrcTravelEntries[SC1PREMIX_HRC_TRAVEL_MAX];
static s32 sSC1PRemixHrcTravelCount;

void sc1PRemixHrcTravelReset(void)
{
    sSC1PRemixHrcTravelCount = 0;
}

static SC1PRemixHrcTravelEntry* sc1PRemixHrcTravelFind(GObj *gobj)
{
    s32 i;

    for (i = 0; i < sSC1PRemixHrcTravelCount; i++)
    {
        if (sSC1PRemixHrcTravelEntries[i].gobj == gobj)
        {
            return &sSC1PRemixHrcTravelEntries[i];
        }
    }
    return NULL;
}

static SC1PRemixHrcTravelEntry* sc1PRemixHrcTravelGet(GObj *gobj)
{
    SC1PRemixHrcTravelEntry *entry = sc1PRemixHrcTravelFind(gobj);

    if (entry != NULL)
    {
        return entry;
    }
    if (sSC1PRemixHrcTravelCount >= SC1PREMIX_HRC_TRAVEL_MAX)
    {
        return NULL;
    }
    entry = &sSC1PRemixHrcTravelEntries[sSC1PRemixHrcTravelCount++];
    entry->gobj = gobj;
    entry->traveler = (SC1PRemixHrcTraveler){ 0, 0.0F, 0.0F };
    return entry;
}

static void sc1PRemixHrcTravelPlace(GObj *gobj, float camera_x, float warp_distance)
{
    DObj *top_joint = DObjGetStruct(gobj);
    SC1PRemixHrcTravelEntry *entry = sc1PRemixHrcTravelGet(gobj);

    if ((top_joint == NULL) || (entry == NULL))
    {
        return;
    }
    entry->is_seen = TRUE;
    top_joint->translate.vec.f.x = sc1PRemixHrcTravelerStep(&entry->traveler, top_joint->translate.vec.f.x, camera_x, warp_distance);
}

static void sc1PRemixHrcTravelForgetGone(void)
{
    s32 i;
    s32 kept = 0;

    for (i = 0; i < sSC1PRemixHrcTravelCount; i++)
    {
        if (sSC1PRemixHrcTravelEntries[i].is_seen != FALSE)
        {
            sSC1PRemixHrcTravelEntries[i].is_seen = FALSE;
            sSC1PRemixHrcTravelEntries[kept++] = sSC1PRemixHrcTravelEntries[i];
        }
    }
    sSC1PRemixHrcTravelCount = kept;
}

void sc1PRemixHrcTravelUpdate(GObj *human_gobj, float camera_x, float warp_distance)
{
    GObj *weapon_gobj;

    for (weapon_gobj = gGCCommonLinks[nGCCommonLinkIDWeapon]; weapon_gobj != NULL; weapon_gobj = weapon_gobj->link_next)
    {
        sc1PRemixHrcTravelPlace(weapon_gobj, camera_x, warp_distance);
    }
    sc1PRemixHrcTravelPlace(human_gobj, camera_x, warp_distance);
    sc1PRemixHrcTravelForgetGone();
}

#endif
