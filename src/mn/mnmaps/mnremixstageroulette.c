/**
 * mnremixstageroulette.c - pure timer of the Random Stage Roulette, see mnremixstageroulette.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnmaps/mnremixstageroulette.h"

#define MNREMIX_ROULETTE_SELECTING (-1)

void mnRemixRouletteReset(MNRemixRoulette *roulette)
{
    roulette->timer = 0;
}

void mnRemixRouletteStart(MNRemixRoulette *roulette)
{
    roulette->timer = MNREMIX_ROULETTE_DELAY;
}

int mnRemixRouletteIsActive(const MNRemixRoulette *roulette)
{
    return (roulette->timer != 0) ? 1 : 0;
}

void mnRemixRouletteStop(MNRemixRoulette *roulette)
{
    roulette->timer = 0;
}

void mnRemixRouletteMarkSelect(MNRemixRoulette *roulette)
{
    roulette->timer = MNREMIX_ROULETTE_SELECTING;
}

int mnRemixRouletteTakeSelect(MNRemixRoulette *roulette)
{
    if (roulette->timer >= 0)
    {
        return 0;
    }
    roulette->timer = 0;

    return 1;
}

int mnRemixRouletteTick(MNRemixRoulette *roulette)
{
    int has_waited = (roulette->timer >= MNREMIX_ROULETTE_DELAY) ? 1 : 0;

    roulette->timer = (has_waited != 0) ? 1 : (roulette->timer + 1);

    return has_waited;
}

int mnRemixRouletteCountSlots(unsigned int slot_mask)
{
    int count = 0;

    for (; slot_mask != 0; slot_mask >>= 1)
    {
        count += (int)(slot_mask & 1U);
    }
    return count;
}

int mnRemixRouletteNthSlot(unsigned int slot_mask, int index)
{
    int slot;

    for (slot = 0; slot < MNREMIX_ROULETTE_SLOT_MAX; slot++)
    {
        if (((slot_mask >> slot) & 1U) == 0)
        {
            continue;
        }
        if (index == 0)
        {
            return slot;
        }
        index--;
    }
    return -1;
}

#endif
