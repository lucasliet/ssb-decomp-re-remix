/**
 * ftremixebinsp.c - C ports of Smash Remix Ebisumaru neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Ebi/EbiSpecial.asm, scope EbiNSP).
 *
 * Ebisumaru's wait, walk and air physics are GoemonNSP routines, ported in ft/ftremix/goemon/.
 * Ebisumaru's neutral special creates no weapon: EbiNSP.end_main_ only returns to idle.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixEbiStatusNSPGroundBegin = 0xE5,
    nFTRemixEbiStatusNSPGroundWait = 0xE6,
    nFTRemixEbiStatusNSPGroundWalk1 = 0xE7,
    nFTRemixEbiStatusNSPGroundWalk2 = 0xE8,
    nFTRemixEbiStatusNSPGroundBWalk1 = 0xE9,
    nFTRemixEbiStatusNSPGroundBWalk2 = 0xEA,
    nFTRemixEbiStatusNSPGroundEnd = 0xEB,
    nFTRemixEbiStatusNSPAirBegin = 0xEC,
    nFTRemixEbiStatusNSPAirWait = 0xED,
    nFTRemixEbiStatusNSPAirEnd = 0xEE
};

/* Kirby and J Kirby use these ids for the same specials. Kirby.asm add_new_action order: 0xDC + 83 + index,
 * where index is 114 for EBI_NSP_Ground_Begin. */
enum
{
    nFTRemixEbiKirbyStatusNSPGroundBegin = 0x1A1,
    nFTRemixEbiKirbyStatusNSPGroundWait = 0x1A2,
    nFTRemixEbiKirbyStatusNSPGroundWalk1 = 0x1A3,
    nFTRemixEbiKirbyStatusNSPGroundWalk2 = 0x1A4,
    nFTRemixEbiKirbyStatusNSPGroundBWalk1 = 0x1A5,
    nFTRemixEbiKirbyStatusNSPGroundBWalk2 = 0x1A6,
    nFTRemixEbiKirbyStatusNSPGroundEnd = 0x1A7,
    nFTRemixEbiKirbyStatusNSPAirBegin = 0x1A8,
    nFTRemixEbiKirbyStatusNSPAirWait = 0x1A9,
    nFTRemixEbiKirbyStatusNSPAirEnd = 0x1AA
};

/* Remix scratch words in the status variables (N64 FTStruct offsets). */
#define EBI_NSP_N64_CHARGE_TIMER 0x0B18
#define EBI_NSP_N64_CHARGE_FLAG 0x0B1C

/* ground_transition_ (0x0802) and the ground/air transitions (0x0803, which adds FTSTATUS_PRESERVE_HIT). */
#define EBI_NSP_WALK_TRANSITION_FLAGS (FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX)
#define EBI_NSP_AIR_TRANSITION_FLAGS (FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX)

/* Walk status slots, from the most backward stick range to the most forward. */
#define EBI_NSP_WALK_SLOT_BWALK2 0
#define EBI_NSP_WALK_SLOT_BWALK1 1
#define EBI_NSP_WALK_SLOT_WAIT 2
#define EBI_NSP_WALK_SLOT_WALK1 3
#define EBI_NSP_WALK_SLOT_WALK2 4

/* Indexed by current ground status minus nFTRemixEbiStatusNSPGroundWait (the same values serve Kirby). */
static const f32 sFTRemixEbiCycleLength[5] = { 40.0F, 80.0F, 60.0F, 80.0F, 60.0F };

static const s32 sFTRemixEbiGroundWalkStatus[5] =
{
    nFTRemixEbiStatusNSPGroundBWalk2,
    nFTRemixEbiStatusNSPGroundBWalk1,
    nFTRemixEbiStatusNSPGroundWait,
    nFTRemixEbiStatusNSPGroundWalk1,
    nFTRemixEbiStatusNSPGroundWalk2
};

