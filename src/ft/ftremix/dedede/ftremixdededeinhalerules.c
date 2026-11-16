/**
 * ftremixdededeinhalerules.c - pure rules of the action ids of the Dedede inhale, see ftremixdededeinhalerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/dedede/ftremixdededeinhalerules.h"

#define FTREMIX_DEDEDE_INHALE_VANILLA_FIRST 0x10D /* nFTKirbyStatusSpecialNStart */
#define FTREMIX_DEDEDE_INHALE_STATUSES 18
#define FTREMIX_DEDEDE_INHALE_NONE 0

typedef struct FTRemixDededeInhaleAction
{
    unsigned short dedede_status;
    unsigned short kirby_status;
} FTRemixDededeInhaleAction;

/* Index: vanilla id - 0x10D. Columns: Dedede.Action.NSP_*, Kirby.Action.DEDEDE_NSP_* (0xDC + index in KIRBY_action_array). */
static const FTRemixDededeInhaleAction sFTRemixDededeInhaleActions[FTREMIX_DEDEDE_INHALE_STATUSES] =
{
    { 0xE7, 0x16C }, /* SpecialNStart     -> NSP_BEGIN_GROUND */
    { 0xE8, 0x16D }, /* SpecialNLoop      -> NSP_LOOP_GROUND */
    { 0xEF, 0x173 }, /* SpecialNEnd       -> NSP_END_GROUND */
    { 0xEA, 0x16E }, /* SpecialNCatch     -> NSP_PULL_GROUND */
    { 0xEB, 0x16F }, /* SpecialNEat       -> NSP_SWALLOW_GROUND */
    { 0xED, 0x171 }, /* SpecialNThrow     -> NSP_SPIT_GROUND */
    { 0xEC, 0x170 }, /* SpecialNWait      -> NSP_IDLE_GROUND */
    { 0xEE, 0x172 }, /* SpecialNTurn      -> NSP_TURN_GROUND */
    { FTREMIX_DEDEDE_INHALE_NONE, FTREMIX_DEDEDE_INHALE_NONE }, /* SpecialNCopy */
    { 0xF0, 0x174 }, /* SpecialAirNStart  -> NSP_BEGIN_AIR */
    { 0xF1, 0x175 }, /* SpecialAirNLoop   -> NSP_LOOP_AIR */
    { 0xF7, 0x17B }, /* SpecialAirNEnd    -> NSP_END_AIR */
    { 0xF2, 0x176 }, /* SpecialAirNCatch  -> NSP_PULL_AIR */
    { 0xF3, 0x177 }, /* SpecialAirNEat    -> NSP_SWALLOW_AIR */
    { 0xF5, 0x179 }, /* SpecialAirNThrow  -> NSP_SPIT_AIR */
    { 0xF4, 0x178 }, /* SpecialAirNWait   -> NSP_FALL */
    { 0xF6, 0x17A }, /* SpecialAirNTurn   -> NSP_TURN_AIR */
    { FTREMIX_DEDEDE_INHALE_NONE, FTREMIX_DEDEDE_INHALE_NONE }  /* SpecialAirNCopy */
};

/* Row of a vanilla SpecialN status, or a null pointer for any other status. */
static const FTRemixDededeInhaleAction* ftRemixDededeInhaleFindAction(int status_id)
{
    int index = status_id - FTREMIX_DEDEDE_INHALE_VANILLA_FIRST;

    if ((index < 0) || (index >= FTREMIX_DEDEDE_INHALE_STATUSES))
    {
        return 0;
    }
    return &sFTRemixDededeInhaleActions[index];
}

int ftRemixDededeInhaleStatus(int status_id)
{
    const FTRemixDededeInhaleAction *action = ftRemixDededeInhaleFindAction(status_id);

    if ((action == 0) || (action->dedede_status == FTREMIX_DEDEDE_INHALE_NONE))
    {
        return status_id;
    }
    return action->dedede_status;
}

int ftRemixDededeInhaleKirbyStatus(int hat_id, int status_id)
{
    const FTRemixDededeInhaleAction *action = ftRemixDededeInhaleFindAction(status_id);

    if ((unsigned int)hat_id < FTREMIX_DEDEDE_INHALE_KIRBY_HAT_MIN)
    {
        return status_id;
    }
    if ((action == 0) || (action->kirby_status == FTREMIX_DEDEDE_INHALE_NONE))
    {
        return status_id;
    }
    return action->kirby_status;
}

#endif
