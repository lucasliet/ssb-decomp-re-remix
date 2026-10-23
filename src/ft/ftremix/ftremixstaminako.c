/**
 * ftremixstaminako.c - Smash Remix Stamina KO and damage cap (Stamina.asm stamina_main_, stamina_ko_ and
 * stamina_percent_prevent).
 *
 * While the Stamina rule is active a fighter whose percent passes the HP (gmRemixStaminaGetTotalHP) is KO'd once.
 * The stored percent is completed up to HP + 1 on that hit and a defeated fighter stores no more damage.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftcommon/ftcommonfunctions.h"
#include "ft/ftparam.h"
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixstamina.h"

/* Stamina.asm stamina_ko_: "Custom Red Flicker id (hard coded)", a Remix colanim id past the vanilla GMColAnimKind list */
#define FTREMIX_STAMINA_KO_RED_FLICKER_COLANIM 0x66

/**
 * Stamina.asm stamina_ko_: KO sound, red flicker and the vanilla death setup (ftCommonDeadInitStatusVars), which runs
 * the score update and takes the stock. The asm erases the passive and yoshi armor too; that part is not ported yet.
 */
static void ftRemixStaminaKO(FTStruct *fp)
{
    if (fp->attr->dead_fgm_ids[0] != nSYAudioFGMVoiceEnd)
    {
        ftCommonDeadAddDeadSFXSoundQueue(fp->attr->dead_fgm_ids[0]);
    }
    ftParamCheckSetFighterColAnimID(fp->fighter_gobj, FTREMIX_STAMINA_KO_RED_FLICKER_COLANIM, 0);
    ftCommonDeadInitStatusVars(fp->fighter_gobj);
}

/**
 * Percent that `damage` adds to `fp->percent_damage` while the Stamina rule is active.
 *
 * Returns `damage` unchanged when the rule is off. A defeated fighter adds nothing. A hit that passes the HP KOs the
 * fighter once and adds only the amount that reaches HP + 1. Outside Stamina the caller's vanilla value is kept.
 */
s32 ftRemixStaminaStoredDamage(FTStruct *fp, s32 damage)
{
    s32 total_hp;

    if (!gmRemixStaminaIsActive())
    {
        return damage;
    }
    if (gmRemixStaminaIsDefeated(fp->player))
    {
        return 0;
    }
    total_hp = gmRemixStaminaGetTotalHP();

    if (fp->percent_damage + damage <= total_hp)
    {
        return damage;
    }
    gmRemixStaminaSetDefeated(fp->player);
    ftRemixStaminaKO(fp);

    return total_hp + 1 - fp->percent_damage;
}

#endif