static const s32 sFTRemixEbiKirbyGroundWalkStatus[5] =
{
    nFTRemixEbiKirbyStatusNSPGroundBWalk2,
    nFTRemixEbiKirbyStatusNSPGroundBWalk1,
    nFTRemixEbiKirbyStatusNSPGroundWait,
    nFTRemixEbiKirbyStatusNSPGroundWalk1,
    nFTRemixEbiKirbyStatusNSPGroundWalk2
};

/* Indexed by current ground status minus nFTRemixEbiStatusNSPGroundBegin. */
static const s32 sFTRemixEbiGroundToAirStatus[7] =
{
    nFTRemixEbiStatusNSPAirBegin,
    nFTRemixEbiStatusNSPAirWait,
    nFTRemixEbiStatusNSPAirWait,
    nFTRemixEbiStatusNSPAirWait,
    nFTRemixEbiStatusNSPAirWait,
    nFTRemixEbiStatusNSPAirWait,
    nFTRemixEbiStatusNSPAirEnd
};

/* Indexed by current air status minus nFTRemixEbiStatusNSPAirBegin. */
static const s32 sFTRemixEbiAirToGroundStatus[3] =
{
    nFTRemixEbiStatusNSPGroundBegin,
    nFTRemixEbiStatusNSPGroundWait,
    nFTRemixEbiStatusNSPGroundEnd
};

/* kirby_ground_to_air_table, indexed by current ground status minus nFTRemixEbiKirbyStatusNSPGroundBegin. */
static const s32 sFTRemixEbiKirbyGroundToAirStatus[7] =
{
    nFTRemixEbiKirbyStatusNSPAirBegin,
    nFTRemixEbiKirbyStatusNSPAirWait,
    nFTRemixEbiKirbyStatusNSPAirWait,
    nFTRemixEbiKirbyStatusNSPAirWait,
    nFTRemixEbiKirbyStatusNSPAirWait,
    nFTRemixEbiKirbyStatusNSPAirWait,
    nFTRemixEbiKirbyStatusNSPAirEnd
};

/* kirby_air_to_ground_table, indexed by current air status minus nFTRemixEbiKirbyStatusNSPAirBegin. */
static const s32 sFTRemixEbiKirbyAirToGroundStatus[3] =
{
    nFTRemixEbiKirbyStatusNSPGroundBegin,
    nFTRemixEbiKirbyStatusNSPGroundWait,
    nFTRemixEbiKirbyStatusNSPGroundEnd
};

/* Kirby or J Kirby get their own status ids for the same special. */
static s32 ftRemixEbiNSPStatusForFighter(FTStruct *fp, s32 ebi_status, s32 kirby_status)
{
    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return kirby_status;
    }
    return ebi_status;
}

/* Sets the Ebisumaru or Kirby status id, then plays the anim events, as the *_initial_ routines do. */
static void ftRemixEbiNSPSetStatus(GObj *fighter_gobj, s32 ebi_status, s32 kirby_status, u32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixEbiNSPStatusForFighter(fp, ebi_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, flags);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Clears temp variables 1 to 3 (motion_vars.flags), which the begin routines reset before the moveset runs. */
static void ftRemixEbiNSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Clears the charge timer and charge flag that the begin routines reset. */
static void ftRemixEbiNSPResetChargeVars(FTStruct *fp)
{
    *ftRemixFighterWord(fp, EBI_NSP_N64_CHARGE_TIMER) = 0;
    *ftRemixFighterWord(fp, EBI_NSP_N64_CHARGE_FLAG) = 0;
}

/* EbiNSP.ground_begin_initial_ */
static void ftRemixEbiNSPGroundBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixEbiNSPSetStatus(fighter_gobj, nFTRemixEbiStatusNSPGroundBegin, nFTRemixEbiKirbyStatusNSPGroundBegin, FTSTATUS_PRESERVE_NONE);
    ftRemixEbiNSPClearTempVars(fp);
    ftRemixEbiNSPResetChargeVars(fp);
}

