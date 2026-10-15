/**
 * ftremixdedededsp.c - C ports of Smash Remix Dedede down special routines
 * (.toolchain/pkgs/smashremix-src/src/Dedede/DededeSpecial.asm, scope DededeDSP).
 *
 * The minion (Waddle Dee, Waddle Doo or Gordo) is a Remix custom item in
 * it/itremix/. This file keeps the held and field minion slots. The
 * max-charge spark is REMIX-TODO(GFXRoutine). Details: docs/remix/notes/40_DEDEDE_DSP.md.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"
#include <ft/ftchar/ftsamus/ftsamusfunctions.h>
#include <it/itremix/itremixwaddledee.h>
#include <it/itremix/itremixwaddledoo.h>
#include <it/itremix/itremixgordo.h>
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <ft/ftanimend.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <it/item.h>
#include <it/itmain.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixDededeStatusDSPGroundBegin = 0xF8,
    nFTRemixDededeStatusDSPGroundCharge = 0xF9,
    nFTRemixDededeStatusDSPGroundShoot = 0xFA,
    nFTRemixDededeStatusDSPAirBegin = 0xFB,
    nFTRemixDededeStatusDSPAirCharge = 0xFC,
    nFTRemixDededeStatusDSPAirShoot = 0xFD
};

/* Down special words past the vanilla FTStruct fields, at their N64 offsets (struct_offsets.txt). */
#define DEDEDE_DSP_OFF_CHARGE_LEVEL 0x0AE4
#define DEDEDE_DSP_OFF_TRANSITION 0x0B18
#define DEDEDE_DSP_OFF_CHARGE_TIMER 0x0B1C
#define DEDEDE_DSP_OFF_SHOOT_TIMER 0x0B28

#define DEDEDE_DSP_CHARGE_MAX 2
#define DEDEDE_DSP_MINION_RUMMAGE_TIME 12
#define DEDEDE_DSP_FIRST_PULL 6
#define DEDEDE_DSP_FIRST_STOW 32
#define DEDEDE_DSP_FIRST_CHARGE 54
#define DEDEDE_DSP_SECOND_PULL 60
#define DEDEDE_DSP_SECOND_STOW 87
#define DEDEDE_DSP_SECOND_CHARGE 109
#define DEDEDE_DSP_FINAL_PULL 115
#define DEDEDE_DSP_CHARGE_END_FRAME 20.0F      /* 0x41A00000 */
#define DEDEDE_DSP_MINION_FAKE_KIND 0x1C       /* Item kind set before destroying a minion, so the destroy skips its smoke gfx */
#define DEDEDE_DSP_MINION_SPAWN_VEL_Y 65.0F    /* 0x42820000 */
#define DEDEDE_DSP_MINION_THROWN 1             /* ITStruct.multi (N64 0x033E) read by minion_null_main_ */
#define DEDEDE_DSP_FIELD_MINIONS_MAX 2

/* N64 0x0B20 holds the attached minion as a pointer, which does not fit one 32-bit word on LP64. */
static GObj *sFTRemixDededeDSPMinion[MAXCONTROLLERS];

/* N64 0x0ADC/0x0AE0: the two thrown minions, oldest first. Entries can outlive a match, so they are
 * checked against the live item list before use. */
static GObj *sFTRemixDededeDSPFieldMinions[MAXCONTROLLERS][DEDEDE_DSP_FIELD_MINIONS_MAX];

