/**
 * ftremixstaminadead.c - Smash Remix Stamina death guards (Stamina.asm stamina_ko_prevent, stamina_ko_prevent_front,
 * stamina_fix_6 and stamina_fix_7).
 *
 * A fighter with stock -1 is out for the rest of a Stamina battle. A second death sends it to sleep without scoring
 * again, and a star KO on the last stock is stored as -1 in the fighter struct (the asm stores -2 and fixes only the fighter
 * struct at rebirth, stamina_fix_7; the match data keeps -2, as in the asm).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixstamina.h"

/* Stamina.asm stamina_fix_7: a star KO on the last stock leaves the stock at -2 before the fix */
#define FTREMIX_STAMINA_STAR_KO_STOCK -2
#define FTREMIX_STAMINA_OUT_OF_STOCK -1

/**
 * TRUE when the Stamina rule is active and the port has no stock left in the match data, so a new death must not score
 * (stamina_ko_prevent and stamina_ko_prevent_front read the match data stock, not the fighter struct).
 */
sb32 ftRemixStaminaIsOutOfStock(FTStruct *fp)
{
    if (gmRemixStaminaIsActive() == FALSE)
    {
        return FALSE;
    }
    return (gSCManagerBattleState->players[fp->player].stock_count == FTREMIX_STAMINA_OUT_OF_STOCK) ? TRUE : FALSE;
}

/**
 * TRUE when a blast-zone death in Stamina starts with the fighter already out of stock. The death procs check it on
 * their first frame (motion flag1 0), before the score takes the stock.
 */
sb32 ftRemixStaminaIsBlastRedeath(FTStruct *fp)
{
    if (fp->motion_vars.flags.flag1 != 0)
    {
        return FALSE;
    }
    return ftRemixStaminaIsOutOfStock(fp);
}

/**
 * Rebirth check in Stamina. Stores a star KO on the last stock as -1 in the fighter struct (stamina_fix_7, which does not
 * touch the match data) and returns TRUE when the fighter is out of stock, so the caller sends it to sleep and does not respawn it.
 */
sb32 ftRemixStaminaRebirthSleeps(FTStruct *fp)
{
    if (gmRemixStaminaIsActive() == FALSE)
    {
        return FALSE;
    }
    if (fp->stock_count == FTREMIX_STAMINA_STAR_KO_STOCK)
    {
        fp->stock_count = FTREMIX_STAMINA_OUT_OF_STOCK;
    }
    return (fp->stock_count == FTREMIX_STAMINA_OUT_OF_STOCK) ? TRUE : FALSE;
}

#endif