/* EbiNSP.air_begin_initial_ */
static void ftRemixEbiNSPAirBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixEbiNSPSetStatus(fighter_gobj, nFTRemixEbiStatusNSPAirBegin, nFTRemixEbiKirbyStatusNSPAirBegin, FTSTATUS_PRESERVE_NONE);
    ftRemixEbiNSPClearTempVars(fp);
    ftRemixEbiNSPResetChargeVars(fp);
}

/* EbiNSP.ground_wait_initial_ (not in the worklist, used by ground_begin_main_) */
static void ftRemixEbiNSPGroundWaitInitial(GObj *fighter_gobj)
{
    ftRemixEbiNSPSetStatus(fighter_gobj, nFTRemixEbiStatusNSPGroundWait, nFTRemixEbiKirbyStatusNSPGroundWait, FTSTATUS_PRESERVE_LOOPSFX);
}

/* EbiNSP.air_wait_initial_ (not in the worklist, used by air_begin_main_) */
static void ftRemixEbiNSPAirWaitInitial(GObj *fighter_gobj)
{
    ftRemixEbiNSPSetStatus(fighter_gobj, nFTRemixEbiStatusNSPAirWait, nFTRemixEbiKirbyStatusNSPAirWait, FTSTATUS_PRESERVE_LOOPSFX);
}

/* EbiNSP.ground_end_initial_ (not in the worklist, used by ground_begin_main_, ground_wait_main_) */
static void ftRemixEbiNSPGroundEndInitial(GObj *fighter_gobj)
{
    ftRemixEbiNSPSetStatus(fighter_gobj, nFTRemixEbiStatusNSPGroundEnd, nFTRemixEbiKirbyStatusNSPGroundEnd, FTSTATUS_PRESERVE_NONE);
}

/* EbiNSP.air_end_initial_ (not in the worklist, used by air_begin_main_, air_wait_main_) */
static void ftRemixEbiNSPAirEndInitial(GObj *fighter_gobj)
{
    ftRemixEbiNSPSetStatus(fighter_gobj, nFTRemixEbiStatusNSPAirEnd, nFTRemixEbiKirbyStatusNSPAirEnd, FTSTATUS_PRESERVE_NONE);
}

/* Shared body of EbiNSP.ground_begin_main_ and air_begin_main_: with temp variable 2 set, releasing B ends the
 * special and holding B starts the wait. Without temp variable 2 nothing happens. */
static void ftRemixEbiNSPBeginProcUpdate(GObj *fighter_gobj, void (*proc_end)(GObj *), void (*proc_wait)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    if (fp->input.pl.button_hold & B_BUTTON)
    {
        proc_wait(fighter_gobj);
        return;
    }
    proc_end(fighter_gobj);
}

/* EbiNSP.ground_begin_main_ */
static void ftRemixEbiNSPGroundBeginProcUpdate(GObj *fighter_gobj)
{
    ftRemixEbiNSPBeginProcUpdate(fighter_gobj, ftRemixEbiNSPGroundEndInitial, ftRemixEbiNSPGroundWaitInitial);
}

/* EbiNSP.air_begin_main_ */
static void ftRemixEbiNSPAirBeginProcUpdate(GObj *fighter_gobj)
{
    ftRemixEbiNSPBeginProcUpdate(fighter_gobj, ftRemixEbiNSPAirEndInitial, ftRemixEbiNSPAirWaitInitial);
}

/* Shared body of EbiNSP.ground_wait_main_ and air_wait_main_: the special ends as soon as B is released. */
static void ftRemixEbiNSPWaitProcUpdate(GObj *fighter_gobj, void (*proc_end)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.button_hold & B_BUTTON)
    {
        return;
    }
    proc_end(fighter_gobj);
}

/* EbiNSP.ground_wait_main_ */
static void ftRemixEbiNSPGroundWaitProcUpdate(GObj *fighter_gobj)
{
    ftRemixEbiNSPWaitProcUpdate(fighter_gobj, ftRemixEbiNSPGroundEndInitial);
}

