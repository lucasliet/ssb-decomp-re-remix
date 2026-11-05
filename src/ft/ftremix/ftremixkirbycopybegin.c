/**
 * ftremixkirbycopybegin.c - begin routines Kirby runs for the neutral special of a copied Slippy or Dragon King
 * (Kirby.asm kirby_ground_nsp / kirby_air_nsp: SlippyNSP.kirby_ground_begin_initial, kirby_air_begin_initial,
 * DragonKingNSP.kirby_ground_begin_initial_, kirby_air_begin_initial_). Each sets Kirby's own copy status, plays the first
 * animation events and clears the move variables, as the Fox and Dragon King entries they are based on do.
 *
 * ftRemixBindKirbyCopyHandlers asks ftRemixFindKirbyCopyBeginProc for these when the Remix address has no entry in the
 * ported procedure tables.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixkirbyhooks.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/ftmain.h>

/* Kirby.Action ids of Action.asm scope KIRBY: SlippyNSPG/A and DragonKingNSPGround/Air. */
enum
{
    nFTRemixKirbySlippyStatusNSPGround = 0x193,
    nFTRemixKirbySlippyStatusNSPAir = 0x194,
    nFTRemixKirbyDragonKingStatusNSPGround = 0x1AB,
    nFTRemixKirbyDragonKingStatusNSPAir = 0x1AC
};

#define FTREMIX_KIRBYCOPYBEGIN_DRAGONKING_BALL_OFFSET 0x0B20

typedef struct FTRemixKirbyCopyBeginProc
{
    u32 addr;
    FTRemixProcFn proc;

} FTRemixKirbyCopyBeginProc;

/* Fox's ftFoxSpecialNSetStatus without the Falco phantasm variable: the status, the first events, then flag0 and flag1 cleared. */
static void ftRemixKirbySlippyNSPBegin(GObj *fighter_gobj, s32 status_id, u32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, flags);
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
}

/* SlippyNSP.kirby_ground_begin_initial */
static void ftRemixKirbySlippyNSPGroundBegin(GObj *fighter_gobj)
{
    ftRemixKirbySlippyNSPBegin(fighter_gobj, nFTRemixKirbySlippyStatusNSPGround, FTSTATUS_PRESERVE_NONE);
}

/* SlippyNSP.kirby_air_begin_initial */
static void ftRemixKirbySlippyNSPAirBegin(GObj *fighter_gobj)
{
    ftRemixKirbySlippyNSPBegin(fighter_gobj, nFTRemixKirbySlippyStatusNSPAir, FTSTATUS_PRESERVE_FASTFALL);
}

/* The status, the first events, the three move flags cleared (ftKirbyCopyCaptainSpecialNInitStatusVars) and no ball object. */
static void ftRemixKirbyDragonKingNSPBegin(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp->motion_vars.flags.flag0 = fp->motion_vars.flags.flag1 = fp->motion_vars.flags.flag2 = 0;
    *(GObj **)ftRemixFighterWord(fp, FTREMIX_KIRBYCOPYBEGIN_DRAGONKING_BALL_OFFSET) = NULL;
}

/* DragonKingNSP.kirby_ground_begin_initial_ */
static void ftRemixKirbyDragonKingNSPGroundBegin(GObj *fighter_gobj)
{
    ftRemixKirbyDragonKingNSPBegin(fighter_gobj, nFTRemixKirbyDragonKingStatusNSPGround);
}

/* DragonKingNSP.kirby_air_begin_initial_ */
static void ftRemixKirbyDragonKingNSPAirBegin(GObj *fighter_gobj)
{
    ftRemixKirbyDragonKingNSPBegin(fighter_gobj, nFTRemixKirbyDragonKingStatusNSPAir);
}

static const FTRemixKirbyCopyBeginProc dFTRemixKirbyCopyBeginProcs[] =
{
    { REMIX_SYM_SlippyNSP__kirby_ground_begin_initial, ftRemixKirbySlippyNSPGroundBegin },
    { REMIX_SYM_SlippyNSP__kirby_air_begin_initial, ftRemixKirbySlippyNSPAirBegin },
    { REMIX_SYM_DragonKingNSP__kirby_ground_begin_initial_, ftRemixKirbyDragonKingNSPGroundBegin },
    { REMIX_SYM_DragonKingNSP__kirby_air_begin_initial_, ftRemixKirbyDragonKingNSPAirBegin }
};

FTRemixProcFn ftRemixFindKirbyCopyBeginProc(u32 addr)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(dFTRemixKirbyCopyBeginProcs); i++)
    {
        if (dFTRemixKirbyCopyBeginProcs[i].addr == addr)
        {
            return dFTRemixKirbyCopyBeginProcs[i].proc;
        }
    }
    return NULL;
}

#endif
