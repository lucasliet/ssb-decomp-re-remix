/**
 * ftremixmarinadsp.c - C ports of Smash Remix Marina down special routines
 * (.toolchain/pkgs/smashremix-src/src/Marina/MarinaSpecial.asm, scope MarinaDSP).
 *
 * The down special reuses Ness's PSI Magnet release-lag routines and keeps its
 * own variables in the same N64 slots: status_vars (0x0B18..0x0B24) and the
 * charge level in passive_vars (0x0ADC).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <it/item.h>
#include <it/itmain.h>
#include <it/itremix/itremixgem.h>
#include <it/itremix/itremixshuriken.h>
#include <it/itremix/itremixboomerang.h>
#include <it/itremix/itremixclanbomb.h>
#include <wp/weapon.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixMarinaStatusDSPGBegin = 0xFE,
    nFTRemixMarinaStatusDSPGWait = 0xFF,
    nFTRemixMarinaStatusDSPGAbsorb = 0x100,
    nFTRemixMarinaStatusDSPGEnd = 0x101,
    nFTRemixMarinaStatusDSPGPull = 0x102,
    nFTRemixMarinaStatusDSPGPullFail = 0x103,
    nFTRemixMarinaStatusDSPGStow = 0x104,
    nFTRemixMarinaStatusDSPABegin = 0x105,
    nFTRemixMarinaStatusDSPAWait = 0x106,
    nFTRemixMarinaStatusDSPAAbsorb = 0x107,
    nFTRemixMarinaStatusDSPAEnd = 0x108,
    nFTRemixMarinaStatusDSPAPull = 0x109,
    nFTRemixMarinaStatusDSPAPullFail = 0x10A,
    nFTRemixMarinaStatusDSPAStow = 0x10B
};

#define MARINA_DSP_MAX_CHARGE 4
#define MARINA_DSP_WAIT_STATUS_FLAGS (FTSTATUS_PRESERVE_EFFECT | FTSTATUS_PRESERVE_LOOPSFX)
#define MARINA_DSP_ABSORB_STATUS_FLAGS (FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_EFFECT)
#define MARINA_DSP_VOICE_FGM_ID 0x420
#define MARINA_DSP_VOICE_ALTERNATE_FGM_ID 0x43C
#define MARINA_DSP_BOWSER_BOMB_ITEM_ID 0x11A
#define MARINA_DSP_TURNIP_ITEM_ID 0x4B
#define MARINA_DSP_TURNIP_STITCH_DAMAGE 30
#define MARINA_DSP_TURNIP_DOT_DAMAGE 12
#define MARINA_DSP_TURNIP_WINK_DAMAGE 6
#define MARINA_DSP_CUSTOM_REFLECT_TYPE 0x0003
#define MARINA_DSP_CHARGE_SHOT_FULL 7
#define MARINA_DSP_CHARGE_SHOT_PROJECTILE_ID 0x0002
#define MARINA_DSP_YOSHI_EGG_PROJECTILE_ID 0x0005
#define MARINA_DSP_LINK_BOOMERANG_PROJECTILE_ID 0x0007
#define MARINA_DSP_PIKACHU_THUNDER_HEAD_PROJECTILE_ID 0x000B
#define MARINA_DSP_PIKACHU_THUNDER_TAIL_PROJECTILE_ID 0x000C
#define MARINA_DSP_ARWING_SHOT_PROJECTILE_ID 0x0012
/* Projectile.id.* from Projectile.asm: VANILLA_LAST_ID 0x1F, then add_projectile order. */
#define MARINA_DSP_GOLDENGUN_BULLET_PROJECTILE_ID 0x0026
#define MARINA_DSP_PIRATELAND_CANNONBALL_PROJECTILE_ID 0x0028

extern void ftNessSpecialLw_InitStatusVars(GObj *fighter_gobj);
extern void ftNessSpecialLw_UpdateReleaseLag(GObj *fighter_gobj);
extern void *func_800269C0_275C0(u16 id);

