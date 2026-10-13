/**
 * ftremixcrashdsp.c - C ports of Smash Remix Crash Dig routines (CrashDSP scope,
 * .toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm).
 *
 * Dig keeps its timer, attached Dig GFX object and move timer offset in the
 * parent's status_vars slots 0x0B18, 0x0B1C and 0x0B20 (FTRemixCrashDSPStatusVars).
 * Dig's temp variables 1/2/3 are motion_vars.flags.flag0/1/2 (0x17C/0x180/0x184).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmrumble.h>
#include <mp/mpcommon.h>

extern void *func_800269C0_275C0(u16 id);

enum
{
    nFTRemixCrashStatusDSPBegin = 0xE5,
    nFTRemixCrashStatusDSPWait = 0xE6,
    nFTRemixCrashStatusDSPTurn = 0xE7,
    nFTRemixCrashStatusDSPEnd = 0xE8,
    nFTRemixCrashStatusDSPDive = 0xE9,
    nFTRemixCrashStatusDSPAirDive = 0xEA
};

#define CRASH_DSP_TRANSITION_FLAGS 0x0007           /* keeps attached gfx, gfx routines, hitboxes */
#define CRASH_DSP_MAX_TIME 180
#define CRASH_DSP_MIN_SPEED 2.0F
#define CRASH_DSP_MAX_SPEED 40.0F
#define CRASH_DSP_ACCELERATION 8.0F
#define CRASH_DSP_STICK_SPEED_MULTIPLIER 0.5F
#define CRASH_DSP_STICK_DEADZONE 11
#define CRASH_DSP_DIVE_Y_SPEED -100.0F
#define CRASH_DSP_DIVE_AIR_Y_SPEED 30.0F
#define CRASH_DSP_DIVE_AIR_FRICTION 1.75F
#define CRASH_DSP_DIVE_FALL_SPEED 100.0F
#define CRASH_DSP_DIVE_GRAVITY 0.5F
#define CRASH_DSP_DIVE_GRAVITY_2 7.0F               /* used while temp variable 2 is set */
#define CRASH_DSP_END_Y_SPEED 60.0F
#define CRASH_DSP_END_Y_SPEED_EDGE 40.0F
#define CRASH_DSP_BEGIN_FROM_DIVE_FRAME 20.0F
#define CRASH_DSP_DIG_ANIM_WAIT 3.0F
#define CRASH_DSP_DIG_SCROLL_PER_SPEED 0.0029296875F /* 0x3B400000 */
#define CRASH_DSP_DIG_GFX_FILE_OFFSET 0x0CC0
#define CRASH_DSP_DIG_SMOKE_OFFSET_Y -80.0F
#define CRASH_DSP_DIG_GFX_XOBJ_KIND 0x1C
#define CRASH_DSP_DIG_GFX_END_XOBJ_UNK05 0x02
#define CRASH_DSP_DUST_OFFSET_X 140.0F
#define CRASH_DSP_DUST_FRAME_MASK 0x0F
#define CRASH_DSP_SFX_RUMBLE_FRAME_MASK 0x1F
#define CRASH_DSP_SFX_DIG 132
#define CRASH_DSP_SFX_BEGIN_FROM_DIVE_1 45
#define CRASH_DSP_SFX_BEGIN_FROM_DIVE_2 220
#define CRASH_DSP_RUMBLE_ID 0
#define CRASH_DSP_RUMBLE_LENGTH 30
#define CRASH_DSP_DROP_THROUGH_FLOOR_FLAG 0x4000

typedef struct FTRemixCrashDSPStatusVars
{
    s32 timer;             /* 0x0B18: DSP timer */
    GObj *dig_gfx;         /* 0x0B1C: attached Dig GFX object, NULL when none */
    s32 move_timer_offset; /* 0x0B20: cadence offset of the dig dust and sfx */
} FTRemixCrashDSPStatusVars;

/* dig_gfx_main_: the vanilla effect update plus the smoke puff when the Dig GFX ends.
 * REMIX-TODO(Size): the Size.multiplier_table scale of the player is not applied. */
