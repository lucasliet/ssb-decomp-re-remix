/**
 * gmremixsmashketballboard.c - the stage parts of Remix Smashketball: the baskets joint and the scoreboard digits
 * (Smashketball.asm setup_ _init_scoreboard, scoreboard_main_, Hazards.asm _nba_jam).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <if/interface.h>
#include <mp/mpdef.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include "gm/gmremixsmashketball.h"

#define GMREMIX_SMASHKETBALL_BASKETS_Y 1335.15F /* Hazards.asm: baskets y in Smashketball (0x44A6E4C4) */
#define GMREMIX_SMASHKETBALL_MAP_LINK 2
#define GMREMIX_SMASHKETBALL_MAP_SCOREBOARD 0
#define GMREMIX_SMASHKETBALL_MAP_NETS 1
#define GMREMIX_SMASHKETBALL_DASH 10
#define GMREMIX_SMASHKETBALL_DIGITS 8
#define GMREMIX_SMASHKETBALL_SCORE_MAX 99

enum
{
    nGMRemixBoardDigitMinutes1,
    nGMRemixBoardDigitMinutes2,
    nGMRemixBoardDigitHome1,
    nGMRemixBoardDigitHome2,
    nGMRemixBoardDigitSeconds1,
    nGMRemixBoardDigitSeconds2,
    nGMRemixBoardDigitAway1,
    nGMRemixBoardDigitAway2
};

static MObj *sGMRemixBoardDigits[GMREMIX_SMASHKETBALL_DIGITS];
static DObj *sGMRemixBoardBaskets;

/* The N-th object of the stage map link: the stage geometry objects are created in file order. */
static GObj* gmRemixSmashketballGetMapObject(s32 index)
{
    GObj *gobj = gGCCommonLinks[GMREMIX_SMASHKETBALL_MAP_LINK];

    while ((gobj != NULL) && (index > 0))
    {
        gobj = gobj->link_next;
        index--;
    }
    return gobj;
}

static DObj* gmRemixSmashketballGetMapJoint(s32 map_index, s32 depth)
{
    GObj *gobj = gmRemixSmashketballGetMapObject(map_index);
    DObj *dobj;
    s32 i;

    if ((gobj == NULL) || (gobj->obj == NULL))
    {
        return NULL;
    }
    dobj = DObjGetStruct(gobj)->child;

    for (i = 0; (i < depth) && (dobj != NULL); i++)
    {
        dobj = dobj->sib_next;
    }
    return dobj;
}

f32 gmRemixSmashketballMoveBaskets(void)
{
    sGMRemixBoardBaskets = gmRemixSmashketballGetMapJoint(GMREMIX_SMASHKETBALL_MAP_NETS, 2);

    if (sGMRemixBoardBaskets == NULL)
    {
        port_log("smashketball: baskets joint not found\n");
        return 0.0F;
    }
    sGMRemixBoardBaskets->translate.vec.f.y = GMREMIX_SMASHKETBALL_BASKETS_Y;

    if (sGMRemixBoardBaskets->user_data.s == nMPYakumonoStatusNone)
    {
        sGMRemixBoardBaskets->user_data.s = nMPYakumonoStatusOn;
    }
    return GMREMIX_SMASHKETBALL_BASKETS_Y;
}

f32 gmRemixSmashketballGetBasketsY(void)
{
    return (sGMRemixBoardBaskets != NULL) ? sGMRemixBoardBaskets->translate.vec.f.y : 0.0F;
}

void gmRemixSmashketballBoardSetup(void)
{
    DObj *scoreboard = gmRemixSmashketballGetMapJoint(GMREMIX_SMASHKETBALL_MAP_SCOREBOARD, 1);
    MObj *mobj;
    s32 i;

    for (i = 0; i < GMREMIX_SMASHKETBALL_DIGITS; i++)
    {
        sGMRemixBoardDigits[i] = NULL;
    }
    if (scoreboard == NULL)
    {
        port_log("smashketball: scoreboard joint not found\n");
        return;
    }
    mobj = scoreboard->mobj;

    for (i = 0; (i < GMREMIX_SMASHKETBALL_DIGITS) && (mobj != NULL); i++)
    {
        sGMRemixBoardDigits[i] = mobj;
        mobj = mobj->next;
    }
}

static void gmRemixSmashketballBoardSetDigit(s32 digit, s32 value)
{
    if (sGMRemixBoardDigits[digit] != NULL)
    {
        sGMRemixBoardDigits[digit]->texture_id_curr = value;
    }
}

static void gmRemixSmashketballBoardSetTwoDigits(s32 first, s32 second, s32 value)
{
    if (value > GMREMIX_SMASHKETBALL_SCORE_MAX)
    {
        value = GMREMIX_SMASHKETBALL_SCORE_MAX;
    }
    gmRemixSmashketballBoardSetDigit(first, value / 10);
    gmRemixSmashketballBoardSetDigit(second, value % 10);
}

static void gmRemixSmashketballBoardUpdateTime(void)
{
    s32 seconds;

    if (gSCManagerBattleState->time_limit == SCBATTLE_TIMELIMIT_INFINITE)
    {
        gmRemixSmashketballBoardSetDigit(nGMRemixBoardDigitMinutes1, GMREMIX_SMASHKETBALL_DASH);
        gmRemixSmashketballBoardSetDigit(nGMRemixBoardDigitMinutes2, GMREMIX_SMASHKETBALL_DASH);
        gmRemixSmashketballBoardSetDigit(nGMRemixBoardDigitSeconds1, GMREMIX_SMASHKETBALL_DASH);
        gmRemixSmashketballBoardSetDigit(nGMRemixBoardDigitSeconds2, GMREMIX_SMASHKETBALL_DASH);
        return;
    }
    seconds = (gSCManagerBattleState->time_remain + 59) / 60;

    gmRemixSmashketballBoardSetTwoDigits(nGMRemixBoardDigitMinutes1, nGMRemixBoardDigitMinutes2, seconds / 60);
    gmRemixSmashketballBoardSetTwoDigits(nGMRemixBoardDigitSeconds1, nGMRemixBoardDigitSeconds2, seconds % 60);
}

void gmRemixSmashketballBoardUpdate(s32 home_score, s32 away_score)
{
    gmRemixSmashketballBoardUpdateTime();
    gmRemixSmashketballBoardSetTwoDigits(nGMRemixBoardDigitHome1, nGMRemixBoardDigitHome2, home_score);
    gmRemixSmashketballBoardSetTwoDigits(nGMRemixBoardDigitAway1, nGMRemixBoardDigitAway2, away_score);
}

#endif