/* Marina's DSP variables in status_vars. min_frame_timer and b_not_held are the
 * slots ftNessSpecialLw_UpdateReleaseLag writes as release_lag and is_release. */
typedef struct FTRemixMarinaDSPStatusVars
{
    s32 min_frame_timer;  /* 0x0B18 */
    sb32 b_not_held;      /* 0x0B1C */
    s32 b_press_buffer;   /* 0x0B20: -1 on the first frame, else FALSE or TRUE */
    s32 add_charge;       /* 0x0B24 */
} FTRemixMarinaDSPStatusVars;

/* Marina's charge level in passive_vars (0x0ADC). */
typedef struct FTRemixMarinaDSPPassiveVars
{
    s32 charge_level;
} FTRemixMarinaDSPPassiveVars;

/* One row of ground_to_air_table or air_to_ground_table. */
typedef struct FTRemixMarinaDSPTransition
{
    s32 status_id;
    s32 flags;      /* FTSTATUS_PRESERVE_* for ftMainSetStatus (table argument 4) */
    sb32 is_reflect; /* TRUE sets the reflect bitflag (0x18C bit 5) */
} FTRemixMarinaDSPTransition;

/* Remix's absorb_struct (MarinaDSP.absorb_struct, 0x8056A1F0). Its first two
 * halfwords (index 1, custom reflect type 3) are packed into kind, the same
 * 32-bit slot FTSpecialColl uses. REMIX-TODO(Reflect): the custom reflect routine
 * that reads this hitbox is not ported. */
static FTSpecialColl sFTRemixMarinaDSPAbsorbColl =
{
    (0x0001 << 16) | 0x0003,
    0,
    { 0.0F, 500.0F, 450.0F },
    { 650.0F, 650.0F, 650.0F },
    0x18000000
};

/* ground_to_air_table, indexed by status_id - nFTRemixMarinaStatusDSPGBegin. */
static const FTRemixMarinaDSPTransition sFTRemixMarinaDSPGroundToAirTable[] =
{
    { nFTRemixMarinaStatusDSPABegin, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPAWait, 0x0097, TRUE },
    { nFTRemixMarinaStatusDSPAAbsorb, 0x0097, TRUE },
    { nFTRemixMarinaStatusDSPAEnd, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPAPull, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPAPullFail, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPAStow, 0x0092, FALSE }
};

/* air_to_ground_table, indexed by status_id - nFTRemixMarinaStatusDSPABegin. */
static const FTRemixMarinaDSPTransition sFTRemixMarinaDSPAirToGroundTable[] =
{
    { nFTRemixMarinaStatusDSPGBegin, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPGWait, 0x0097, TRUE },
    { nFTRemixMarinaStatusDSPGAbsorb, 0x0097, TRUE },
    { nFTRemixMarinaStatusDSPGEnd, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPGPull, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPGPullFail, 0x0092, FALSE },
    { nFTRemixMarinaStatusDSPGStow, 0x0092, FALSE }
};

/* Charge gained per item id, Marina's item_charge_table. Ids 0x00-0x39 are
 * vanilla and early custom items; 0x3A-0x4B are the stage, Pokemon and
 * character items added in Item.asm order. */
static const u8 sFTRemixMarinaDSPItemChargeTable[] =
{
    2, 2, 1, 1, 0, 0, 0, 2,  /* 0x00 */
    2, 1, 2, 1, 1, 4, 4, 4,  /* 0x08 */
    1, 1, 2, 2, 1, 1, 0, 0,  /* 0x10 */
    0, 0, 2, 0, 0, 0, 0, 0,  /* 0x18 */
    0, 0, 0, 0, 0, 0, 0, 0,  /* 0x20 */
    0, 0, 0, 0, 0, 0, 0, 0,  /* 0x28 */
    3, 0, 2, 0, 1, 2, 0, 0,  /* 0x30 */
    1, 0, 0, 0, 0, 0, 0, 0,  /* 0x38 */
    0, 0, 1, 2, 3, 4, 1, 1,  /* 0x40 */
    3, 0, 2, 1               /* 0x48 */
};