static void ftRemixCrashDSPDigGfxProcUpdate(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    DObj *root;
    DObj *topjoint;
    Vec3f smoke_pos;

    if (ep->is_pause_effect)
    {
        return;
    }
    gcPlayAnimAll(effect_gobj);

    if (effect_gobj->anim_frame > 0.0F)
    {
        return;
    }
    root = DObjGetStruct(effect_gobj);
    topjoint = (DObj *)root->user_data.p;
    smoke_pos = topjoint->translate.vec.f;
    smoke_pos.y += CRASH_DSP_DIG_SMOKE_OFFSET_Y;
    smoke_pos.z = 0.0F;
    efManagerDustHeavyDoubleMakeEffect(&smoke_pos, 1, 1.0F);

    efManagerSetPrevStructAlloc(ep);
    gcEjectGObj(effect_gobj);
}

/* Dig GFX effect descriptor (dig_gfx_struct), laid out like Link's spin attack effect: the
 * Remix data holds flags/dl_link 0x060F, the matrix kinds 0x50/0x1A and 0x1C, the update and
 * render procs and the DObjDesc, MObjSub, AnimJoint and MatAnimJoint offsets inside the file.
 * The file pointer is filled per call. */
/* Word 0xCC0 of Crash's second special file is the pointer to the Dig GFX file (CRASH_DIG_GFX);
 * the effect reads its data offsets from there. */
static void *sFTRemixCrashDigFile;

static EFDesc sFTRemixCrashDigGfxDesc =
{
    .flags = 0x06,
    .dl_link = 0x0F,
    .transform_types1 = { 0x50, 0x1A, 0x00 },
    .transform_types2 = { 0x1C, 0x00, 0x00 },
    .proc_update = ftRemixCrashDSPDigGfxProcUpdate,
    .proc_display = gcDrawDObjTreeForGObj,
    .o_dobjsetup = 0x230,
    .o_mobjsub = 0x340,
    .o_anim_joint = 0x378,
    .o_matanim_joint = 0x394
};

/* Overlays the Dig fields on the parent's status_vars union (0x0B18 onward). */
static FTRemixCrashDSPStatusVars *ftRemixCrashDSPVars(FTStruct *fp)
{
    return (FTRemixCrashDSPStatusVars *)&fp->status_vars;
}

static f32 ftRemixCrashDSPAbs(f32 value)
{
    return (value < 0.0F) ? -value : value;
}

static void ftRemixCrashDSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* ftMainSetStatus with frame speed 1.0 followed by the animation events, the
 * shared body of every CrashDSP.*_initial_ routine. */
static void ftRemixCrashDSPSetStatus(GObj *fighter_gobj, s32 status_id, f32 frame_begin)
{
    ftMainSetStatus(fighter_gobj, status_id, frame_begin, 1.0F, CRASH_DSP_TRANSITION_FLAGS);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Creates the Dig GFX effect and attaches its root to the player topjoint.
 * Returns NULL when the effect could not be created. */
static GObj *ftRemixCrashDSPCreateDigGfx(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *dig_gfx;
    EFStruct *dig_effect;
    DObj *root;
    DObj *topjoint;

    sFTRemixCrashDigFile = PORT_RESOLVE(*(u32 *)((u8 *)*fp->data->p_file_special2 + CRASH_DSP_DIG_GFX_FILE_OFFSET));
    sFTRemixCrashDigGfxDesc.file_head = &sFTRemixCrashDigFile;
    dig_gfx = efManagerMakeEffectNoForce(&sFTRemixCrashDigGfxDesc);
    if (dig_gfx == NULL)
    {
        return NULL;
    }

    dig_effect = (EFStruct *)dig_gfx->user_data.p;
    dig_effect->fighter_gobj = fighter_gobj;

    root = (DObj *)dig_gfx->obj;
    topjoint = fp->joints[nFTPartsJointTopN];
    root->user_data.p = topjoint;
    root->rotate.vec.f.y = topjoint->rotate.vec.f.y;

    return dig_gfx;
}

/* setup_dig_gfx_: marks the player as having an effect attached and switches
 * the Dig GFX render type so it can be scaled. */
static void ftRemixCrashDSPSetupDigGfx(FTStruct *fp, GObj *dig_gfx)
{
    DObj *root = (DObj *)dig_gfx->obj;

    fp->is_effect_attach = TRUE;
    root->xobjs[1]->kind = CRASH_DSP_DIG_GFX_XOBJ_KIND;
}

/* dig_graphic_update_: slope tilt of the Dig GFX and the texture scroll that
 * follows the player's horizontal speed. */
static void ftRemixCrashDSPDigGraphicUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *dig_gfx = ftRemixCrashDSPVars(fp)->dig_gfx;
    f32 slope_angle = ftKirbySpecialLwGetGroundAxisYaw(fp);
    DObj *root;
    MObj *mobj;

    if (dig_gfx == NULL)
    {
        return;
    }

    root = (DObj *)dig_gfx->obj;
    root->rotate.vec.f.z = slope_angle;

    mobj = root->child->mobj;
    mobj->sub.trav -= ftRemixCrashDSPAbs(fp->physics.vel_air.x) * CRASH_DSP_DIG_SCROLL_PER_SPEED;
}