static sb32 ftRemixDededeDSPIsLinked(GObj *gobj, s32 link_id)
{
    GObj *it_gobj;

    for (it_gobj = gGCCommonLinks[link_id]; it_gobj != NULL; it_gobj = it_gobj->link_next)
    {
        if (it_gobj == gobj)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* WaddleDee.remove_minion_check_: fills a free field slot, or retires the oldest minion (1 frame left). */
void ftRemixDededeDSPAddFieldMinion(GObj *owner_gobj, GObj *minion_gobj)
{
    GObj **slots;

    if ((owner_gobj == NULL) || !ftRemixDededeDSPIsLinked(owner_gobj, nGCCommonLinkIDFighter))
    {
        return;
    }
    slots = sFTRemixDededeDSPFieldMinions[ftGetStruct(owner_gobj)->player];

    if ((slots[0] == NULL) || !ftRemixDededeDSPIsLinked(slots[0], nGCCommonLinkIDItem))
    {
        slots[0] = minion_gobj;
        return;
    }
    if ((slots[1] == NULL) || !ftRemixDededeDSPIsLinked(slots[1], nGCCommonLinkIDItem))
    {
        slots[1] = minion_gobj;
        return;
    }
    itGetStruct(slots[0])->lifetime = 1;
    slots[0] = slots[1];
    slots[1] = minion_gobj;
}

/* WaddleDee.minion_free_: removes the minion from a living Dedede owner's held and field slots. */
void ftRemixDededeDSPFreeMinion(GObj *owner_gobj, GObj *minion_gobj)
{
    FTStruct *fp;
    GObj **slots;

    if ((owner_gobj == NULL) || !ftRemixDededeDSPIsLinked(owner_gobj, nGCCommonLinkIDFighter))
    {
        return;
    }
    fp = ftGetStruct(owner_gobj);

    if (fp->fkind != REMIX_ID_DEDEDE)
    {
        return;
    }
    if (sFTRemixDededeDSPMinion[fp->player] == minion_gobj)
    {
        sFTRemixDededeDSPMinion[fp->player] = NULL;
    }
    slots = sFTRemixDededeDSPFieldMinions[fp->player];

    if (slots[0] == minion_gobj)
    {
        slots[0] = slots[1];
        slots[1] = NULL;
    }
    else if (slots[1] == minion_gobj)
    {
        slots[1] = NULL;
    }
}

/* Reads a down special word at its N64 FTStruct offset. */
static s32 *ftRemixDededeDSPWord(FTStruct *fp, u32 n64_offset)
{
    return ftRemixFighterWord(fp, n64_offset);
}

/* DededeDSP.destroy_attached_minion_: clears the minion slot and destroys its item. */
static void ftRemixDededeDSPDestroyMinion(FTStruct *fp)
{
    GObj *minion_gobj = sFTRemixDededeDSPMinion[fp->player];

    if (minion_gobj == NULL)
    {
        return;
    }
    sFTRemixDededeDSPMinion[fp->player] = NULL;
    itGetStruct(minion_gobj)->kind = DEDEDE_DSP_MINION_FAKE_KIND;
    itMainDestroyItem(minion_gobj);
}

/* DededeDSP.dedede_on_hit_subroutine_: resets the charge level and destroys the minion. */
static void ftRemixDededeDSPOnHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_LEVEL) = 0;
    ftRemixDededeDSPDestroyMinion(fp);
}

/* Installs the down special's on-hit routine as the fighter's damage proc. */
static void ftRemixDededeDSPSetOnHit(FTStruct *fp)
{
    fp->proc_damage = ftRemixDededeDSPOnHit;
}

/* DededeDSP.attach_minion_: the charge level picks Waddle Dee (0), Waddle Doo (1) or Gordo (2). */
static GObj* ftRemixDededeDSPMakeMinion(GObj *fighter_gobj, s32 charge_level, Vec3f *pos, Vec3f *vel)
{
    switch (charge_level)
    {
    case 0:
        return itRemixWaddleDeeStageSetting(fighter_gobj, pos, vel, 0);

    case 1:
        return itRemixWaddleDooMakeItem(fighter_gobj, pos, vel, 0);

    default:
        return itRemixGordoMakeItem(fighter_gobj, pos, vel, 0);
    }
}

/* DededeDSP.attach_minion_: spawns the minion at the hand and attaches it. */
static void ftRemixDededeDSPAttachMinion(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos;
    Vec3f vel = { 0.0F, DEDEDE_DSP_MINION_SPAWN_VEL_Y, 0.0F };

    ftRemixDededeDSPSetOnHit(fp);
    ftSamusSpecialNGetChargeShotPosition(fp, &pos);
    sFTRemixDededeDSPMinion[fp->player] = ftRemixDededeDSPMakeMinion(fighter_gobj, *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_LEVEL), &pos, &vel);
}

/* Shared body of DededeDSP.ground_begin_initial_ and air_begin_initial_. */
static void ftRemixDededeDSPBeginSetStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    sFTRemixDededeDSPMinion[fp->player] = NULL;
    fp->motion_vars.flags.flag0 = 0;
    ftRemixDededeDSPSetOnHit(fp);
    *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_TRANSITION) = (*ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_LEVEL) == DEDEDE_DSP_CHARGE_MAX);
}

/* DededeDSP.ground_begin_initial_ */
static void ftRemixDededeDSPGroundBeginSetStatus(GObj *fighter_gobj)
{
    ftRemixDededeDSPBeginSetStatus(fighter_gobj, nFTRemixDededeStatusDSPGroundBegin);
}

/* DededeDSP.air_begin_initial_ */
static void ftRemixDededeDSPAirBeginSetStatus(GObj *fighter_gobj)
{
    ftRemixDededeDSPBeginSetStatus(fighter_gobj, nFTRemixDededeStatusDSPAirBegin);
}

/* Shared body of the grounded and aerial charge initial routines. */
static void ftRemixDededeDSPChargeSetStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_TIMER) = 0;
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixDededeDSPSetOnHit(fp);
}

