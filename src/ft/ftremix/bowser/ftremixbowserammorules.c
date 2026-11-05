/**
 * ftremixbowserammorules.c - pure rules of the fire breath ammo, see ftremixbowserammorules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/bowser/ftremixbowserammorules.h"

#define FTREMIX_BOWSER_AMMO_TIMER_SHIFT 16

unsigned int ftRemixBowserAmmoSpawnWord(void)
{
    return FTREMIX_BOWSER_AMMO_MAX;
}

unsigned int ftRemixBowserAmmoRefillWord(unsigned int word)
{
    return (word & ~FTREMIX_BOWSER_AMMO_HALFWORD_MASK) | FTREMIX_BOWSER_AMMO_MAX;
}

unsigned int ftRemixBowserAmmoRechargeWord(unsigned int word, int is_firing)
{
    unsigned int timer = ((word >> FTREMIX_BOWSER_AMMO_TIMER_SHIFT) + 1) & FTREMIX_BOWSER_AMMO_HALFWORD_MASK;
    unsigned int ammo = word & FTREMIX_BOWSER_AMMO_HALFWORD_MASK;

    if (timer != FTREMIX_BOWSER_AMMO_RECHARGE_FRAMES)
    {
        return (timer << FTREMIX_BOWSER_AMMO_TIMER_SHIFT) | ammo;
    }
    if ((is_firing != 0) || (ammo == FTREMIX_BOWSER_AMMO_MAX))
    {
        return ammo;
    }
    return (ammo + FTREMIX_BOWSER_AMMO_RECHARGE_STEP) & FTREMIX_BOWSER_AMMO_HALFWORD_MASK;
}

#endif