/* Spawns one dust puff behind the player. */
static void ftRemixCrashDSPMakeDust(FTStruct *fp)
{
    Vec3f dust_pos;

    dust_pos.x = fp->coll_data.p_translate->x - (fp->lr * CRASH_DSP_DUST_OFFSET_X);
    dust_pos.y = fp->coll_data.p_translate->y;
    dust_pos.z = 0.0F;
    efManagerDustLightMakeEffect(&dust_pos, fp->lr, 1.0F);
}

/* dig_update_: dust every 16 frames while moving, sound and rumble every 32. */
static void ftRemixCrashDSPDigUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashDSPStatusVars *vars = ftRemixCrashDSPVars(fp);
    s32 cadence;

    if (vars->dig_gfx == NULL)
    {
        return;
    }
    if (fp->physics.vel_air.x == 0.0F)
    {
        return;
    }

    cadence = vars->timer - vars->move_timer_offset;
    if ((cadence & CRASH_DSP_DUST_FRAME_MASK) != 0)
    {
        return;
    }
    ftRemixCrashDSPMakeDust(fp);

    if ((cadence & CRASH_DSP_SFX_RUMBLE_FRAME_MASK) != 0)
    {
        return;
    }
    func_800269C0_275C0(CRASH_DSP_SFX_DIG);
    gmRumbleSetPlayerRumbleParams(fp->player, CRASH_DSP_RUMBLE_ID, CRASH_DSP_RUMBLE_LENGTH);
}

/* Held B button, as checked by DSPWait and DSPTurn. */
static sb32 ftRemixCrashDSPIsBHeld(FTStruct *fp)
{
    return (fp->input.pl.button_hold & B_BUTTON) != 0;
}

/* True when the stick is at least the dead zone away from neutral, and points
 * against DIRECTION (the turn check of DSPWait and DSPTurn). */
static sb32 ftRemixCrashDSPStickReverses(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_magnitude = (stick_x < 0) ? -stick_x : stick_x;

    if (stick_magnitude < CRASH_DSP_STICK_DEADZONE)
    {
        return FALSE;
    }
    return (stick_x < 0) != (fp->lr < 0);
}

/* DSP air friction with DIVE_AIR_FRICTION in place of the attribute value
 * (the asm passes a stack struct whose air_friction slot is 1.75F). */
static void ftRemixCrashDSPAirDiveFriction(FTStruct *fp)
{
    FTAttributes friction_attr = *fp->attr;

    friction_attr.air_friction = CRASH_DSP_DIVE_AIR_FRICTION;
    ftPhysicsApplyAirVelXFriction(fp, &friction_attr);
}

/* DSPBegin on solid floors, and the Dig GFX creation that goes with it. */
static void ftRemixCrashDSPBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *dig_gfx;

    ftRemixCrashDSPSetStatus(fighter_gobj, nFTRemixCrashStatusDSPBegin, 0.0F);
    dig_gfx = ftRemixCrashDSPCreateDigGfx(fighter_gobj);
    if (dig_gfx != NULL)
    {
        /* REMIX-TODO(PokemonAnnouncer): PokemonAnnouncer.burrow_announcement_ is skipped. */
        ftRemixCrashDSPSetupDigGfx(fp, dig_gfx);
        ftRemixCrashDSPVars(fp)->dig_gfx = dig_gfx;
    }
    ftRemixCrashDSPClearTempVars(fp);
}

/* DSPDive on drop-through platforms: the fighter falls through the floor. */
static void ftRemixCrashDSPDiveInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashDSPSetStatus(fighter_gobj, nFTRemixCrashStatusDSPDive, 0.0F);
    ftRemixCrashDSPClearTempVars(fp);
    fp->physics.vel_air.x = 0.0F;
}

