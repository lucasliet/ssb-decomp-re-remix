/**
 * ftremixdsamusnsp.c - C ports of Smash Remix Dark Samus neutral special (Charge Shot)
 * routines (.toolchain/pkgs/smashremix-src/src/DSamus/DsamusSpecial.asm, scope DSamusNSP).
 *
 * The air and ground charge states are Samus's charge shot states (status_vars.samus.specialn)
 * with the ammo count in passive_vars.samus.charge_level (N64 0x0ADC). The charge tick timer
 * reuses specialn.charge_int (N64 0x0B1C) and the charge ball gobj is specialn.charge_gobj (0x0B20).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include "remix/remix_roster_data.h"
#include <ft/fighter.h>
#include <ft/ftchar/ftsamus/ftsamus.h>
#include <ft/ftchar/ftsamus/ftsamusfunctions.h>
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>
#include <wp/weapon.h>

/* Status ids from scope Action in DSamus.asm (0xDC + i). */
enum
{
    nFTRemixDSamusStatusChargeShotCharging = 0xDF,
    nFTRemixDSamusStatusChargeShotChargingAir = 0xE7
};

/* Kirby.Action ids from scope KIRBY in Action.asm, used when Kirby or J Kirby copies Dark Samus. */
enum
{
    nFTRemixKirbyStatusDSCharging = 0x133,
    nFTRemixKirbyStatusDSamusNSPChargeAir = 0x1AD
};

#define DSAMUS_NSP_CHARGE_TICS 0x14
#define DSAMUS_NSP_MAX_AMMO 7
#define DSAMUS_NSP_EMPTY_COLANIM_ID 6
#define DSAMUS_NSP_CHARGE_FLAGS (FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX)

#define DSAMUS_NSP_AMMO(fp) ((fp)->passive_vars.samus.charge_level)
#define DSAMUS_NSP_KIRBY_AMMO(fp) ((fp)->passive_vars.kirby.copysamus_charge_level)
#define DSAMUS_NSP_CHARGE_TIMER(fp) ((fp)->status_vars.samus.specialn.charge_int)
#define DSAMUS_NSP_CHARGE_GOBJ(fp) ((fp)->status_vars.samus.specialn.charge_gobj)
#define DSAMUS_NSP_DSAMUS_HAT_ID 0x13

/** True for Kirby or J Kirby copying Dark Samus, which the asm branches on with the same two ids. */
static sb32 ftRemixDSamusNSPIsKirbyCopy(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/** Stores the ammo count as the charge size of the charge ball weapon, if one is attached. */
static void ftRemixDSamusNSPSetChargeShotAmmo(FTStruct *fp, s32 ammo)
{
    GObj *charge_gobj = DSAMUS_NSP_CHARGE_GOBJ(fp);

    if (charge_gobj == NULL)
    {
        return;
    }
    wpGetStruct(charge_gobj)->weapon_vars.charge_shot.charge_size = ammo;
}

/* DSamusNSP.air_charge_transition_: enters the air charge action and sets the on-hit routine. */
static void ftRemixDSamusNSPAirChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action_id;

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);

    if (ftRemixDSamusNSPIsKirbyCopy(fp))
    {
        action_id = nFTRemixKirbyStatusDSamusNSPChargeAir;
    }
    else
    {
        action_id = nFTRemixDSamusStatusChargeShotChargingAir;
    }
    ftMainSetStatus(fighter_gobj, action_id, fighter_gobj->anim_frame, 1.0F, DSAMUS_NSP_CHARGE_FLAGS);

    if (ftRemixDSamusNSPIsKirbyCopy(fp))
    {
        fp->proc_damage = ftKirbyCopySamusSpecialNProcDamage;
    }
    else
    {
        fp->proc_damage = ftSamusSpecialNProcDamage;
    }
}

/* DSamusNSP.ground_charge_transition_: enters the ground charge action and sets the on-hit routine. */
static void ftRemixDSamusNSPGroundChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action_id;

    mpCommonSetFighterGround(fp);

    if (ftRemixDSamusNSPIsKirbyCopy(fp))
    {
        action_id = nFTRemixKirbyStatusDSCharging;
    }
    else
    {
        action_id = nFTRemixDSamusStatusChargeShotCharging;
    }
    ftMainSetStatus(fighter_gobj, action_id, fighter_gobj->anim_frame, 1.0F, DSAMUS_NSP_CHARGE_FLAGS);

    if (ftRemixDSamusNSPIsKirbyCopy(fp))
    {
        fp->proc_damage = ftKirbyCopySamusSpecialNProcDamage;
    }
    else
    {
        fp->proc_damage = ftSamusSpecialNProcDamage;
    }
}

/* DSamusNSP.ground_charge_collision_: attaches the charge ball and moves to the air charge when there is no floor. */
static void ftRemixDSamusNSPGroundChargeCollision(GObj *fighter_gobj)
{
    ftSamusSpecialNSetChargeShotPosition(ftGetStruct(fighter_gobj));
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixDSamusNSPAirChargeTransition);
}

/* DSamusNSP.air_charge_collision_: attaches the charge ball and moves to the ground charge on landing. */
static void ftRemixDSamusNSPAirChargeCollision(GObj *fighter_gobj)
{
    ftSamusSpecialNSetChargeShotPosition(ftGetStruct(fighter_gobj));
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDSamusNSPGroundChargeTransition);
}

