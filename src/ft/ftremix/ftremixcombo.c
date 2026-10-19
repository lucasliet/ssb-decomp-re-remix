/**
 * ftremixcombo.c - Smash Remix Improved Combo Meter and Tech Chase Combo Meter (src/Combo.asm improve_meter_).
 *
 * Vanilla resets the combo count of a fighter when its hitstun flag is clear. The Remix patch at 0x800E19CC keeps the
 * count while the fighter is in one of the actions below, or in knockback hitstun. The Tech Chase setting adds the
 * tech and down bounce actions to that list.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmremixtoggle.h>

/* Action.DownBounceD, DownBounceU, TechF, TechB and Tech (Action.asm), the prefix of the table with Tech Chase on */
static const u16 sFTRemixComboTechChaseActions[] = { 0x0043, 0x0044, 0x0049, 0x004A, 0x0051 };

/*
 * Combo.asm action_table: captures, falcon dive and the throws, Tornado, Barrel, Stun, shield break fall, stun landings
 * and starts, EggLayPulled, and the 0x00B4 literal of the asm. The asm table ends at a zero halfword.
 */
static const u16 sFTRemixComboActions[] =
{
    0x00AB, 0x00AC, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC,
    0x003C, 0x003D, 0x00A4, 0x009F, 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00B1
};

/** TRUE when the action is in the combo table; with Tech Chase on, the tech and down bounce actions count too. */
static sb32 ftRemixComboIsKeepingAction(u16 action, sb32 is_tech_chase)
{
    s32 i;

    if (is_tech_chase)
    {
        for (i = 0; i < ARRAY_COUNT(sFTRemixComboTechChaseActions); i++)
        {
            if (sFTRemixComboTechChaseActions[i] == action)
            {
                return TRUE;
            }
        }
    }
    for (i = 0; i < ARRAY_COUNT(sFTRemixComboActions); i++)
    {
        if (sFTRemixComboActions[i] == action)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * TRUE while the fighter is in knockback hitstun. The asm reads the low halfword of the hitstun counter (0x0B1A), which is
 * the low 16 bits of status_vars.common.damage.hitstun_tics on the big-endian N64 layout.
 */
static sb32 ftRemixComboIsKnockbackHitstun(FTStruct *fp)
{
    if (fp->damage_knockback_stack == 0.0F)
    {
        return FALSE;
    }
    return (fp->status_vars.common.damage.hitstun_tics & 0xFFFF) != 0;
}

/**
 * TRUE when the combo count of the fighter must not reset this frame, in place of the vanilla is_hitstun check. The asm
 * reads the current action as the low halfword of status_id (0x0026 of the player struct).
 */
sb32 ftRemixComboKeepsCount(FTStruct *fp)
{
    if (fp->is_hitstun)
    {
        return TRUE;
    }
    if (!gmRemixToggle(improved_combo_meter))
    {
        return FALSE;
    }
    if (ftRemixComboIsKeepingAction((u16)fp->status_id, gmRemixToggle(tech_chase_combo_meter) != 0))
    {
        return TRUE;
    }
    return ftRemixComboIsKnockbackHitstun(fp);
}

#endif