/* DSPAirDive: the aerial dive, starting at DIVE_AIR_Y_SPEED. */
static void ftRemixCrashDSPDiveAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashDSPSetStatus(fighter_gobj, nFTRemixCrashStatusDSPAirDive, 0.0F);
    ftRemixCrashDSPClearTempVars(fp);
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = CRASH_DSP_DIVE_AIR_Y_SPEED;
}

/* DSPWait: resets the topjoint rotation of the Dig GFX.
 * REMIX-TODO(animation): track_dig_wait is not applied (0x8000BD1C has no decomp name). */
static void ftRemixCrashDSPWaitInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *dig_gfx = ftRemixCrashDSPVars(fp)->dig_gfx;
    DObj *root;

    ftRemixCrashDSPSetStatus(fighter_gobj, nFTRemixCrashStatusDSPWait, 0.0F);
    if (dig_gfx == NULL)
    {
        return;
    }

    root = (DObj *)dig_gfx->obj;
    root->rotate.vec.f.y = fp->joints[nFTPartsJointTopN]->rotate.vec.f.y;
}

/* DSPTurn. REMIX-TODO(animation): track_dig_turn is not applied to the Dig GFX. */
static void ftRemixCrashDSPTurnInitial(GObj *fighter_gobj)
{
    ftRemixCrashDSPSetStatus(fighter_gobj, nFTRemixCrashStatusDSPTurn, 0.0F);
}

/* DSPEnd: stops the Dig GFX from following the player and sets the fall
 * speed. A slide-off end drops at END_Y_SPEED_EDGE instead of END_Y_SPEED. */
static void ftRemixCrashDSPEndInitial(GObj *fighter_gobj, sb32 is_slide_off)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *dig_gfx = ftRemixCrashDSPVars(fp)->dig_gfx;
    DObj *root;

    /* REMIX-TODO(PokemonAnnouncer): PokemonAnnouncer.dig_announcement_ is skipped. */
    ftRemixCrashDSPSetStatus(fighter_gobj, nFTRemixCrashStatusDSPEnd, 0.0F);
    if (dig_gfx != NULL)
    {
        root = (DObj *)dig_gfx->obj;
        /* REMIX-TODO(dig gfx): the asm also copies the topjoint translation to
         * dig_gfx GObj +0x4C/+0x50/+0x54 (not named in the decomp); skipped.
         * REMIX-TODO(animation): track_dig_end is not applied. */
        root->rotate.vec.f.y = fp->joints[nFTPartsJointTopN]->rotate.vec.f.y;
        root->xobjs[0]->unk05 = CRASH_DSP_DIG_GFX_END_XOBJ_UNK05;
    }

    fp->physics.vel_air.y = is_slide_off ? CRASH_DSP_END_Y_SPEED_EDGE : CRASH_DSP_END_Y_SPEED;
    mpCommonSetFighterAir(fp);
}

/* DSPEnd with the slide-off flag set, used as the ground collision transition. */
static void ftRemixCrashDSPSlideOffEndInitial(GObj *fighter_gobj)
{
    ftRemixCrashDSPEndInitial(fighter_gobj, TRUE);
}

/* DSPBegin when a dive lands on a floor, starting at frame 20 with two sfx. */
static void ftRemixCrashDSPBeginInitialFromDive(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *dig_gfx;

    mpCommonSetFighterGround(fp);
    ftRemixCrashDSPSetStatus(fighter_gobj, nFTRemixCrashStatusDSPBegin, CRASH_DSP_BEGIN_FROM_DIVE_FRAME);
    dig_gfx = ftRemixCrashDSPCreateDigGfx(fighter_gobj);
    if (dig_gfx != NULL)
    {
        DObj *root = (DObj *)dig_gfx->obj;

        ftRemixCrashDSPSetupDigGfx(fp, dig_gfx);
        ftRemixCrashDSPVars(fp)->dig_gfx = dig_gfx;
        root->child->anim_wait = CRASH_DSP_DIG_ANIM_WAIT;
        func_800269C0_275C0(CRASH_DSP_SFX_BEGIN_FROM_DIVE_1);
        func_800269C0_275C0(CRASH_DSP_SFX_BEGIN_FROM_DIVE_2);
    }
    ftRemixCrashDSPClearTempVars(fp);
}