/* EbiNSP.air_wait_main_ */
static void ftRemixEbiNSPAirWaitProcUpdate(GObj *fighter_gobj)
{
    ftRemixEbiNSPWaitProcUpdate(fighter_gobj, ftRemixEbiNSPAirEndInitial);
}

/* EbiNSP.end_main_: returns to idle once the end animation has finished. */
static void ftRemixEbiNSPEndProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* EbiNSP.ground_to_air_: moves a grounded special to its aerial status. */
static void ftRemixEbiNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 air_status;

    mpCommonSetFighterAir(fp);
    air_status = sFTRemixEbiGroundToAirStatus[fp->status_id - nFTRemixEbiStatusNSPGroundBegin];
    ftMainSetStatus(fighter_gobj, air_status, fighter_gobj->anim_frame, 1.0F, EBI_NSP_AIR_TRANSITION_FLAGS);
    ftPhysicsClampAirVelXMax(fp);
}

/* EbiNSP.air_to_ground_: moves an aerial special to its grounded status. */
static void ftRemixEbiNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 ground_status;

    mpCommonSetFighterGround(fp);
    ground_status = sFTRemixEbiAirToGroundStatus[fp->status_id - nFTRemixEbiStatusNSPAirBegin];
    ftMainSetStatus(fighter_gobj, ground_status, fighter_gobj->anim_frame, 1.0F, EBI_NSP_AIR_TRANSITION_FLAGS);
}

/* EbiNSP.ground_collision_ */
static void ftRemixEbiNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixEbiNSPGroundToAir);
}

/* EbiNSP.air_collision_ */
static void ftRemixEbiNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixEbiNSPAirToGround);
}

/* EbiNSP.kirby_ground_to_air_: moves a grounded Kirby special to its aerial status. */
static void ftRemixEbiNSPKirbyGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 air_status;

    mpCommonSetFighterAir(fp);
    air_status = sFTRemixEbiKirbyGroundToAirStatus[fp->status_id - nFTRemixEbiKirbyStatusNSPGroundBegin];
    ftMainSetStatus(fighter_gobj, air_status, fighter_gobj->anim_frame, 1.0F, EBI_NSP_WALK_TRANSITION_FLAGS);
    ftPhysicsClampAirVelXMax(fp);
}

/* EbiNSP.kirby_air_to_ground_: moves an aerial Kirby special to its grounded status. */
static void ftRemixEbiNSPKirbyAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 ground_status;

    mpCommonSetFighterGround(fp);
    ground_status = sFTRemixEbiKirbyAirToGroundStatus[fp->status_id - nFTRemixEbiKirbyStatusNSPAirBegin];
    ftMainSetStatus(fighter_gobj, ground_status, fighter_gobj->anim_frame, 1.0F, EBI_NSP_WALK_TRANSITION_FLAGS);
}

/* EbiNSP.kirby_ground_collision_ */
static void ftRemixEbiNSPKirbyGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixEbiNSPKirbyGroundToAir);
}

/* EbiNSP.kirby_air_collision_ */
static void ftRemixEbiNSPKirbyAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixEbiNSPKirbyAirToGround);
}

/* Start frame for a walk cycle change: the current cycle position scaled to the target cycle, truncated.
 * Returns 0 when either side is the wait status. The asm's second check reads a2 (the start frame register)
 * where the comment says target, so this port tests the target as its comment says; see the notes file. */
static f32 ftRemixEbiNSPCycleStartFrame(GObj *fighter_gobj, s32 target, s32 wait_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 multiplier;

    if ((fp->status_id == wait_status) || (target == wait_status))
    {
        return 0.0F;
    }
    multiplier = fighter_gobj->anim_frame / sFTRemixEbiCycleLength[fp->status_id - wait_status];
    return (f32) (s32) (multiplier * sFTRemixEbiCycleLength[target - wait_status]);
}