/* Shared body of the grounded and aerial shoot initial routines. */
static void ftRemixDededeDSPShootSetStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_SHOOT_TIMER) = 0;
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    fp->is_special_interrupt = TRUE;
    if (sFTRemixDededeDSPMinion[fp->player] == NULL)
    {
        ftRemixDededeDSPAttachMinion(fighter_gobj);
    }
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixDededeDSPSetOnHit(fp);
}

/* DededeDSP.ground_begin_transition_ */
static void ftRemixDededeDSPGroundBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDededeStatusDSPGroundBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixDededeDSPSetOnHit(fp);
}

/* DededeDSP.air_begin_transition_ */
static void ftRemixDededeDSPAirBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDededeStatusDSPAirBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixDededeDSPSetOnHit(fp);
}

/* DededeDSP.ground_charge_transition_ (flags 0x802: COLANIM and LOOPSFX). */
static void ftRemixDededeDSPGroundChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDededeStatusDSPGroundCharge, fighter_gobj->anim_frame, 1.0F,
        FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX);
    ftRemixDededeDSPSetOnHit(fp);
}

/* DededeDSP.air_charge_transition_ (flags 0x802: COLANIM and LOOPSFX). */
static void ftRemixDededeDSPAirChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDededeStatusDSPAirCharge, fighter_gobj->anim_frame, 1.0F,
        FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX);
    ftRemixDededeDSPSetOnHit(fp);
}

/* DededeDSP.ground_to_air: the shoot continues as the aerial shoot, after the fighter leaves the ground. */
static void ftRemixDededeDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id + 3, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixDededeDSPSetOnHit(fp);
}

/* DededeDSP.air_to_ground: the shoot continues as the grounded shoot, after the fighter lands. */
static void ftRemixDededeDSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id - 3, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixDededeDSPSetOnHit(fp);
}

/* DededeDSP.begin_main_: once the begin animation ends, starts the charge or the shoot. */
static void ftRemixDededeDSPBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_shoot = (*ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_TRANSITION) != 0);

    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    if (fp->ga)
    {
        if (is_shoot)
        {
            ftRemixDededeDSPShootSetStatus(fighter_gobj, nFTRemixDededeStatusDSPAirShoot);
        }
        else
        {
            ftRemixDededeDSPChargeSetStatus(fighter_gobj, nFTRemixDededeStatusDSPAirCharge);
        }
        return;
    }
    if (is_shoot)
    {
        ftRemixDededeDSPShootSetStatus(fighter_gobj, nFTRemixDededeStatusDSPGroundShoot);
    }
    else
    {
        ftRemixDededeDSPChargeSetStatus(fighter_gobj, nFTRemixDededeStatusDSPGroundCharge);
    }
}

/* Ends the charge at full power once the animation reaches DEDEDE_DSP_CHARGE_END_FRAME (DededeDSP.charge_main_ tail). */
static void ftRemixDededeDSPChargeCheckEnd(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (*ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_LEVEL) != DEDEDE_DSP_CHARGE_MAX)
    {
        return;
    }
    if (fighter_gobj->anim_frame < DEDEDE_DSP_CHARGE_END_FRAME)
    {
        return;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
    ftRemixDededeDSPDestroyMinion(fp);
}

/* Adds one charge level. Reaching max plays GFXRoutine.id.DEDEDE_CHARGE (REMIX-TODO(GFXRoutine): the white spark). */
static void ftRemixDededeDSPAddCharge(FTStruct *fp)
{
    *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_LEVEL) += 1;
}

/* DededeDSP.charge_main_: attaches, stows or charges the minion on the frames set by the timer. */
static void ftRemixDededeDSPChargeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *timer = ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_TIMER);

    *timer += 1;
    switch (*timer)
    {
        case DEDEDE_DSP_FIRST_PULL:
        case DEDEDE_DSP_SECOND_PULL:
        case DEDEDE_DSP_FINAL_PULL:
            ftRemixDededeDSPAttachMinion(fighter_gobj);
            return;
        case DEDEDE_DSP_FIRST_STOW:
        case DEDEDE_DSP_SECOND_STOW:
            ftRemixDededeDSPDestroyMinion(fp);
            return;
        case DEDEDE_DSP_FIRST_CHARGE:
        case DEDEDE_DSP_SECOND_CHARGE:
            ftRemixDededeDSPAddCharge(fp);
            return;
        default:
            break;
    }
    ftRemixDededeDSPChargeCheckEnd(fighter_gobj);
}

/* Shared release test for the charge interrupts: B released after the minimum rummage time. */
static sb32 ftRemixDededeDSPIsReleased(FTStruct *fp, u32 elapsed_frames)
{
    return (elapsed_frames >= DEDEDE_DSP_MINION_RUMMAGE_TIME) && !(fp->input.pl.button_hold & B_BUTTON);
}