/* CrashDSP.initial_: special entry for DSP on the ground. Starts DSPDive on
 * drop-through platforms and DSPBegin on solid floors. */
static void ftRemixCrashDSPInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashDSPStatusVars *vars = ftRemixCrashDSPVars(fp);

    vars->dig_gfx = NULL;
    vars->move_timer_offset = -1;
    if (fp->coll_data.floor_flags & CRASH_DSP_DROP_THROUGH_FLOOR_FLAG)
    {
        ftRemixCrashDSPDiveInitial(fighter_gobj);
        return;
    }
    ftRemixCrashDSPBeginInitial(fighter_gobj);
}

/* CrashDSP.begin_main_: waits for the begin animation to end, then DSPWait. */
static void ftRemixCrashDSPBeginMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashDSPDigGraphicUpdate(fighter_gobj);
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftRemixCrashDSPVars(fp)->timer = CRASH_DSP_MAX_TIME;
    ftRemixCrashDSPWaitInitial(fighter_gobj);
}

/* CrashDSP.wait_main_: holds DSPWait until B is released, the timer ends, or
 * the stick asks to turn. */
static void ftRemixCrashDSPWaitMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashDSPStatusVars *vars = ftRemixCrashDSPVars(fp);

    ftRemixCrashDSPDigGraphicUpdate(fighter_gobj);
    ftRemixCrashDSPDigUpdate(fighter_gobj);

    vars->timer -= 1;
    if ((vars->timer == 0) || !ftRemixCrashDSPIsBHeld(fp))
    {
        ftRemixCrashDSPEndInitial(fighter_gobj, FALSE);
        return;
    }
    if (ftRemixCrashDSPStickReverses(fp))
    {
        ftRemixCrashDSPTurnInitial(fighter_gobj);
    }
}

/* CrashDSP.turn_main_: flips DIRECTION once when temp variable 2 is set, then
 * returns to DSPWait when the turn animation ends. */
static void ftRemixCrashDSPTurnMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashDSPStatusVars *vars = ftRemixCrashDSPVars(fp);

    ftRemixCrashDSPDigGraphicUpdate(fighter_gobj);
    ftRemixCrashDSPDigUpdate(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        fp->lr = -fp->lr;
        fp->motion_vars.flags.flag1 = 0;
    }

    vars->timer -= 1;
    if ((vars->timer == 0) || !ftRemixCrashDSPIsBHeld(fp))
    {
        ftRemixCrashDSPEndInitial(fighter_gobj, FALSE);
        return;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixCrashDSPWaitInitial(fighter_gobj);
    }
}

/* CrashDSP.dive_main_: once temp variable 2 is set, drops at DIVE_Y_SPEED and
 * ignores the floor it left. */
static void ftRemixCrashDSPDiveMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    fp->coll_data.ignore_line_id = fp->coll_data.floor_line_id;
    fp->physics.vel_air.y = CRASH_DSP_DIVE_Y_SPEED;
    mpCommonSetFighterAir(fp);
}

/* CrashDSP.physics_: moves toward the stick speed on the ground, with the
 * acceleration, clamp and move timer offset of the asm. */
static void ftRemixCrashDSPPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashDSPStatusVars *vars = ftRemixCrashDSPVars(fp);
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_magnitude = (stick_x < 0) ? -stick_x : stick_x;
    f32 target_x;
    f32 x_diff;
    f32 x_vel;

    if (stick_magnitude < CRASH_DSP_STICK_DEADZONE)
    {
        if (ftRemixCrashDSPAbs(fp->physics.vel_air.x) <= CRASH_DSP_MIN_SPEED)
        {
            vars->move_timer_offset = -1;
            fp->physics.vel_air.x = 0.0F;
            return;
        }
        target_x = 0.0F;
    }
    else
    {
        target_x = stick_x * CRASH_DSP_STICK_SPEED_MULTIPLIER;
    }

    if (vars->move_timer_offset < 0)
    {
        vars->move_timer_offset = vars->timer & 0x1F;
    }

    x_vel = fp->physics.vel_air.x;
    x_diff = target_x - x_vel;
    if (ftRemixCrashDSPAbs(x_diff) >= 2.0F)
    {
        x_vel += (x_diff >= 0.0F) ? CRASH_DSP_ACCELERATION : -CRASH_DSP_ACCELERATION;
    }
    if (ftRemixCrashDSPAbs(x_vel) > CRASH_DSP_MAX_SPEED)
    {
        x_vel = (x_vel < 0.0F) ? -CRASH_DSP_MAX_SPEED : CRASH_DSP_MAX_SPEED;
    }

    fp->physics.vel_air.x = x_vel;
    fp->physics.vel_ground.x = x_vel * fp->lr;
}

