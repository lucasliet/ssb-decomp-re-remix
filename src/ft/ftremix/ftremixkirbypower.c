/**
 * ftremixkirbypower.c - Smash Remix Kirby power flags (src/KirbyHats.asm: magic_hat_on, taunt_loses_power,
 * set_absorbed_, set_absorbed_hat_, hat_loss_prevent_, kirby_power_loss_prevent_).
 *
 * Both tables are per port. The magic hat is a power with no moves (NONE) that a Kirby can hold and pass on when
 * inhaled. taunt_loses_power says whether a Kirby loses its power when hit or when it taunts; the Remix menu turns it off
 * and the power then stays. Defaults match the game without the menu: no magic hat, power is lost.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "remix/remix_roster_data.h"

#define FTREMIX_KIRBYPOWER_PORTS 4

static sb32 sFTRemixKirbyMagicHatOn[FTREMIX_KIRBYPOWER_PORTS];
static sb32 sFTRemixKirbyTauntLosesPower[FTREMIX_KIRBYPOWER_PORTS] = { TRUE, TRUE, TRUE, TRUE };

static sb32 ftRemixKirbyPowerIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == nFTKindNKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

static sb32 ftRemixKirbyPowerHasPort(FTStruct *fp)
{
    return (fp->player >= 0) && (fp->player < FTREMIX_KIRBYPOWER_PORTS);
}

void ftRemixKirbyPowerSetTauntLosesPower(s32 player, sb32 taunt_loses_power)
{
    if ((player >= 0) && (player < FTREMIX_KIRBYPOWER_PORTS))
    {
        sFTRemixKirbyTauntLosesPower[player] = taunt_loses_power;
    }
}

void ftRemixKirbyPowerReset(void)
{
    s32 player;

    for (player = 0; player < FTREMIX_KIRBYPOWER_PORTS; player++)
    {
        sFTRemixKirbyMagicHatOn[player] = FALSE;
    }
}

/* set_absorbed_ (0x8014C034): a Kirby whose power is taken loses its magic hat and goes back to losing its power. */
void ftRemixKirbyPowerStolen(FTStruct *fp)
{
    if (ftRemixKirbyPowerHasPort(fp) == FALSE)
    {
        return;
    }
    sFTRemixKirbyMagicHatOn[fp->player] = FALSE;
    sFTRemixKirbyTauntLosesPower[fp->player] = TRUE;
}

/* set_absorbed_hat_ (0x80162110): inhaling a Kirby that holds the magic hat gives the magic power. */
s32 ftRemixKirbyStolenPowerID(FTStruct *victim_fp, s32 copy_id)
{
    if (ftRemixKirbyPowerIsKirby(victim_fp) && ftRemixKirbyPowerHasPort(victim_fp) && (sFTRemixKirbyMagicHatOn[victim_fp->player] != FALSE))
    {
        return REMIX_ID_NONE;
    }
    return copy_id;
}

/* set_absorbed_hat_._update_magic_hat_on (0x80161F3C): copying the magic power turns the magic hat on. */
void ftRemixKirbyUpdateMagicHat(FTStruct *fp, s32 copy_id)
{
    if ((copy_id == REMIX_ID_NONE) && ftRemixKirbyPowerHasPort(fp))
    {
        sFTRemixKirbyMagicHatOn[fp->player] = TRUE;
    }
}

/* hat_loss_prevent_ (0x801635F4) and kirby_power_loss_prevent_ (0x80163620): with taunt_loses_power off the power stays;
 * otherwise the magic hat goes with it. Returns TRUE when the power must be kept. */
sb32 ftRemixKirbyLoseCopyPrevented(FTStruct *fp)
{
    if (ftRemixKirbyPowerHasPort(fp) == FALSE)
    {
        return FALSE;
    }
    if (sFTRemixKirbyTauntLosesPower[fp->player] == FALSE)
    {
        return TRUE;
    }
    sFTRemixKirbyMagicHatOn[fp->player] = FALSE;

    return FALSE;
}

#endif
