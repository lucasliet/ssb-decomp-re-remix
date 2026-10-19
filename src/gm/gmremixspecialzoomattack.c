/**
 * gmremixspecialzoomattack.c - Smash Remix SpecialZoom.asm check_special_zoom: the attacker's special moves that trigger
 * the Special Zoom (BETA) setting, by fighter kind and status id.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixhooks.h"

/* Status ids of the Special Zoom attacker list (Action.asm scopes CAPTAIN, DK, LUIGI, KIRBY, JIGGLY, NESS, top-level
 * UTilt, Ganondorf, Lucas and Roy; Roy's ids are DSP_G_Strong_End and DSP_A_Strong_End by add_new_action order) */
enum
{
    nFTRemixSpecialZoomStatusFalconPunch = 0xE4,
    nFTRemixSpecialZoomStatusFalconPunchAir = 0xE5,
    nFTRemixSpecialZoomStatusGiantPunchFullyCharged = 0xE4,
    nFTRemixSpecialZoomStatusGiantPunchFullyChargedAir = 0xE5,
    nFTRemixSpecialZoomStatusSuperJumpPunch = 0xE1,
    nFTRemixSpecialZoomStatusWarlockPunch = 0xE4,
    nFTRemixSpecialZoomStatusWarlockPunchAir = 0xE5,
    nFTRemixSpecialZoomStatusUTilt = 0xC7,
    nFTRemixSpecialZoomStatusJigglyRest = 0xEA,
    nFTRemixSpecialZoomStatusJigglyRestAir = 0xEB,
    nFTRemixSpecialZoomStatusPKTA = 0xE7,
    nFTRemixSpecialZoomStatusPKTAAir = 0xEC,
    nFTRemixSpecialZoomStatusRoyDSPGroundStrongEnd = 0xF2,
    nFTRemixSpecialZoomStatusRoyDSPAirStrongEnd = 0xF6,
    nFTRemixSpecialZoomStatusKirbyFalconPunch = 0x127,
    nFTRemixSpecialZoomStatusKirbyFalconPunchAir = 0x128,
    nFTRemixSpecialZoomStatusKirbyWarlockPunch = 0x12F,
    nFTRemixSpecialZoomStatusKirbyWarlockPunchAir = 0x130,
    nFTRemixSpecialZoomStatusKirbyFullyChargedPunch = 0xF8,
    nFTRemixSpecialZoomStatusKirbyFullyChargedPunchAir = 0xF9,
};

/* Luigi's Super Jump Punch only counts on its first animation frame (SpecialZoom.asm attacker_special_move_luigi) */
#define FTREMIX_SPECIAL_ZOOM_LUIGI_FIRST_FRAME 1

/**
 * TRUE when status_id is one of the two given status ids.
 */
static sb32 ftRemixSpecialZoomIsStatusOf(s32 status_id, s32 first_status_id, s32 second_status_id)
{
    return (status_id == first_status_id) || (status_id == second_status_id);
}

/**
 * TRUE when a Kirby is in one of the punch statuses that trigger the zoom.
 */
static sb32 ftRemixSpecialZoomIsKirbyPunchStatus(s32 status_id)
{
    return ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusKirbyFalconPunch, nFTRemixSpecialZoomStatusKirbyFalconPunchAir)
        || ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusKirbyWarlockPunch, nFTRemixSpecialZoomStatusKirbyWarlockPunchAir)
        || ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusKirbyFullyChargedPunch, nFTRemixSpecialZoomStatusKirbyFullyChargedPunchAir);
}

/**
 * TRUE when the attacker fighter is in a special move that triggers the zoom, for its fighter kind.
 */
static sb32 ftRemixSpecialZoomIsAttackerStatus(FTStruct *attacker_fp)
{
    s32 status_id = attacker_fp->status_id;

    switch (attacker_fp->fkind)
    {
        case nFTKindCaptain:
        case REMIX_ID_JFALCON:
            return ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusFalconPunch, nFTRemixSpecialZoomStatusFalconPunchAir);
        case nFTKindDonkey:
        case REMIX_ID_JDK:
            return ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusGiantPunchFullyCharged, nFTRemixSpecialZoomStatusGiantPunchFullyChargedAir);
        case nFTKindLuigi:
        case REMIX_ID_JLUIGI:
        case REMIX_ID_MLUIGI:
            return (attacker_fp->status_total_tics == FTREMIX_SPECIAL_ZOOM_LUIGI_FIRST_FRAME) && (status_id == nFTRemixSpecialZoomStatusSuperJumpPunch);
        case REMIX_ID_GND:
            return (status_id == nFTRemixSpecialZoomStatusWarlockPunch) || (status_id == nFTRemixSpecialZoomStatusWarlockPunchAir) || (status_id == nFTRemixSpecialZoomStatusUTilt);
        case nFTKindPurin:
        case REMIX_ID_JPUFF:
        case REMIX_ID_EPUFF:
            return ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusJigglyRest, nFTRemixSpecialZoomStatusJigglyRestAir);
        case nFTKindNess:
        case REMIX_ID_JNESS:
        case REMIX_ID_LUCAS:
            return ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusPKTA, nFTRemixSpecialZoomStatusPKTAAir);
        case REMIX_ID_ROY:
            return ftRemixSpecialZoomIsStatusOf(status_id, nFTRemixSpecialZoomStatusRoyDSPGroundStrongEnd, nFTRemixSpecialZoomStatusRoyDSPAirStrongEnd);
        case nFTKindKirby:
        case REMIX_ID_JKIRBY:
            return ftRemixSpecialZoomIsKirbyPunchStatus(status_id);
        default:
            return FALSE;
    }
}

/**
 * SpecialZoom.asm check_special_zoom: TRUE when the attacker (the fighter of the damage's player number, NULL when there is
 * none) is in a special move that triggers the Special Zoom.
 */
sb32 ftRemixSpecialZoomIsAttackerSpecialMove(GObj *attacker_gobj)
{
    if (attacker_gobj == NULL)
    {
        return FALSE;
    }
    return ftRemixSpecialZoomIsAttackerStatus(ftGetStruct(attacker_gobj));
}

#endif
