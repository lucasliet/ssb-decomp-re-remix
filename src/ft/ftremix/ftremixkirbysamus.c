/**
 * ftremixkirbysamus.c - Dark Samus and Mewtwo branches of the Charge Shot Kirby copies from Samus (src/DSamus/DsamusSpecial.asm
 * DSamusNSP kirby patches, src/Mewtwo/MewtwoSpecial.asm kirby_shoot_velocity_patch_ and the Kirby branch of ball_position_fix_).
 * The Samus and Dark Samus themselves are in dsamus/ftremixdsamushooks.c and mewtwo/ftremixmewtwonsp.c.
 *
 * A Kirby with the Dark Samus power is told apart by the hat id byte (N64 0x0980), as the asm does. The ammo of the Dark Samus
 * hat is the Kirby charge level (N64 0x0AE0).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixkirbyhooks.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_roster_data.h"
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <ft/ftmain.h>
#include <gm/gmcollision.h>
#include <wp/weapon.h>

/* Kirby.Action id of DSamusNSPChargeAir in Action.asm scope KIRBY, used by set_aerial_charge_kirby. */
#define FTREMIX_KIRBYSAMUS_STATUS_DSAMUS_CHARGE_AIR 0x1AD

#define FTREMIX_KIRBYSAMUS_MEWTWO_RECOIL_PER_LEVEL 5.0F /* SHOOT_X_SPEED 0x40A0 */
#define FTREMIX_KIRBYSAMUS_MEWTWO_RECOIL_Y_STEP (-5.0F) /* lui 0xC0A0 */

static sb32 ftRemixKirbySamusIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

static sb32 ftRemixKirbySamusHasDSamusHat(FTStruct *fp)
{
    return *ftRemixFighterKirbyHat(fp) == FTREMIX_KIRBY_DSAMUS_HAT_ID;
}

sb32 ftRemixKirbyDSamusNSPAirStartIsRelease(FTStruct *fp, sb32 vanilla_is_release)
{
    if (ftRemixKirbySamusHasDSamusHat(fp) == FALSE)
    {
        return vanilla_is_release;
    }
    return (fp->passive_vars.kirby.copysamus_charge_level == FTKIRBY_COPYSAMUS_CHARGE_MAX) ? TRUE : FALSE;
}

sb32 ftRemixKirbyDSamusNSPKeepsReleaseOnAirSwitch(FTStruct *fp)
{
    return ftRemixKirbySamusHasDSamusHat(fp);
}

/* set_aerial_charge_kirby: ftKirbyCopySamusSpecialNLoopSetStatus entered past its status id load, with the Dark Samus aerial charge. */
sb32 ftRemixKirbyDSamusNSPCheckAerialCharge(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos;

    if ((ftRemixKirbySamusHasDSamusHat(fp) == FALSE) || (fp->passive_vars.kirby.copysamus_charge_level == FTKIRBY_COPYSAMUS_CHARGE_MAX))
    {
        return FALSE;
    }
    if (fp->status_vars.kirby.copysamus_specialn.is_release != FALSE)
    {
        return FALSE;
    }
    ftMainSetStatus(fighter_gobj, FTREMIX_KIRBYSAMUS_STATUS_DSAMUS_CHARGE_AIR, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);

    fp->proc_damage = ftKirbyCopySamusSpecialNProcDamage;
    fp->status_vars.kirby.copysamus_specialn.charge_int = FTKIRBY_COPYSAMUS_CHARGE_INT;

    ftKirbyCopySamusSpecialNGetChargeShotPosition(fp, &pos);
    fp->status_vars.kirby.copysamus_specialn.charge_gobj = wpSamusChargeShotMakeWeapon(fighter_gobj, &pos, fp->passive_vars.kirby.copysamus_charge_level, FALSE);

    return TRUE;
}

sb32 ftRemixKirbyMewtwoNGetBallPosition(FTStruct *fp, Vec3f *pos)
{
    if (ftRemixKirbySamusIsKirby(fp) == FALSE)
    {
        return FALSE;
    }
    pos->x = pos->y = pos->z = 0.0F;

    gmCollisionGetFighterPartsWorldPosition(fp->joints[nFTPartsJointCommonStart + FTREMIX_KIRBY_MEWTWO_BALL_JOINT], pos);

    return TRUE;
}

void ftRemixKirbyMewtwoNGetShootRecoil(FTStruct *fp, f32 *recoil_base, f32 *recoil_y_step)
{
    if (fp->passive_vars.kirby.copy_id != REMIX_ID_MTWO)
    {
        return;
    }
    *recoil_base += FTREMIX_KIRBYSAMUS_MEWTWO_RECOIL_PER_LEVEL * fp->passive_vars.kirby.copysamus_charge_level;
    *recoil_y_step = FTREMIX_KIRBYSAMUS_MEWTWO_RECOIL_Y_STEP;
}

#endif
