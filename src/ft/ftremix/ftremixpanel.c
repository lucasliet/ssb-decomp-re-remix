/**
 * ftremixpanel.c - fighter-side readers of the Remix CSS panel options that need only a value at creation
 * (css/Damage.asm char_init_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmremixpanel.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "remix/remix_training.h"

#define FTREMIX_PANEL_DAMAGE_MAX 999

/**
 * Initial damage of a new fighter (asm Damage.char_init_): the panel value of its port for the VS, 1P and Bonus matches,
 * unless the fighter already starts with damage (Sudden Death). Training takes the menu percent (Reset Sets Percent) and the How to Play demo keeps the vanilla value.
 * Called from ftManagerInitFighter with the damage of the fighter description.
 */
s32 ftRemixPanelGetInitialDamage(FTStruct *fp, s32 desc_damage)
{
    s32 game_type;

    /* The 1P character select builds its preview fighter before any battle scene has set the battle state
     * (fresh boot without the attract demo); the N64 reads a fixed struct there, the port has NULL. */
    if ((desc_damage != 0) || (fp->pkind == nFTPlayerKindDemo) || (gSCManagerBattleState == NULL))
    {
        return desc_damage;
    }
    game_type = gSCManagerBattleState->game_type;
    if ((game_type == nSCBattleGameTypeTraining) && (fp->player < 4))
    {
        /* Training.asm init_character_: a fighter created for the match takes the menu percent when Reset Sets Percent is on. */
        return (remix_training_get(REMIX_TRAINING_RESET_SETS_PERCENT(fp->player)) != 0) ? remix_training_get(REMIX_TRAINING_PERCENT(fp->player)) : desc_damage;
    }
    if ((game_type != nSCBattleGameTypeRoyal) && (game_type != nSCBattleGameTypeBonus) && (game_type != nSCBattleGameType1PGame))
    {
        return desc_damage;
    }
    return gmRemixPanel(DAMAGE, fp->player) % (FTREMIX_PANEL_DAMAGE_MAX + 1);
}

#endif