static FTRemixMarinaDSPStatusVars *ftRemixMarinaDSPStatusVars(FTStruct *fp)
{
    return (FTRemixMarinaDSPStatusVars *)&fp->status_vars;
}

static FTRemixMarinaDSPPassiveVars *ftRemixMarinaDSPPassiveVars(FTStruct *fp)
{
    return (FTRemixMarinaDSPPassiveVars *)&fp->passive_vars;
}

/* Changes status, then plays the new status's animation events. */
static void ftRemixMarinaDSPSetStatusAnimEvents(GObj *fighter_gobj, s32 status_id, u32 flags)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, flags);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Adds amount to the charge level, capped at MARINA_DSP_MAX_CHARGE. */
static void ftRemixMarinaDSPAddCharge(FTStruct *fp, s32 amount)
{
    FTRemixMarinaDSPPassiveVars *passive = ftRemixMarinaDSPPassiveVars(fp);
    s32 charge = passive->charge_level + amount;

    if (charge > MARINA_DSP_MAX_CHARGE)
    {
        charge = MARINA_DSP_MAX_CHARGE;
    }
    passive->charge_level = charge;
}

/* update_buffer_: a B press this frame sets b_press_buffer; the first frame of
 * the special (b_press_buffer == -1) only clears it. */
static void ftRemixMarinaDSPUpdateBuffer(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixMarinaDSPStatusVars *vars = ftRemixMarinaDSPStatusVars(fp);

    if (vars->b_press_buffer < 0)
    {
        vars->b_press_buffer = FALSE;
        return;
    }
    if (fp->input.pl.button_tap & B_BUTTON)
    {
        vars->b_press_buffer = TRUE;
    }
}

/* True when a B press is buffered or pressed this frame: the pull should start. */
static sb32 ftRemixMarinaDSPIsPullRequested(FTStruct *fp)
{
    return (fp->input.pl.button_tap & B_BUTTON) || (ftRemixMarinaDSPStatusVars(fp)->b_press_buffer != FALSE);
}

/* True when the pull is armed (temp variable 2) and the charge is full. */
static sb32 ftRemixMarinaDSPIsPullReady(FTStruct *fp)
{
    return (fp->motion_vars.flags.flag1 != 0) && (ftRemixMarinaDSPPassiveVars(fp)->charge_level == MARINA_DSP_MAX_CHARGE);
}

/* Returns to idle once the animation frame has reached zero (end of the animation). */
static void ftRemixMarinaDSPIdleWhenAnimEnd(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* Plays the taunt voice when taunt is held, else the default voice.
 * Non-static: the Cargo shake (ftremixmarinansp.c) plays the same voice. */
void ftRemixMarinaDSPPlayVoice(FTStruct *fp)
{
    u16 voice_id = MARINA_DSP_VOICE_FGM_ID;

    if ((fp->input.pl.button_hold & fp->input.button_mask_l) != 0)
    {
        voice_id = MARINA_DSP_VOICE_ALTERNATE_FGM_ID;
    }
    func_800269C0_275C0(voice_id);
}

s32 ftRemixMarinaDSPGetItemChargeLevel(ITStruct *ip);

/* absorb_setup_: enables the reflect bitflag and attaches the absorb hitbox. */
static void ftRemixMarinaDSPAbsorbSetup(FTStruct *fp)
{
    fp->is_reflect = TRUE;
    fp->special_coll = &sFTRemixMarinaDSPAbsorbColl;
}

/* begin_initial_ (ground and aerial): starts the special and resets its variables. */
static void ftRemixMarinaDSPBeginSetStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixMarinaDSPStatusVars *vars = ftRemixMarinaDSPStatusVars(fp);

    ftRemixMarinaDSPSetStatusAnimEvents(fighter_gobj, status_id, 0);
    ftNessSpecialLw_InitStatusVars(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
    vars->b_press_buffer = -1;
    vars->add_charge = 0;
}

/* wait_initial_ (ground and aerial): clears the temp variables and arms the absorb. */
static void ftRemixMarinaDSPWaitSetStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, MARINA_DSP_WAIT_STATUS_FLAGS);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixMarinaDSPAbsorbSetup(fp);
}

