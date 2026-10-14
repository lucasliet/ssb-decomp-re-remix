/**
 * grremixturnpike.c - Toad's Turnpike light angle cycle. See grremixturnpike.h.
 */

#ifdef PORT

#include "grremixturnpike.h"
#include "grremixhazards.h"
#include <gr/ground.h>
#include <ft/fighter.h>

#define GRREMIX_TURNPIKE_CYCLE_TIME 720
#define GRREMIX_TURNPIKE_STATES_NUM 6
#define GRREMIX_TURNPIKE_LIGHT_Y 30.0F
#define GRREMIX_TURNPIKE_PROC_PRIORITY 4

typedef struct GRRemixTurnpikeEntry
{
    f32 start_angle;
    f32 speed;
    s32 direction;
    s32 end_time;
} GRRemixTurnpikeEntry;

typedef struct GRRemixTurnpikeState
{
    s32 timer;
    s32 state;
    s32 offset;
} GRRemixTurnpikeState;

static const GRRemixTurnpikeEntry sGRRemixTurnpikeTimeline[GRREMIX_TURNPIKE_STATES_NUM] =
{
    { -90.0F, 1.0F, -1, 180 },
    { 90.0F, 0.0F, -1, 225 },
    { 90.0F, 1.25F, -1, 360 },
    { -90.0F, 1.0F, 1, 540 },
    { 90.0F, 0.0F, 1, 585 },
    { 90.0F, 1.25F, 1, 0 },
};

static GRRemixTurnpikeState sGRRemixTurnpike;

/** Advances the cycle timer and wraps it to 0 at the end of the cycle (asm: _check_table). */
static void grRemixTurnpikeAdvanceTimer(void)
{
    sGRRemixTurnpike.timer++;
    if (sGRRemixTurnpike.timer == GRREMIX_TURNPIKE_CYCLE_TIME)
    {
        sGRRemixTurnpike.timer = 0;
    }
}

/** Returns the timeline entry of the current state. */
static const GRRemixTurnpikeEntry *grRemixTurnpikeCurrentEntry(void)
{
    return &sGRRemixTurnpikeTimeline[sGRRemixTurnpike.state];
}

/** Moves to the next timeline entry and resets the offset; the state wraps after the last entry (asm: _apply_state). */
static void grRemixTurnpikeNextState(void)
{
    sGRRemixTurnpike.state++;
    if (sGRRemixTurnpike.state == GRREMIX_TURNPIKE_STATES_NUM)
    {
        sGRRemixTurnpike.state = 0;
    }
    sGRRemixTurnpike.offset = 0;
}

/** Writes the light angle of every fighter, mirrored for fighters facing left (asm: _loop). */
static void grRemixTurnpikeApplyLight(f32 angle)
{
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

    while (fighter_gobj != NULL)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        fp->colanim.is_use_light = TRUE;
        fp->colanim.light_angle_x = (fp->lr == 1) ? angle : -angle;
        fp->colanim.light_angle_y = GRREMIX_TURNPIKE_LIGHT_Y;
        fighter_gobj = fighter_gobj->link_next;
    }
}

/** Per-frame update of the turnpike lights; the asm routine is toads_turnpike_main (asm: _continue). */
static void grRemixTurnpikeProcUpdate(GObj *turnpike_gobj)
{
    const GRRemixTurnpikeEntry *entry;
    f32 angle;

    grRemixTurnpikeAdvanceTimer();
    if (grRemixTurnpikeCurrentEntry()->end_time == sGRRemixTurnpike.timer)
    {
        grRemixTurnpikeNextState();
    }

    entry = grRemixTurnpikeCurrentEntry();
    sGRRemixTurnpike.offset += entry->direction;
    angle = (f32) sGRRemixTurnpike.offset * entry->speed + entry->start_angle;
    grRemixTurnpikeApplyLight(angle);
}

/**
 * Starts the turnpike light cycle. The asm registers the routine only while
 * Toggles.entry_hazard_mode allows movement (asm: _movement_enabled).
 */
void grRemixTurnpikeMakeGround(void)
{
    GObj *turnpike_gobj;

    sGRRemixTurnpike.timer = 0;
    sGRRemixTurnpike.state = 0;
    sGRRemixTurnpike.offset = 0;

    if (!grRemixMovementIsOn())
    {
        return;
    }

    turnpike_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(turnpike_gobj, grRemixTurnpikeProcUpdate, nGCProcessKindFunc, GRREMIX_TURNPIKE_PROC_PRIORITY);
}

#endif
