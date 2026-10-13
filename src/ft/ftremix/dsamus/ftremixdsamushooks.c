/**
 * ftremixdsamushooks.c - Dark Samus values for the vanilla Samus special
 * routines that DsamusSpecial.asm patches (scopes DSamusNSP and DSamusDSP).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixfields.h"
#include <wp/weapon.h>

#define DSAMUS_DSP_GROUND_X_MULTIPLIER 0.625F /* GROUND_X_MULTIPLIER 0x3F20 */
#define DSAMUS_DSP_GROUND_ACCELERATION 10.0F  /* GROUND_ACCELERATION 0x4120 */
#define DSAMUS_DSP_GROUND_Y_VELOCITY 8.0F     /* GROUND_Y_VELOCITY 0x4100 */
#define DSAMUS_DSP_AIR_ACCELERATION 0.1875F   /* AIR_ACCELERATION 0x3E40 */
#define DSAMUS_STATUS_DSP_AIR 0xE8            /* DSamus.Action.DSPAir */
#define DSAMUS_STATUS_NSP_AIR_CHARGE 0xE7     /* DSamus.Action.NSP_Air_Charge, set_aerial_charge */
#define DSAMUS_KIRBY_HAT_ID 0x13

/* DSamusNSP.charge_level_array: graphic size, x speed, damage, hitbox size,
 * collision size, shoot FGM, charge FGM, hit FGM, priority. */
static const wpSamusChargeShotAttributes sFTRemixDSamusChargeLevels[FTSAMUS_CHARGE_MAX + 1] =
{
    { 120.0F, 60.0F,  2,  75, 10, 0xEE, 0xEF, 0x18, 1 },
    { 180.0F, 55.0F,  4, 100, 10, 0xEE, 0xF0, 0x18, 1 },
    { 250.0F, 50.0F,  6, 125, 10, 0xED, 0xF1, 0x17, 1 },
    { 320.0F, 45.0F,  9, 150, 10, 0xED, 0xF2, 0x17, 1 },
    { 400.0F, 40.0F, 12, 175, 10, 0xED, 0xF3, 0x17, 1 },
    { 490.0F, 35.0F, 15, 200, 10, 0xEC, 0xF4, 0x16, 1 },
    { 620.0F, 30.0F, 18, 225, 10, 0xEC, 0xF5, 0x16, 1 },
    { 740.0F, 25.0F, 24, 275, 10, 0xEB, 0xF6, 0x16, 2 }
};

static sb32 ftRemixIsDSamus(FTStruct *fp)
{
    return fp->fkind == REMIX_ID_DSAMUS;
}

/* DSamusDSP.ground_x_physics_patch_ (0x8015E020) */
void ftRemixDSamusDSPGroundPhysics(FTStruct *fp, f32 *vel, f32 *friction)
{
    if (ftRemixIsDSamus(fp))
    {
        *vel = DSAMUS_DSP_GROUND_X_MULTIPLIER;
        *friction = DSAMUS_DSP_GROUND_ACCELERATION;
    }
}

/* DSamusDSP.air_x_physics_patch_ (0x8015E0C0) */
f32 ftRemixDSamusDSPAirAccel(FTStruct *fp, f32 vanilla_accel)
{
    return ftRemixIsDSamus(fp) ? DSAMUS_DSP_AIR_ACCELERATION : vanilla_accel;
}

/* DSamusDSP.ground_y_velocity_patch_ (0x8015E1AC) */
f32 ftRemixDSamusDSPGroundJumpVelY(FTStruct *fp, f32 vanilla_vel_y)
{
    return ftRemixIsDSamus(fp) ? DSAMUS_DSP_GROUND_Y_VELOCITY : vanilla_vel_y;
}

/* DSamusDSP.air_action_patch_ (0x8015E284) */
s32 ftRemixDSamusDSPAirStatus(FTStruct *fp, s32 vanilla_status_id)
{
    return ftRemixIsDSamus(fp) ? DSAMUS_STATUS_DSP_AIR : vanilla_status_id;
}

/* DSamusNSP.initial_variable_override (0x8015DC20): the aerial start only
 * auto-shoots at full charge. */
sb32 ftRemixDSamusNSPAirStartIsRelease(FTStruct *fp, sb32 vanilla_is_release)
{
    if (!ftRemixIsDSamus(fp))
    {
        return vanilla_is_release;
    }
    return (fp->passive_vars.samus.charge_level == FTSAMUS_CHARGE_MAX) ? TRUE : FALSE;
}

/* DSamusNSP.remove_shoot_flag_aerial_transition (0x8015D590): leaving the
 * ground keeps the buffered shoot flag instead of forcing it. */
sb32 ftRemixDSamusNSPKeepsReleaseOnAirSwitch(FTStruct *fp)
{
    return ftRemixIsDSamus(fp);
}

/* DSamusNSP.set_aerial_charge: ftSamusSpecialNLoopSetStatus entered past its
 * status id load, with DSamus's aerial charge action instead. */
static void ftRemixDSamusNSPAirChargeSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos;

    ftMainSetStatus(fighter_gobj, DSAMUS_STATUS_NSP_AIR_CHARGE, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);

    fp->proc_damage = ftSamusSpecialNProcDamage;
    fp->status_vars.samus.specialn.charge_int = FTSAMUS_CHARGE_INT;

    ftSamusSpecialNGetChargeShotPosition(fp, &pos);
    fp->status_vars.samus.specialn.charge_gobj = wpSamusChargeShotMakeWeapon(fighter_gobj, &pos, fp->passive_vars.samus.charge_level, FALSE);
}

/* DSamusNSP.check_aerial_charge (0x8015D420): below full charge and with no
 * buffered shot, the aerial start keeps charging. Returns TRUE when it did. */
sb32 ftRemixDSamusNSPCheckAerialCharge(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!ftRemixIsDSamus(fp) || (fp->passive_vars.samus.charge_level == FTSAMUS_CHARGE_MAX))
    {
        return FALSE;
    }
    if (fp->status_vars.samus.specialn.is_release != FALSE)
    {
        return FALSE;
    }
    ftRemixDSamusNSPAirChargeSetStatus(fighter_gobj);

    return TRUE;
}

/* DSamusNSP.charge_level_patch_ (0x80168C64, 0x80168E50): Dark Samus, and Kirby
 * wearing her hat, launch with charge_level_array. */
const wpSamusChargeShotAttributes *ftRemixDSamusChargeShotAttributes(WPStruct *wp, const wpSamusChargeShotAttributes *vanilla_attrs)
{
    FTStruct *fp;

    if ((wp->owner_gobj == NULL) || (wp->owner_gobj->user_data.p == NULL))
    {
        return vanilla_attrs;
    }
    fp = ftGetStruct(wp->owner_gobj);

    if (ftRemixIsDSamus(fp) || (*ftRemixFighterKirbyHat(fp) == DSAMUS_KIRBY_HAT_ID))
    {
        return sFTRemixDSamusChargeLevels;
    }
    return vanilla_attrs;
}

#endif