/* absorb_initial_: picks the aerial or grounded absorb, faces the reflected
 * direction and adds the stored charge. Remix tests the flag word at 0x18C for
 * the kinetic state (its comment says so); ported as ga (0x14C), the field
 * Remix reads for kinetic state elsewhere. See the notes file. */
static void ftRemixMarinaDSPAbsorbSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixMarinaDSPStatusVars *vars = ftRemixMarinaDSPStatusVars(fp);
    s32 status_id = nFTRemixMarinaStatusDSPGAbsorb;

    if (fp->ga == nMPKineticsAir)
    {
        status_id = nFTRemixMarinaStatusDSPAAbsorb;
    }
    fp->lr = fp->reflect_lr;
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, MARINA_DSP_ABSORB_STATUS_FLAGS);
    fp->motion_vars.flags.flag1 = 0;
    fp->is_reflect = TRUE;
    ftRemixMarinaDSPAddCharge(fp, vars->add_charge);
}

/* pull_initial_ (ground and aerial): pulls, or fails when there is no charge. */
static void ftRemixMarinaDSPPullSetStatus(GObj *fighter_gobj, s32 pull_id, s32 pull_fail_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = pull_id;

    if (ftRemixMarinaDSPPassiveVars(fp)->charge_level == 0)
    {
        status_id = pull_fail_id;
    }
    ftRemixMarinaDSPSetStatusAnimEvents(fighter_gobj, status_id, 0);
    ftRemixMarinaDSPPlayVoice(fp);
    fp->motion_vars.flags.flag0 = 0;
}

/* MarinaDSP.ground_initial_ (special entry): begins, or stows a held item below full charge. */
static void ftRemixMarinaDSPGroundInitialSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->item_gobj == NULL)
    {
        ftRemixMarinaDSPBeginSetStatus(fighter_gobj, nFTRemixMarinaStatusDSPGBegin);
        return;
    }
    if (ftRemixMarinaDSPPassiveVars(fp)->charge_level == MARINA_DSP_MAX_CHARGE)
    {
        return;
    }
    ftRemixMarinaDSPSetStatusAnimEvents(fighter_gobj, nFTRemixMarinaStatusDSPGStow, 0);
}

/* MarinaDSP.air_initial_ (special entry): begins, or stows a held item below full charge. */
static void ftRemixMarinaDSPAirInitialSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->item_gobj == NULL)
    {
        ftRemixMarinaDSPBeginSetStatus(fighter_gobj, nFTRemixMarinaStatusDSPABegin);
        return;
    }
    if (ftRemixMarinaDSPPassiveVars(fp)->charge_level == MARINA_DSP_MAX_CHARGE)
    {
        return;
    }
    ftRemixMarinaDSPSetStatusAnimEvents(fighter_gobj, nFTRemixMarinaStatusDSPAStow, 0);
}

static void ftRemixMarinaDSPGroundWaitSetStatus(GObj *fighter_gobj)
{
    ftRemixMarinaDSPWaitSetStatus(fighter_gobj, nFTRemixMarinaStatusDSPGWait);
}

static void ftRemixMarinaDSPAirWaitSetStatus(GObj *fighter_gobj)
{
    ftRemixMarinaDSPWaitSetStatus(fighter_gobj, nFTRemixMarinaStatusDSPAWait);
}

static void ftRemixMarinaDSPGroundPullSetStatus(GObj *fighter_gobj)
{
    ftRemixMarinaDSPPullSetStatus(fighter_gobj, nFTRemixMarinaStatusDSPGPull, nFTRemixMarinaStatusDSPGPullFail);
}

