/**
 * sc1premixhrcrun.c - per-frame run of the Home-Run Contest, see sc1premixhrcrun.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <if/ifcommon.h>
#include <it/itmanager.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include <sys/objdef.h>
#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1premixhrc.h"
#include "sc/sc1pmode/sc1premixhrcrules.h"
#include "sc/sc1pmode/sc1premixhrcrun.h"
#include "sc/sc1pmode/sc1premixhrcsigns.h"
#include "sc/sc1pmode/sc1premixhrctravel.h"

#define SC1PREMIX_HRC_HUD_SLOT 29
#define SC1PREMIX_HRC_HUD_COLOR 0xFFFFFFFF
#define SC1PREMIX_HRC_FEET_TENTHS 10
#define SC1PREMIX_HRC_PLATFORM_LINE_ID 0
#define SC1PREMIX_HRC_COMPLETE_FGM 0x1CB
#define SC1PREMIX_HRC_BAT_SPAWN_X (-22542.0F)
#define SC1PREMIX_HRC_BAT_SPAWN_Y 256.0F

/* Action.asm ThrownDK and DK.Cargo; Lanky stands as an Orangstand from DSPGBegin on (HRC.asm _check_sandbag_moving). */
#define SC1PREMIX_HRC_STATUS_THROWN_DK 0xB8
#define SC1PREMIX_HRC_STATUS_DK_CARGO 0xEB
#define SC1PREMIX_HRC_STATUS_LANKY_ORANGSTAND 0xEB

/* Specials that need the controls on to let go (HRC.asm _disable_inputs): Conker Catapult Charge, Bowser Fire Breath and Kirby Inhale. */
#define SC1PREMIX_HRC_STATUS_CONKER_CATAPULT_CHARGE 0xF7
#define SC1PREMIX_HRC_STATUS_BOWSER_FIRE_BREATH 0xE4
#define SC1PREMIX_HRC_STATUS_KIRBY_INHALE 0x10E

static SC1PRemixHrcState sSC1PRemixHrcState;
static sb32 sSC1PRemixHrcIsEnded;
static sb32 sSC1PRemixHrcIsBatSpawned;
static sb32 sSC1PRemixHrcIsCompleting;

static const s32 sSC1PRemixHrcRestingStatuses[] =
{
    nFTCommonStatusWait,
    nFTCommonStatusOttotto,
    nFTCommonStatusWalkSlow,
    nFTCommonStatusWalkMiddle,
    nFTCommonStatusWalkFast,
    nFTCommonStatusRun,
    nFTCommonStatusDownWaitD,
    nFTCommonStatusDownWaitU
};

static GObj* sc1PRemixHrcRunGetSandbag(void)
{
    s32 player;

    for (player = 0; player < GMCOMMON_PLAYERS_MAX; player++)
    {
        if ((gSCManagerBattleState->players[player].fkind == REMIX_ID_SANDBAG) && (gSCManagerBattleState->players[player].fighter_gobj != NULL))
        {
            return gSCManagerBattleState->players[player].fighter_gobj;
        }
    }
    return NULL;
}

static GObj* sc1PRemixHrcRunGetHuman(void)
{
    return gSCManagerBattleState->players[gSCManagerSceneData.player].fighter_gobj;
}

