/**
 * ftremixpianoshared.c - Smash Remix PianoSpecial.asm vanilla patches (scope Piano leftovers): the command grab
 * action of the captured fighter (command_grab_action_fix_, 0x8014C8FC) and Mad Piano's absorb healing and bonus
 * ammunition (absorb_behaviour_, 0x800E33AC). Each helper keeps the vanilla value for every other fighter.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixfields.h"
#include "ftremixhooks.h"
#include "ftremixnessrules.h"
#include "fighter_registry.h"

#define PIANO_ABSORB_BONUS_AMMO_MAX 5.0F            /* 0x40A00000, absorb_behaviour_ bonus_ammo maximum */
#define PIANO_ABSORB_HEAL_PER_DAMAGE 0.75F          /* 0x3F400000, absorb_behaviour_ healing multiplier */
#define PIANO_ABSORB_AMMO_PER_DAMAGE (1.0F / 3.0F)  /* 0x3EAAAAAB, absorb_behaviour_: one projectile per 3% */
#define PIANO_ABSORB_BONUS_AMMO_N64_OFFSET 0x0ADC   /* passive_vars word, absorb_behaviour_ `lwc1 f10, 0x0ADC(s0)` */

enum
{
    nFTRemixCommandGrabActionEggLayPulled = 0xB1,   /* Action.EggLayPulled, Action.asm line 182 */
    nFTRemixCommandGrabActionThrownDK = 0xB8,       /* Action.ThrownDK, Action.asm line 189 */
    nFTRemixCommandGrabActionThrown2 = 0xBA         /* Action.Thrown2, Action.asm line 191 */
};

/* command_grab_action_fix_: Kirby is a Marina grabber only while it copies Marina. */
static sb32 ftRemixCommandGrabIsKirbyCopyingMarina(FTStruct *grabber_fp)
{
    if ((grabber_fp->fkind != REMIX_ID_KIRBY) && (grabber_fp->fkind != REMIX_ID_JKIRBY))
    {
        return FALSE;
    }
    return (grabber_fp->passive_vars.kirby.copy_id == REMIX_ID_MARINA);
}

/* command_grab_action_fix_ (0x8014C8FC): the action the captured fighter enters when grabbed by grabber_gobj. */
s32 ftRemixCommandGrabCaptureAction(GObj *grabber_gobj)
{
    FTStruct *grabber_fp = ftGetStruct(grabber_gobj);

    if (grabber_fp->fkind == REMIX_ID_PIANO)
    {
        return nFTRemixCommandGrabActionThrownDK;
    }
    if ((grabber_fp->fkind == REMIX_ID_DRAGONKING) || (grabber_fp->fkind == REMIX_ID_MARINA))
    {
        return nFTRemixCommandGrabActionThrown2;
    }
    if (ftRemixCommandGrabIsKirbyCopyingMarina(grabber_fp))
    {
        return nFTRemixCommandGrabActionThrown2;
    }
    return nFTRemixCommandGrabActionEggLayPulled;
}

/* absorb_behaviour_ (0x800E33AC) and Lucas psi_magnet_multiplier_ (0x800E33B4): the healing for absorbed damage. Mad Piano
 * heals 0.75x the damage, Lucas 2.5x, others 2x. */
s32 ftRemixAbsorbHealAmount(FTStruct *fp, s32 damage)
{
    if (fp->fkind == REMIX_ID_PIANO)
    {
        return (s32)(damage * PIANO_ABSORB_HEAL_PER_DAMAGE);
    }
    return (s32)(damage * ftRemixNessRulesPsiMagnetHealMultiplier(fp->fkind));
}

/* absorb_behaviour_ (0x800E33AC): Mad Piano gains one bonus projectile per 3% absorbed, up to 5.0. */
void ftRemixAbsorbPianoBonusAmmo(FTStruct *fp, s32 damage)
{
    f32 *bonus_ammo;

    if (fp->fkind != REMIX_ID_PIANO)
    {
        return;
    }
    bonus_ammo = ftRemixFighterFloat(fp, PIANO_ABSORB_BONUS_AMMO_N64_OFFSET);
    *bonus_ammo += damage * PIANO_ABSORB_AMMO_PER_DAMAGE;

    if (*bonus_ammo >= PIANO_ABSORB_BONUS_AMMO_MAX)
    {
        *bonus_ammo = PIANO_ABSORB_BONUS_AMMO_MAX;
    }
}

enum
{
    nFTRemixPianoGrabActionGrabWait = 0xA8          /* Action.GrabWait, Action.asm line 173 */
};

/* capture_action_fix_ (Piano.asm), custom_capture_dk_interrupt of Mad Piano. While Piano is in GrabWait, the grabbed
 * fighter is made invisible. The vanilla ThrownDK break-out is skipped for every Piano grab (returns TRUE). */
s32 ftRemixPianoCaptureDKInterrupt(FTStruct *grabber_fp)
{
    FTStruct *captured_fp;

    if (grabber_fp->status_id != nFTRemixPianoGrabActionGrabWait)
    {
        return TRUE;
    }
    captured_fp = ftGetStruct(grabber_fp->catch_gobj);
    captured_fp->is_invisible = TRUE;

    return TRUE;
}

#endif