static void ftRemixMarinaDSPAirPullSetStatus(GObj *fighter_gobj)
{
    ftRemixMarinaDSPPullSetStatus(fighter_gobj, nFTRemixMarinaStatusDSPAPull, nFTRemixMarinaStatusDSPAPullFail);
}

static void ftRemixMarinaDSPGroundEndSetStatus(GObj *fighter_gobj)
{
    ftRemixMarinaDSPSetStatusAnimEvents(fighter_gobj, nFTRemixMarinaStatusDSPGEnd, 0);
}

static void ftRemixMarinaDSPAirEndSetStatus(GObj *fighter_gobj)
{
    ftRemixMarinaDSPSetStatusAnimEvents(fighter_gobj, nFTRemixMarinaStatusDSPAEnd, 0);
}

/* MarinaDSP.ground_begin_main_ */
static void ftRemixMarinaDSPGroundBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarinaDSPUpdateBuffer(fighter_gobj);
    if (ftRemixMarinaDSPIsPullReady(fp))
    {
        ftRemixMarinaDSPGroundPullSetStatus(fighter_gobj);
        return;
    }
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixMarinaDSPGroundWaitSetStatus);
}

/* MarinaDSP.air_begin_main_ */
static void ftRemixMarinaDSPAirBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarinaDSPUpdateBuffer(fighter_gobj);
    if (ftRemixMarinaDSPIsPullReady(fp))
    {
        ftRemixMarinaDSPAirPullSetStatus(fighter_gobj);
        return;
    }
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixMarinaDSPAirWaitSetStatus);
}

/* MarinaDSP.ground_wait_main_ */
static void ftRemixMarinaDSPGroundWaitProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixMarinaDSPStatusVars *vars = ftRemixMarinaDSPStatusVars(fp);

    if (ftRemixMarinaDSPIsPullRequested(fp))
    {
        ftRemixMarinaDSPGroundPullSetStatus(fighter_gobj);
        return;
    }
    ftNessSpecialLw_UpdateReleaseLag(fighter_gobj);
    if ((vars->min_frame_timer > 0) || !vars->b_not_held)
    {
        return;
    }
    ftRemixMarinaDSPGroundEndSetStatus(fighter_gobj);
}

/* MarinaDSP.air_wait_main_ */
static void ftRemixMarinaDSPAirWaitProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixMarinaDSPStatusVars *vars = ftRemixMarinaDSPStatusVars(fp);

    if (ftRemixMarinaDSPIsPullRequested(fp))
    {
        ftRemixMarinaDSPAirPullSetStatus(fighter_gobj);
        return;
    }
    ftNessSpecialLw_UpdateReleaseLag(fighter_gobj);
    if ((vars->min_frame_timer > 0) || !vars->b_not_held)
    {
        return;
    }
    ftRemixMarinaDSPAirEndSetStatus(fighter_gobj);
}

/* MarinaDSP.ground_absorb_main_ */
static void ftRemixMarinaDSPGroundAbsorbProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarinaDSPUpdateBuffer(fighter_gobj);
    if (fp->motion_vars.flags.flag1 != 0)
    {
        if (ftRemixMarinaDSPStatusVars(fp)->b_press_buffer != FALSE)
        {
            ftRemixMarinaDSPGroundPullSetStatus(fighter_gobj);
            return;
        }
        if (!(fp->input.pl.button_hold & B_BUTTON))
        {
            ftRemixMarinaDSPGroundEndSetStatus(fighter_gobj);
            return;
        }
    }
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixMarinaDSPGroundWaitSetStatus);
}

