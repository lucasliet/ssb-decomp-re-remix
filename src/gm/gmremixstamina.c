/**
 * gmremixstamina.c - state of the Remix Stamina rule (Stamina.asm TOTAL_HP, STAMINA_MODE, percent_port).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixstamina.h"
#include "gm/gmremixstaminarules.h"

sb32 gGMRemixStaminaRule = FALSE;
s32 gGMRemixStaminaHP = GMREMIX_STAMINA_HP_DEFAULT;

static sb32 sGMRemixStaminaIsDefeated[GMCOMMON_PLAYERS_MAX];

sb32 gmRemixStaminaIsActive(void)
{
    /* The CSS preview fighter runs ftMainSetStatus before any battle scene set gSCManagerBattleState
     * (r13_md20_css crashed here with the Stamina rule on and the CSS opened from boot) */
    if ((gGMRemixStaminaRule == FALSE) || (gSCManagerBattleState == NULL))
    {
        return FALSE;
    }
    return (gSCManagerBattleState->game_rules == SCBATTLE_GAMERULE_TIMED_STOCK) ? TRUE : FALSE;
}

s32 gmRemixStaminaGetTotalHP(void)
{
    return gmRemixStaminaRulesTotalHP(gGMRemixStaminaHP);
}

sb32 gmRemixStaminaIsDefeated(s32 player)
{
    return sGMRemixStaminaIsDefeated[player];
}

void gmRemixStaminaSetDefeated(s32 player)
{
    sGMRemixStaminaIsDefeated[player] = TRUE;
}

void gmRemixStaminaResetDefeated(void)
{
    s32 i;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        sGMRemixStaminaIsDefeated[i] = FALSE;
    }
}

#endif