/* Shield press cancels the charge: destroys the minion and runs proc_idle. */
static void ftRemixDededeDSPChargeCancel(GObj *fighter_gobj, void (*proc_idle)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!(fp->input.button_mask_z & fp->input.pl.button_tap))
    {
        return;
    }
    ftRemixDededeDSPDestroyMinion(fp);
    proc_idle(fighter_gobj);
}

/* DededeDSP.ground_charge_interrupt_ */
static void ftRemixDededeDSPGroundChargeProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 timer = *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_TIMER);

    if (ftRemixDededeDSPIsReleased(fp, timer))
    {
        ftRemixDededeDSPShootSetStatus(fighter_gobj, nFTRemixDededeStatusDSPGroundShoot);
        return;
    }
    ftRemixDededeDSPChargeCancel(fighter_gobj, ftCommonWaitSetStatus);
}

/* DededeDSP.air_charge_interrupt_: reads the action frame timer (status_total_tics at 0x001C), not 0x0B1C. */
static void ftRemixDededeDSPAirChargeProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixDededeDSPIsReleased(fp, fp->status_total_tics))
    {
        ftRemixDededeDSPShootSetStatus(fighter_gobj, nFTRemixDededeStatusDSPAirShoot);
        return;
    }
    ftRemixDededeDSPChargeCancel(fighter_gobj, ftCommonFallSetStatus);
}

/* DededeDSP.shoot_main_: throws the minion when temp variable 1 is set, then returns to idle when the animation ends. */
static void ftRemixDededeDSPShootProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_SHOOT_TIMER) += 1;
    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        *ftRemixDededeDSPWord(fp, DEDEDE_DSP_OFF_CHARGE_LEVEL) = 0;
        if (sFTRemixDededeDSPMinion[fp->player] != NULL)
        {
            itGetStruct(sFTRemixDededeDSPMinion[fp->player])->multi = DEDEDE_DSP_MINION_THROWN;
            sFTRemixDededeDSPMinion[fp->player] = NULL;
        }
    }
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* DededeDSP.ground_begin_collision_ */
static void ftRemixDededeDSPGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixDededeDSPAirBeginTransition);
}

/* DededeDSP.air_begin_collision_ */
static void ftRemixDededeDSPAirBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDededeDSPGroundBeginTransition);
}

/* DededeDSP.ground_charge_collision_ */
static void ftRemixDededeDSPGroundChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixDededeDSPAirChargeTransition);
}

/* DededeDSP.air_charge_collision_ */
static void ftRemixDededeDSPAirChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDededeDSPGroundChargeTransition);
}

/* DededeDSP.ground_shoot_collision_ */
static void ftRemixDededeDSPGroundShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixDededeDSPGroundToAir);
}

/* DededeDSP.air_shoot_collision_ */
static void ftRemixDededeDSPAirShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDededeDSPAirToGround);
}

const FTRemixProc dFTRemixDededeDSPProcs[] =
{
    { REMIX_SYM_DededeDSP__air_begin_collision_, ftRemixDededeDSPAirBeginProcMap },
    { REMIX_SYM_DededeDSP__air_begin_initial_, ftRemixDededeDSPAirBeginSetStatus },
    { REMIX_SYM_DededeDSP__air_charge_collision_, ftRemixDededeDSPAirChargeProcMap },
    { REMIX_SYM_DededeDSP__air_charge_interrupt_, ftRemixDededeDSPAirChargeProcInterrupt },
    { REMIX_SYM_DededeDSP__air_shoot_collision_, ftRemixDededeDSPAirShootProcMap },
    { REMIX_SYM_DededeDSP__begin_main_, ftRemixDededeDSPBeginProcUpdate },
    { REMIX_SYM_DededeDSP__charge_main_, ftRemixDededeDSPChargeProcUpdate },
    { REMIX_SYM_DededeDSP__ground_begin_collision_, ftRemixDededeDSPGroundBeginProcMap },
    { REMIX_SYM_DededeDSP__ground_begin_initial_, ftRemixDededeDSPGroundBeginSetStatus },
    { REMIX_SYM_DededeDSP__ground_charge_collision_, ftRemixDededeDSPGroundChargeProcMap },
    { REMIX_SYM_DededeDSP__ground_charge_interrupt_, ftRemixDededeDSPGroundChargeProcInterrupt },
    { REMIX_SYM_DededeDSP__ground_shoot_collision_, ftRemixDededeDSPGroundShootProcMap },
    { REMIX_SYM_DededeDSP__shoot_main_, ftRemixDededeDSPShootProcUpdate }
};

const s32 dFTRemixDededeDSPProcsNum = ARRAY_COUNT(dFTRemixDededeDSPProcs);

#endif