/* CrashDSP.dive_physics_: the dive movement applies only in the air. */
static void ftRemixCrashDSPDiveAirPhysics(GObj *fighter_gobj);

static void ftRemixCrashDSPDivePhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsGround)
    {
        ftRemixCrashDSPDiveAirPhysics(fighter_gobj);
    }
}

/* CrashDSP.dive_air_physics_: a copy of 0x800D90E0 (air drift) with Dig's
 * gravity, fall speed and air friction. Only the asm's splice points are
 * ported; the fast fall branch of the copied code is replaced by the dive
 * gravity. */
static void ftRemixCrashDSPDiveAirPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;
    f32 gravity = fp->motion_vars.flags.flag1 ? CRASH_DSP_DIVE_GRAVITY_2 : CRASH_DSP_DIVE_GRAVITY;

    ftPhysicsApplyGravityClampTVel(fp, gravity, CRASH_DSP_DIVE_FALL_SPEED);
    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) != FALSE)
    {
        return;
    }
    ftPhysicsClampAirVelXStickDefault(fp, attr);
    ftRemixCrashDSPAirDiveFriction(fp);
}

/* CrashDSP.collision_: ground collision for DSPBegin, DSPWait and DSPTurn. */
static void ftRemixCrashDSPCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixCrashDSPSlideOffEndInitial);
}

/* CrashDSP.dive_collision_: landing on the ground or in the air. */
static void ftRemixCrashDSPDiveCollision(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    mpCommonProcFighterLanding(fighter_gobj, ftRemixCrashDSPBeginInitialFromDive);
}

/* Remix address -> port routine for every CrashDSP routine in the worklist. */
const FTRemixProc dFTRemixCrashDSPProcs[] =
{
    { REMIX_SYM_CrashDSP__begin_main_, ftRemixCrashDSPBeginMain },
    { REMIX_SYM_CrashDSP__collision_, ftRemixCrashDSPCollision },
    { REMIX_SYM_CrashDSP__dive_air_initial_, ftRemixCrashDSPDiveAirInitial },
    { REMIX_SYM_CrashDSP__dive_air_physics_, ftRemixCrashDSPDiveAirPhysics },
    { REMIX_SYM_CrashDSP__dive_collision_, ftRemixCrashDSPDiveCollision },
    { REMIX_SYM_CrashDSP__dive_main_, ftRemixCrashDSPDiveMain },
    { REMIX_SYM_CrashDSP__dive_physics_, ftRemixCrashDSPDivePhysics },
    { REMIX_SYM_CrashDSP__initial_, ftRemixCrashDSPInitial },
    { REMIX_SYM_CrashDSP__physics_, ftRemixCrashDSPPhysics },
    { REMIX_SYM_CrashDSP__turn_main_, ftRemixCrashDSPTurnMain },
    { REMIX_SYM_CrashDSP__wait_main_, ftRemixCrashDSPWaitMain }
};

/* CrashDSP.dig_ecb_patch_ (0x800E6F4C in ftMainSetStatus): the ECB shrinks while underground.
 * Registered as Crash's ecb_override (PortFTECBOverrideFn). */
int ftRemixCrashDSPECBOverride(FTStruct *fp, int next_status_id, float *out_upper, float *out_middle)
{
    if ((next_status_id == nFTRemixCrashStatusDSPWait) || (next_status_id == nFTRemixCrashStatusDSPTurn))
    {
        *out_upper = 150.0F;
        *out_middle = 80.0F;
    }
    else
    {
        *out_upper = 320.0F;
        *out_middle = 190.0F;
    }
    return TRUE;
}

const s32 dFTRemixCrashDSPProcsNum = ARRAY_COUNT(dFTRemixCrashDSPProcs);

#endif
