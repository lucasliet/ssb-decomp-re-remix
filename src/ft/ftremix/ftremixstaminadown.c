/**
 * ftremixstaminadown.c - Smash Remix Stamina guards for the down states and the ledge grab (Stamina.asm
 * stamina_down_wait_down, stamina_down_wait_input, stamina_down_bounce_input and stamina_ledge).
 *
 * A fighter out of HP cannot leave down wait, cannot stand up from it, and cannot grab a ledge. Down bounce goes
 * straight to down wait, the way the asm forces it.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"

/**
 * TRUE when the fighter is out of HP in a Stamina match, so its down wait and down bounce cannot take inputs or stand up.
 */
sb32 ftRemixStaminaDownBlocked(GObj *fighter_gobj)
{
    return ftRemixStaminaIsOutOfHP(ftGetStruct(fighter_gobj));
}

/**
 * TRUE when the fighter is out of HP in a Stamina match, so it cannot grab a ledge.
 */
sb32 ftRemixStaminaLedgeBlocked(FTStruct *fp)
{
    return ftRemixStaminaIsOutOfHP(fp);
}

#endif