/* MarinaDSP.air_absorb_main_ */
static void ftRemixMarinaDSPAirAbsorbProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarinaDSPUpdateBuffer(fighter_gobj);
    if (fp->motion_vars.flags.flag1 != 0)
    {
        if (ftRemixMarinaDSPStatusVars(fp)->b_press_buffer != FALSE)
        {
            ftRemixMarinaDSPAirPullSetStatus(fighter_gobj);
            return;
        }
        if (!(fp->input.pl.button_hold & B_BUTTON))
        {
            ftRemixMarinaDSPAirEndSetStatus(fighter_gobj);
            return;
        }
    }
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixMarinaDSPAirWaitSetStatus);
}

/* MarinaDSP.ground_end_main_ */
static void ftRemixMarinaDSPGroundEndProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.button_tap & B_BUTTON)
    {
        ftRemixMarinaDSPGroundPullSetStatus(fighter_gobj);
        return;
    }
    ftRemixMarinaDSPIdleWhenAnimEnd(fighter_gobj);
}

/* MarinaDSP.air_end_main_ */
static void ftRemixMarinaDSPAirEndProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.button_tap & B_BUTTON)
    {
        ftRemixMarinaDSPAirPullSetStatus(fighter_gobj);
        return;
    }
    ftRemixMarinaDSPIdleWhenAnimEnd(fighter_gobj);
}

/* create_and_assign_clanpot_item_: the clanpot charge picks the item (2 Shuriken, 3 Boomerang, 4 ClanBomb, otherwise Gem). */
static GObj* ftRemixMarinaDSPMakeClanpotItem(GObj *fighter_gobj, s32 charge_level, Vec3f *pos, Vec3f *vel)
{
    switch (charge_level)
    {
    case 2:
        return itRemixShurikenMakeItem(fighter_gobj, pos, vel, 0);

    case 3:
        return itRemixBoomerangMakeItem(fighter_gobj, pos, vel, 0);

    case 4:
        return itRemixClanBombMakeItem(fighter_gobj, pos, vel, 0);

    default:
        return itRemixGemMakeItem(fighter_gobj, pos, vel, 0);
    }
}

/* create_and_assign_clanpot_item_: spawns the item at Marina's position and holds it. Direct
 * spawns skip the spawn gfx, which is what Item.skip_item_spawn_gfx_ does in Remix. */
static void ftRemixMarinaDSPCreateAndAssignClanpotItem(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *item_gobj;

    if (fp->item_gobj != NULL)
    {
        return;
    }
    item_gobj = ftRemixMarinaDSPMakeClanpotItem(fighter_gobj, ftRemixMarinaDSPPassiveVars(fp)->charge_level, &DObjGetStruct(fighter_gobj)->translate.vec.f, &vel);

    if (item_gobj != NULL)
    {
        itMainSetFighterHold(item_gobj, fighter_gobj);
    }
}

/* MarinaDSP.pull_main_ (ground and aerial). */
static void ftRemixMarinaDSPPullProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixMarinaDSPCreateAndAssignClanpotItem(fighter_gobj);
        ftRemixMarinaDSPPassiveVars(fp)->charge_level = 0;
    }
    ftRemixMarinaDSPIdleWhenAnimEnd(fighter_gobj);
}

/* MarinaDSP.stow_main_ (ground and aerial): destroys the held item and adds its charge. */
static void ftRemixMarinaDSPStowProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *item_gobj = fp->item_gobj;

    if ((fp->motion_vars.flags.flag0 != 0) && (item_gobj != NULL))
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixMarinaDSPAddCharge(fp, ftRemixMarinaDSPGetItemChargeLevel(itGetStruct(item_gobj)));
        fp->item_gobj = NULL;
        itMainDestroyItem(item_gobj);
    }
    ftRemixMarinaDSPIdleWhenAnimEnd(fighter_gobj);
}