/* ground_transition_ and kirby_ground_transition_: changes the walk status, keeping the cycle position. */
static void ftRemixEbiNSPGroundTransition(GObj *fighter_gobj, s32 target, s32 wait_status)
{
    f32 start_frame = ftRemixEbiNSPCycleStartFrame(fighter_gobj, target, wait_status);

    ftMainSetStatus(fighter_gobj, target, start_frame, 1.0F, EBI_NSP_WALK_TRANSITION_FLAGS);
}

/* Maps the stick direction (stick_x times the facing) to a walk slot, as ground_interrupt_ does. */
static s32 ftRemixEbiNSPGroundWalkSlot(s32 stick_dir)
{
    if (stick_dir < -44)
    {
        return EBI_NSP_WALK_SLOT_BWALK2;
    }
    if (stick_dir < -9)
    {
        return EBI_NSP_WALK_SLOT_BWALK1;
    }
    if (stick_dir < 10)
    {
        return EBI_NSP_WALK_SLOT_WAIT;
    }
    if (stick_dir < 45)
    {
        return EBI_NSP_WALK_SLOT_WALK1;
    }
    return EBI_NSP_WALK_SLOT_WALK2;
}

/* EbiNSP.ground_interrupt_: switches between the grounded wait and walk statuses from the stick. Ebisumaru uses
 * the Ebi statuses; every other fighter that reaches this routine (Kirby, J Kirby) uses the Kirby statuses. */
static void ftRemixEbiNSPGroundProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 stick_dir = fp->input.pl.stick_range.x * fp->lr;
    s32 slot = ftRemixEbiNSPGroundWalkSlot(stick_dir);
    s32 target;

    if (fp->fkind == REMIX_ID_EBI)
    {
        target = sFTRemixEbiGroundWalkStatus[slot];
        if (target != fp->status_id)
        {
            ftRemixEbiNSPGroundTransition(fighter_gobj, target, nFTRemixEbiStatusNSPGroundWait);
        }
        return;
    }
    target = sFTRemixEbiKirbyGroundWalkStatus[slot];
    if (target != fp->status_id)
    {
        ftRemixEbiNSPGroundTransition(fighter_gobj, target, nFTRemixEbiKirbyStatusNSPGroundWait);
    }
}

/* Remix address -> port routine for every EbiNSP routine in the worklist. */
const FTRemixProc dFTRemixEbiNSPProcs[] =
{
    { REMIX_SYM_EbiNSP__air_begin_initial_, ftRemixEbiNSPAirBeginInitial },
    { REMIX_SYM_EbiNSP__air_begin_main_, ftRemixEbiNSPAirBeginProcUpdate },
    { REMIX_SYM_EbiNSP__air_collision_, ftRemixEbiNSPAirProcMap },
    { REMIX_SYM_EbiNSP__air_wait_main_, ftRemixEbiNSPAirWaitProcUpdate },
    { REMIX_SYM_EbiNSP__end_main_, ftRemixEbiNSPEndProcUpdate },
    { REMIX_SYM_EbiNSP__ground_begin_initial_, ftRemixEbiNSPGroundBeginInitial },
    { REMIX_SYM_EbiNSP__ground_begin_main_, ftRemixEbiNSPGroundBeginProcUpdate },
    { REMIX_SYM_EbiNSP__ground_collision_, ftRemixEbiNSPGroundProcMap },
    { REMIX_SYM_EbiNSP__ground_interrupt_, ftRemixEbiNSPGroundProcInterrupt },
    { REMIX_SYM_EbiNSP__ground_wait_main_, ftRemixEbiNSPGroundWaitProcUpdate },
    { REMIX_SYM_EbiNSP__kirby_ground_collision_, ftRemixEbiNSPKirbyGroundProcMap },
    { REMIX_SYM_EbiNSP__kirby_air_collision_, ftRemixEbiNSPKirbyAirProcMap }
};

const s32 dFTRemixEbiNSPProcsNum = ARRAY_COUNT(dFTRemixEbiNSPProcs);

#endif