/* DSamusNSP.air_charge_main: gains one ammo every DSAMUS_NSP_CHARGE_TICS frames, up to the maximum. */
static void ftRemixDSamusNSPAirChargeMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 ammo;

    DSAMUS_NSP_CHARGE_TIMER(fp) -= 1;
    if (DSAMUS_NSP_CHARGE_TIMER(fp) != 0)
    {
        return;
    }
    DSAMUS_NSP_CHARGE_TIMER(fp) = DSAMUS_NSP_CHARGE_TICS;

    ammo = DSAMUS_NSP_AMMO(fp);
    if (ammo >= DSAMUS_NSP_MAX_AMMO)
    {
        return;
    }
    ammo++;
    DSAMUS_NSP_AMMO(fp) = ammo;

    if (ammo != DSAMUS_NSP_MAX_AMMO)
    {
        ftRemixDSamusNSPSetChargeShotAmmo(fp, ammo);
        return;
    }
    ftParamCheckSetFighterColAnimID(fighter_gobj, DSAMUS_NSP_EMPTY_COLANIM_ID, 0);
    ftSamusSpecialNDestroyChargeShot(fp);
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* DSamusNSP.air_charge_interrupt_: A or B fires the air shot; shield destroys the ball and falls. */
static void ftRemixDSamusNSPAirChargeInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 button_tap = fp->input.pl.button_tap;

    if ((button_tap & (A_BUTTON | B_BUTTON)) != 0)
    {
        if (fp->fkind == REMIX_ID_DSAMUS)
        {
            ftSamusSpecialAirNEndSetStatus(fighter_gobj);
        }
        else
        {
            ftKirbyCopySamusSpecialAirNEndSetStatus(fighter_gobj);
        }
        return;
    }

    if ((fp->input.button_mask_z & button_tap) == 0)
    {
        return;
    }
    ftSamusSpecialNDestroyChargeShot(fp);
    ftCommonFallSetStatus(fighter_gobj);
}

/* DSamusNSP.aerial_b_button_buffer_kirby: Kirby with the Dark Samus hat sets the shoot variable on B press. */
static void ftRemixDSamusNSPAerialBButtonBufferKirby(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (*ftRemixFighterKirbyHat(fp) != DSAMUS_NSP_DSAMUS_HAT_ID)
    {
        return;
    }
    ftSamusSpecialNStartProcInterrupt(fighter_gobj);
}

/* DSamusNSP.air_charge_kirby_main: Kirby copy of air_charge_main, counting ammo in copysamus_charge_level. */
static void ftRemixDSamusNSPKirbyAirChargeMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 ammo;

    DSAMUS_NSP_CHARGE_TIMER(fp) -= 1;
    if (DSAMUS_NSP_CHARGE_TIMER(fp) != 0)
    {
        return;
    }
    DSAMUS_NSP_CHARGE_TIMER(fp) = DSAMUS_NSP_CHARGE_TICS;

    ammo = DSAMUS_NSP_KIRBY_AMMO(fp);
    if (ammo >= DSAMUS_NSP_MAX_AMMO)
    {
        return;
    }
    ammo++;
    DSAMUS_NSP_KIRBY_AMMO(fp) = ammo;

    if (ammo != DSAMUS_NSP_MAX_AMMO)
    {
        ftRemixDSamusNSPSetChargeShotAmmo(fp, ammo);
        return;
    }
    ftParamCheckSetFighterColAnimID(fighter_gobj, DSAMUS_NSP_EMPTY_COLANIM_ID, 0);
    ftKirbyCopySamusSpecialNDestroyChargeShot(fp);
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* DSamusNSP.kirby_ground_charge_collision_: attaches the Kirby charge ball, air charge when no floor. */
static void ftRemixDSamusNSPKirbyGroundChargeCollision(GObj *fighter_gobj)
{
    ftKirbyCopySamusSpecialNSetChargeShotPosition(ftGetStruct(fighter_gobj));
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDSamusNSPAirChargeTransition);
}

/* DSamusNSP.air_charge_collision_kirby_: attaches the Kirby charge ball and moves to the ground charge on landing. */
static void ftRemixDSamusNSPKirbyAirChargeCollision(GObj *fighter_gobj)
{
    ftKirbyCopySamusSpecialNSetChargeShotPosition(ftGetStruct(fighter_gobj));
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDSamusNSPGroundChargeTransition);
}

/* Table of the DSamusNSP routines in the worklist 22_DSAMUS and the Kirby copy routines. */
const FTRemixProc dFTRemixDSamusNSPProcs[] =
{
    { REMIX_SYM_DSamusNSP__air_charge_main, ftRemixDSamusNSPAirChargeMain },
    { REMIX_SYM_DSamusNSP__air_charge_interrupt_, ftRemixDSamusNSPAirChargeInterrupt },
    { REMIX_SYM_DSamusNSP__ground_charge_collision_, ftRemixDSamusNSPGroundChargeCollision },
    { REMIX_SYM_DSamusNSP__air_charge_collision_, ftRemixDSamusNSPAirChargeCollision },
    { REMIX_SYM_DSamusNSP__aerial_b_button_buffer_kirby, ftRemixDSamusNSPAerialBButtonBufferKirby },
    { REMIX_SYM_DSamusNSP__air_charge_kirby_main, ftRemixDSamusNSPKirbyAirChargeMain },
    { REMIX_SYM_DSamusNSP__kirby_ground_charge_collision_, ftRemixDSamusNSPKirbyGroundChargeCollision },
    { REMIX_SYM_DSamusNSP__air_charge_collision_kirby_, ftRemixDSamusNSPKirbyAirChargeCollision },
};

const s32 dFTRemixDSamusNSPProcsNum = ARRAY_COUNT(dFTRemixDSamusNSPProcs);

#endif