/* ground_to_air_: switches to the aerial status (MarinaDSP.ground_collision_ transition). */
static void ftRemixMarinaDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    const FTRemixMarinaDSPTransition *transition;

    mpCommonSetFighterAir(fp);
    transition = &sFTRemixMarinaDSPGroundToAirTable[fp->status_id - nFTRemixMarinaStatusDSPGBegin];
    ftMainSetStatus(fighter_gobj, transition->status_id, fighter_gobj->anim_frame, 1.0F, transition->flags);
    ftPhysicsClampAirVelXMax(fp);
    if (transition->is_reflect)
    {
        fp->is_reflect = TRUE;
    }
}

/* air_to_ground_: switches to the grounded status (MarinaDSP.air_collision_ transition). */
static void ftRemixMarinaDSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    const FTRemixMarinaDSPTransition *transition;

    mpCommonSetFighterGround(fp);
    transition = &sFTRemixMarinaDSPAirToGroundTable[fp->status_id - nFTRemixMarinaStatusDSPABegin];
    ftMainSetStatus(fighter_gobj, transition->status_id, fighter_gobj->anim_frame, 1.0F, transition->flags);
    if (transition->is_reflect)
    {
        fp->is_reflect = TRUE;
    }
}

/* MarinaDSP.ground_collision_ */
static void ftRemixMarinaDSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMarinaDSPGroundToAir);
}

/* MarinaDSP.air_collision_ */
static void ftRemixMarinaDSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMarinaDSPAirToGround);
}

/* get_item_level_: charge gained by absorbing or stowing the item ip. Non-static:
 * the Remix item absorb patch (vanilla 0x800E5A10) calls it. */
s32 ftRemixMarinaDSPGetItemChargeLevel(ITStruct *ip)
{
    if (ip->kind == MARINA_DSP_BOWSER_BOMB_ITEM_ID)
    {
        return 2;
    }
    if (ip->kind == MARINA_DSP_TURNIP_ITEM_ID)
    {
        if (ip->attack_coll.damage == MARINA_DSP_TURNIP_STITCH_DAMAGE)
        {
            return 4;
        }
        if (ip->attack_coll.damage == MARINA_DSP_TURNIP_DOT_DAMAGE)
        {
            return 3;
        }
        if (ip->attack_coll.damage == MARINA_DSP_TURNIP_WINK_DAMAGE)
        {
            return 2;
        }
    }
    return sFTRemixMarinaDSPItemChargeTable[ip->kind];
}

/* Stores the charge a reflected hit adds on absorb (status_vars add_charge, 0x0B24).
 * Non-static: the Remix absorb patch in ftMainSearchHitItem calls it. */
void ftRemixMarinaDSPSetAddCharge(FTStruct *fp, s32 add_charge)
{
    ftRemixMarinaDSPStatusVars(fp)->add_charge = add_charge;
}

/* get_projectile_level_ for a charge shot: 3 at full charge, 1 below charge size 4, else 2.
 * The size is the N64 0x2A4 field, which is charge_size in the port layout. */
static s32 ftRemixMarinaDSPGetChargeShotLevel(WPStruct *wp)
{
    s32 charge_size = wp->weapon_vars.charge_shot.charge_size;

    if (charge_size == MARINA_DSP_CHARGE_SHOT_FULL)
    {
        return 3;
    }
    if ((u32)charge_size < 4)
    {
        return 1;
    }
    return 2;
}

/* get_projectile_level_: charge gained by absorbing the projectile wp. */
static s32 ftRemixMarinaDSPGetProjectileChargeLevel(WPStruct *wp)
{
    switch (wp->kind)
    {
    case MARINA_DSP_CHARGE_SHOT_PROJECTILE_ID:
        return ftRemixMarinaDSPGetChargeShotLevel(wp);
    case MARINA_DSP_YOSHI_EGG_PROJECTILE_ID:
    case MARINA_DSP_LINK_BOOMERANG_PROJECTILE_ID:
    case MARINA_DSP_PIKACHU_THUNDER_HEAD_PROJECTILE_ID:
    case MARINA_DSP_PIKACHU_THUNDER_TAIL_PROJECTILE_ID:
        return 2;
    case MARINA_DSP_ARWING_SHOT_PROJECTILE_ID:
    case MARINA_DSP_GOLDENGUN_BULLET_PROJECTILE_ID:
    case MARINA_DSP_PIRATELAND_CANNONBALL_PROJECTILE_ID:
        return 4;
    default:
        return 1;
    }
}