static sb32 sc1PRemixHrcRunIsResting(s32 status_id)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sSC1PRemixHrcRestingStatuses); i++)
    {
        if (sSC1PRemixHrcRestingStatuses[i] == status_id)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static sb32 sc1PRemixHrcRunIsHumanStuckInHold(const FTStruct *sandbag_fp, const FTStruct *human_fp)
{
    if ((sandbag_fp->status_id == SC1PREMIX_HRC_STATUS_THROWN_DK) && (human_fp->status_id == SC1PREMIX_HRC_STATUS_DK_CARGO))
    {
        return TRUE;
    }
    return ((human_fp->fkind == REMIX_ID_LANKY) && (human_fp->status_id >= SC1PREMIX_HRC_STATUS_LANKY_ORANGSTAND)) ? TRUE : FALSE;
}

static void sc1PRemixHrcRunFillAttempt(SC1PRemixHrcAttempt *attempt, const SC1PRemixHrcMove *move, const FTStruct *sandbag_fp, const FTStruct *human_fp)
{
    attempt->time_passed = (s32)gSCManagerBattleState->time_passed;
    attempt->is_past_start = move->is_past_start;
    attempt->is_sandbag_aerial = (sandbag_fp->ga == nMPKineticsAir) ? TRUE : FALSE;
    attempt->is_over_platform = (sandbag_fp->coll_data.floor_line_id == SC1PREMIX_HRC_PLATFORM_LINE_ID) ? TRUE : FALSE;
    attempt->is_sandbag_moving = (sandbag_fp->coll_data.pos_diff.x != 0.0F) ? TRUE : FALSE;
    attempt->is_human_stuck_in_hold = sc1PRemixHrcRunIsHumanStuckInHold(sandbag_fp, human_fp);
    attempt->is_human_resting = sc1PRemixHrcRunIsResting(human_fp->status_id);
}

static void sc1PRemixHrcRunFollowSandbag(GObj *sandbag_gobj, GObj *human_gobj, SC1PRemixHrcMove *move)
{
    DObj *top_joint = DObjGetStruct(sandbag_gobj);

    sc1PRemixHrcTrack(&sSC1PRemixHrcState, top_joint->translate.vec.f.x, gmRemixCameraGetHrcX(), move);

    gmRemixCameraSetHrcX(move->camera_x);
    top_joint->translate.vec.f.x += move->sandbag_shift;

    if (move->sandbag_shift != 0.0F)
    {
        sc1PRemixHrcSignsAdvance();
    }
    sc1PRemixHrcTravelUpdate(human_gobj, move->camera_x, sSC1PRemixHrcState.warp_distance);
}

/** Drops the Home-Run Bat on the left side of the platform (SinglePlayerModes.asm _hrc, Hazards.add_item_ with BAT_SPAWN). */
static void sc1PRemixHrcRunSpawnBat(void)
{
    Vec3f pos = { SC1PREMIX_HRC_BAT_SPAWN_X, SC1PREMIX_HRC_BAT_SPAWN_Y, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };

    sSC1PRemixHrcIsBatSpawned = TRUE;
    itManagerMakeItemSetupCommon(NULL, nITKindBat, &pos, &vel, ITEM_FLAG_PARENT_GROUND);
}

static void sc1PRemixHrcRunEndAttempt(sb32 is_complete)
{
    sSC1PRemixHrcIsCompleting = is_complete;

    if (sSC1PRemixHrcState.complete_timer-- > 0)
    {
        return;
    }
    sSC1PRemixHrcIsEnded = TRUE;
    port_log("hrc: attempt ended, complete=%d distance=%d\n", (int)is_complete, (int)sSC1PRemixHrcState.distance);

    if (is_complete != FALSE)
    {
        ifCommonAnnounceCompleteInitInterface(SC1PREMIX_HRC_COMPLETE_FGM);
        return;
    }
    ifCommonAnnounceEndMessage();
}

static sb32 sc1PRemixHrcRunNeedsInputsToRelease(const FTStruct *fp)
{
    switch (fp->fkind)
    {
    case REMIX_ID_CONKER:
        return (fp->status_id == SC1PREMIX_HRC_STATUS_CONKER_CATAPULT_CHARGE) ? TRUE : FALSE;

    case REMIX_ID_BOWSER:
        return (fp->status_id == SC1PREMIX_HRC_STATUS_BOWSER_FIRE_BREATH) ? TRUE : FALSE;

    case REMIX_ID_KIRBY:
    case REMIX_ID_JKIRBY:
        return (fp->status_id == SC1PREMIX_HRC_STATUS_KIRBY_INHALE) ? TRUE : FALSE;

    default:
        return FALSE;
    }
}

/** Cuts the inputs of the player once the attempt is over; the specials above keep the controls on but start from clear inputs. */
static void sc1PRemixHrcRunCutInputs(GObj *human_gobj)
{
    ftParamLockPlayerControl(human_gobj);

    if (sc1PRemixHrcRunNeedsInputsToRelease(ftGetStruct(human_gobj)) != FALSE)
    {
        ftParamUnlockPlayerControl(human_gobj);
    }
}

static void sc1PRemixHrcRunDrawHud(void)
{
    SC1PRemixHrcCounterLayout layout;
    s32 distance = sSC1PRemixHrcState.distance;
    s32 complete_tics = (sSC1PRemixHrcIsCompleting != FALSE) ? (SC1PREMIX_HRC_COMPLETE_TIMER_START - sSC1PRemixHrcState.complete_timer) : 0;

    sc1PRemixHrcCounterAt(complete_tics, &layout);
    remix_hud_text_putf(SC1PREMIX_HRC_HUD_SLOT, layout.x, layout.y, SC1PREMIX_HRC_HUD_COLOR, layout.height,
                        "%d.%d FT.", (int)(distance / SC1PREMIX_HRC_FEET_TENTHS), (int)(distance % SC1PREMIX_HRC_FEET_TENTHS));
}

void sc1PRemixHrcRunStart(void)
{
    if (sc1PRemixHrcIsActive() == FALSE)
    {
        return;
    }
    sc1PRemixHrcStateReset(&sSC1PRemixHrcState);
    sSC1PRemixHrcIsEnded = FALSE;
    sSC1PRemixHrcIsBatSpawned = FALSE;
    sSC1PRemixHrcIsCompleting = FALSE;
    sc1PRemixHrcTravelReset();
    gmRemixCameraSetHrcX(SC1PREMIX_HRC_DEFAULT_CAMERA_X);
}

void sc1PRemixHrcRunUpdate(void)
{
    GObj *sandbag_gobj;
    GObj *human_gobj;
    SC1PRemixHrcMove move;
    SC1PRemixHrcAttempt attempt;
    s32 verdict;

    if (sc1PRemixHrcIsActive() == FALSE)
    {
        return;
    }
    sc1PRemixHrcRunDrawHud();

    sandbag_gobj = sc1PRemixHrcRunGetSandbag();
    human_gobj = sc1PRemixHrcRunGetHuman();

    if ((sandbag_gobj == NULL) || (human_gobj == NULL))
    {
        return;
    }
    if (sSC1PRemixHrcIsBatSpawned == FALSE)
    {
        sc1PRemixHrcRunSpawnBat();
        sc1PRemixHrcSignsInit();
    }
    if (gSCManagerBattleState->game_status != nSCBattleGameStatusGo)
    {
        return;
    }
    sc1PRemixHrcRunFollowSandbag(sandbag_gobj, human_gobj, &move);

    if (sSC1PRemixHrcIsEnded != FALSE)
    {
        return;
    }
    sc1PRemixHrcRunFillAttempt(&attempt, &move, ftGetStruct(sandbag_gobj), ftGetStruct(human_gobj));
    verdict = sc1PRemixHrcAttemptVerdict(&attempt);

    if (verdict == SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS)
    {
        sc1PRemixHrcRunCutInputs(human_gobj);
    }
    if (verdict == SC1PREMIX_HRC_VERDICT_END)
    {
        sc1PRemixHrcRunEndAttempt(move.is_past_start);
    }
}

s32 sc1PRemixHrcRunGetDistance(void)
{
    return sSC1PRemixHrcState.distance;
}

#endif