/* Remix checks the reflect hitbox (fp->special_coll) for custom reflect type 3 with a
 * non-zero custom id; custom id 0 is the franklin badge. */
static sb32 ftRemixMarinaDSPIsCustomReflect(FTStruct *fp)
{
    s32 kind;

    if (fp->special_coll == NULL)
    {
        return FALSE;
    }
    kind = fp->special_coll->kind;
    if ((kind & 0xFFFF) != MARINA_DSP_CUSTOM_REFLECT_TYPE)
    {
        return FALSE;
    }
    return (kind >> 16) != 0;
}

/* Remix item absorb patch (vanilla 0x800E5A10, ftMainSearchHitItem): sets the charge of the absorbed item ip. */
void ftRemixMarinaDSPAbsorbItemCharge(FTStruct *fp, ITStruct *ip)
{
    if (ftRemixMarinaDSPIsCustomReflect(fp) != FALSE)
    {
        ftRemixMarinaDSPSetAddCharge(fp, ftRemixMarinaDSPGetItemChargeLevel(ip));
    }
}

/* Remix projectile absorb patch (vanilla 0x800E5330, ftMainSearchHitWeapon): sets the charge of the projectile wp. */
void ftRemixMarinaDSPAbsorbProjectileCharge(FTStruct *fp, WPStruct *wp)
{
    if (ftRemixMarinaDSPIsCustomReflect(fp) != FALSE)
    {
        ftRemixMarinaDSPSetAddCharge(fp, ftRemixMarinaDSPGetProjectileChargeLevel(wp));
    }
}

/* Remix address -> port routine for every MarinaDSP routine its action array
 * or special tables reference. */
const FTRemixProc dFTRemixMarinaDSPProcs[] =
{
    { REMIX_SYM_MarinaDSP__air_absorb_main_, ftRemixMarinaDSPAirAbsorbProcUpdate },
    { REMIX_SYM_MarinaDSP__air_begin_main_, ftRemixMarinaDSPAirBeginProcUpdate },
    { REMIX_SYM_MarinaDSP__air_collision_, ftRemixMarinaDSPAirProcMap },
    { REMIX_SYM_MarinaDSP__air_end_main_, ftRemixMarinaDSPAirEndProcUpdate },
    { REMIX_SYM_MarinaDSP__air_initial_, ftRemixMarinaDSPAirInitialSetStatus },
    { REMIX_SYM_MarinaDSP__air_wait_main_, ftRemixMarinaDSPAirWaitProcUpdate },
    { REMIX_SYM_MarinaDSP__ground_absorb_main_, ftRemixMarinaDSPGroundAbsorbProcUpdate },
    { REMIX_SYM_MarinaDSP__ground_begin_main_, ftRemixMarinaDSPGroundBeginProcUpdate },
    { REMIX_SYM_MarinaDSP__ground_collision_, ftRemixMarinaDSPGroundProcMap },
    { REMIX_SYM_MarinaDSP__ground_end_main_, ftRemixMarinaDSPGroundEndProcUpdate },
    { REMIX_SYM_MarinaDSP__ground_initial_, ftRemixMarinaDSPGroundInitialSetStatus },
    { REMIX_SYM_MarinaDSP__ground_wait_main_, ftRemixMarinaDSPGroundWaitProcUpdate },
    { REMIX_SYM_MarinaDSP__pull_main_, ftRemixMarinaDSPPullProcUpdate },
    { REMIX_SYM_MarinaDSP__stow_main_, ftRemixMarinaDSPStowProcUpdate }
};

const s32 dFTRemixMarinaDSPProcsNum = ARRAY_COUNT(dFTRemixMarinaDSPProcs);

#endif
