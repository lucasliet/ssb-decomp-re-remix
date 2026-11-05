/**
 * ftremixcrashnsp.c - C ports of Smash Remix Crash neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm, scope CrashNSP).
 *
 * Kirby and J Kirby copy this move with their own action ids (Kirby.asm
 * add_new_action_params CRASH_NSP_*). The shield-hit routine is installed on
 * the fighter through proc_shield (N64 0x09F4), so it is a C function pointer.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include "fighter_registry.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <sys/objdisplay.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <it/item.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixCrashStatusNSPG = 0xDF,
    nFTRemixCrashStatusNSPA = 0xE0,
    nFTRemixCrashStatusNSPGBlocked = 0xE1,
    nFTRemixCrashStatusNSPABlocked = 0xE2
};

/* Kirby copies: 0xDC + 83 (KIRBY parent size 0x67C / 0x14) + index 127..130 of the KIRBY add_new_action calls
 * (Kirby.asm lines 307-310). Derived, not read from remix.z64. See the notes file. */
enum
{
    nFTRemixCrashKirbyStatusNSPG = 0x1AE,
    nFTRemixCrashKirbyStatusNSPA = 0x1AF,
    nFTRemixCrashKirbyStatusNSPGBlocked = 0x1B0,
    nFTRemixCrashKirbyStatusNSPABlocked = 0x1B1
};

/* Flags of the transition routines (asm argument 4 = 0x0807), named by the asm comment:
 * 3C FGM = LOOPSFX, attached gfx = EFFECT, gfx routines = COLANIM, hitboxes = HIT. */
#define CRASH_NSP_CONTINUE_FLAGS (FTSTATUS_PRESERVE_LOOPSFX | FTSTATUS_PRESERVE_EFFECT | FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_HIT)

#define CRASH_NSP_INITIAL_GROUND_VEL_SCALE 1.5F   /* 0x3FC00000 */
#define CRASH_NSP_INITIAL_AIR_VEL_SCALE 1.25F     /* 0x3FA00000 */
#define CRASH_NSP_SHIELD_AIR_VEL_X_SCALE 0.25F    /* 0x3E800000 */
#define CRASH_NSP_SHIELD_VEL_Y_SCALE 0.5F         /* 0x3F000000 */
#define CRASH_NSP_SHIELD_GROUND_VEL_X_SCALE 0.5F  /* 0x3F000000 */
#define CRASH_NSP_MIN_SPEED 2.0F                  /* 0x40000000 */
#define CRASH_NSP_MAX_SPEED 78.0F                 /* 0x429C0000 */
#define CRASH_NSP_ACCELERATION 2.0F               /* 0x40000000 */
#define CRASH_NSP_NEUTRAL_ACCELERATION 1.25F      /* 0x3FA00000: used by the grounded routine when the stick is neutral */
#define CRASH_NSP_END_ACCELERATION 4.5F           /* 0x40900000 */
#define CRASH_NSP_G_SPEED_MULTIPLIER 0.625F       /* 0x3F200000 */
#define CRASH_NSP_A_SPEED_MULTIPLIER 0.5F         /* 0x3F000000 */
#define CRASH_NSP_STICK_NEUTRAL_RANGE 11          /* |stick_x| below this counts as neutral */
#define CRASH_NSP_GRAVITY 2.625F                  /* 0x40280000 */
#define CRASH_NSP_KIRBY_GRAVITY 2.109375F         /* 0x40070000 */
#define CRASH_NSP_FALL_SPEED 48.0F                /* 0x42400000 */
#define CRASH_NSP_JUMP_SPEED 72.0F                /* 0x42900000 */
#define CRASH_NSP_KIRBY_JUMP_SPEED 64.0F          /* 0x42800000 */
#define CRASH_NSP_SHIELD_HITBOX_NUM 3             /* Character.get_hitbox_collision_flags_ loops over three hitboxes */
#define CRASH_NSP_JUMP_SMOKE_LR 1

/* Per-fighter values that Remix keeps in status vars 0x0B18..0x0B24 while the blocked
 * routines run, which have no typed field in FTStatusVars. */
typedef struct FTRemixCrashNSPShieldVars
{
    f32 vel_air_x;
    f32 vel_air_y;
    f32 vel_ground_x;
    sb32 is_hitlag_over;
} FTRemixCrashNSPShieldVars;

static FTRemixCrashNSPShieldVars sFTRemixCrashNSPShieldVars[MAXCONTROLLERS];

static void ftRemixCrashNSPShieldHit(GObj *fighter_gobj);

/* Shared switch between Crash and Kirby: the fighter's fkind picks the status id. */
static void ftRemixCrashNSPChangeStatus(GObj *fighter_gobj, s32 crash_status, s32 kirby_status, f32 frame, u32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = (fp->fkind == REMIX_ID_CRASH) ? crash_status : kirby_status;

    ftMainSetStatus(fighter_gobj, status_id, frame, 1.0F, flags);
}

#define CRASH_NSP_KIRBY_SPIN_FILE_OFFSET 0x0CC4 /* asm create_spin_gfx_: Kirby reads the spin file at Crash file 7 + 0xCC4 */
#define CRASH_NSP_SPIN_COSTUME_COUNT 7

/* Spin GFX descriptor (spin_gfx_struct), Link's Up Special layout as in the Dig GFX. The file pointer is set per call. */
static void *sFTRemixCrashNSPSpinKirbyFile;
static EFDesc sFTRemixCrashNSPSpinGfxDesc =
{
    .flags = 0x06,
    .dl_link = 0x0F,
    .transform_types1 = { 0x50, 0x1A, 0x00 },
    .transform_types2 = { 0x1C, 0x00, 0x00 },
    .proc_update = efManagerHaveStructProcUpdate,
    .proc_display = gcDrawDObjTreeForGObj,
    .o_dobjsetup = 0x708,
    .o_mobjsub = 0x838,
    .o_anim_joint = 0x894,
    .o_matanim_joint = 0x8B4
};

/* spin_gfx_table of CrashSpecial.asm: per costume, top prim, top env, bottom prim, bottom env. */
static const u32 sFTRemixCrashNSPSpinColours[CRASH_NSP_SPIN_COSTUME_COUNT][4] =
{
    { 0xFFFFFFB0, 0xFF4E00FF, 0x42001FA0, 0x120015FF },
    { 0xFFFFFFB0, 0xFF0000FF, 0x52002FA0, 0x220005FF },
    { 0xFFFFFFB0, 0x60E800FF, 0x62002FA0, 0x120015FF },
    { 0xFFFFFFB0, 0xB0D0FFFF, 0x003070A0, 0x00080FFF },
    { 0xFFFFFFB0, 0xFFE000FF, 0x404040A0, 0x080808FF },
    { 0xFFFFFFB0, 0x4C3020FF, 0x004058A0, 0x240008FF },
    { 0xFFFFFFB0, 0xFF7400FF, 0x1F4200A0, 0x040F00FF }
};

/* kirby_spin_gfx_table of CrashSpecial.asm, same layout as sFTRemixCrashNSPSpinColours. */
static const u32 sFTRemixCrashNSPKirbySpinColours[CRASH_NSP_SPIN_COSTUME_COUNT][4] =
{
    { 0xFF0000A0, 0xFFA5B480, 0xFFA5B4A0, 0xFF4E00C0 },
    { 0xFF8000A0, 0xFFF00080, 0xFFFF00A0, 0xFF4E00C0 },
    { 0x0000FFA0, 0x00F0FF80, 0x00F0FFA0, 0xFF4E00C0 },
    { 0x804000A0, 0xFF000080, 0xFF0000A0, 0xFF4E00C0 },
    { 0x008800A0, 0x00FF6080, 0x00FF60A0, 0xFF4E00C0 },
    { 0x606060A0, 0xFFFFFF80, 0xFFFFFFA0, 0xFF4E00C0 },
    { 0x904080A0, 0x1828A880, 0x1828A8A0, 0xFF4E00C0 }
};

/* Kirby and J Kirby use the Crash spin file at offset 0xCC4, Crash uses the file itself. */
static void **ftRemixCrashNSPSpinFileHead(FTStruct *fp)
{
    FTData *crash_data = port_fighter_data(REMIX_ID_CRASH);

    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        sFTRemixCrashNSPSpinKirbyFile = (u8 *)*crash_data->p_file_special2 + CRASH_NSP_KIRBY_SPIN_FILE_OFFSET;
        return &sFTRemixCrashNSPSpinKirbyFile;
    }
    return crash_data->p_file_special2;
}

/* create_spin_gfx_ (CrashSpecial.asm:862-887) and the tail of Link's original function: creates the attached Spin GFX, NULL when no effect slot is free. */
static GObj *ftRemixCrashNSPCreateSpinGFX(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *spin_gobj;
    DObj *root;

    sFTRemixCrashNSPSpinGfxDesc.file_head = ftRemixCrashNSPSpinFileHead(fp);
    spin_gobj = efManagerMakeEffectNoForce(&sFTRemixCrashNSPSpinGfxDesc);
    if (spin_gobj == NULL)
    {
        return NULL;
    }
    /* The asm jumps back into Link's efManagerLinkSpinAttackMakeEffect (0x80103398) after the creation, which finishes the effect. */
    efGetStruct(spin_gobj)->fighter_gobj = fighter_gobj;
    fp->proc_lagstart = ftParamProcPauseEffect;
    fp->proc_lagend = ftParamProcResumeEffect;
    root = DObjGetStruct(spin_gobj);
    root->user_data.p = fp->joints[nFTPartsJointTopN];
    root->rotate.vec.f.y = (fp->lr == +1) ? F_CLC_DTOR32(30.0F) : F_CLC_DTOR32(210.0F);

    return spin_gobj;
}

/* setup_spin_gfx_ (CrashSpecial.asm:119-155): marks the fighter as having an attached effect, sets the Spin GFX
 * primitive and environment colours from the costume table and sizes the effect by the owner multiplier. The
 * asm also swaps the render routine to the Size one, which the port does once here (see ftremixsizefx2.c). */
static void ftRemixCrashNSPSetupSpinGFX(FTStruct *fp, GObj *spin_gobj)
{
    s32 costume = (fp->costume < CRASH_NSP_SPIN_COSTUME_COUNT) ? fp->costume : 0;
    const u32 *colours = (fp->fkind == REMIX_ID_CRASH) ? sFTRemixCrashNSPSpinColours[costume]
                                                        : sFTRemixCrashNSPKirbySpinColours[costume];
    DObj *top_joint = ((DObj *)spin_gobj->obj)->child;
    DObj *bottom_joint = top_joint->child;

    fp->is_effect_attach = TRUE;
    top_joint->mobj->sub.primcolor.pack = colours[0];
    top_joint->mobj->sub.envcolor.pack = colours[1];
    bottom_joint->mobj->sub.primcolor.pack = colours[2];
    bottom_joint->mobj->sub.envcolor.pack = colours[3];
#ifdef REGION_REMIX
    ftRemixSizeScaleLinkEffect(fp, spin_gobj);
#endif
}

/* Creates the Spin GFX and sets it up when the effect was created (the asm branch on v0 after create_spin_gfx_). */
static void ftRemixCrashNSPSpawnSpinGFX(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *spin_gobj = ftRemixCrashNSPCreateSpinGFX(fighter_gobj);

    if (spin_gobj != NULL)
    {
        ftRemixCrashNSPSetupSpinGFX(fp, spin_gobj);
    }
}

/* FLING_WIDTH, FLING_HEIGHT and FLING_HEIGHT_MIDDLE are all `lui 0x4396` (CrashSpecial.asm 17-19), and the fling
 * speed is the same float with the sign of the side (asm 366-368 and 417-419). */
#define CRASH_NSP_FLING_WIDTH 300.0F              /* 0x43960000 */
#define CRASH_NSP_FLING_HEIGHT 300.0F             /* 0x43960000 */
#define CRASH_NSP_FLING_HEIGHT_MIDDLE 300.0F      /* 0x43960000, added to the fighter y */
#define CRASH_NSP_FLING_SPEED_X 300.0F            /* 0x4396 / 0xC396 */
#define CRASH_NSP_FLING_FGM_ID 0x5BD              /* FGM.play(0x5BD), item throw sound */

extern void *func_800269C0_275C0(u16 id);

/* flung_item_state_table (CrashSpecial.asm 1045-1046): eight zero words, so the flung item has no procs. */
static ITStatusDesc sFTRemixCrashNSPFlungItemStatusDesc;

/* spin_detect_item_ (CrashSpecial.asm:264-329) range test. The item position is the item struct at 0x40
 * (MPCollData.pos_prev) and the fighter position is the TopN translate. */
static sb32 ftRemixCrashNSPIsItemInSpinRange(GObj *fighter_gobj, ITStruct *ip)
{
    Vec3f *fighter_pos = &DObjGetStruct(fighter_gobj)->translate.vec.f;
    f32 diff_x = fighter_pos->x - ip->coll_data.pos_prev.x;
    f32 diff_y = (fighter_pos->y + CRASH_NSP_FLING_HEIGHT_MIDDLE) - ip->coll_data.pos_prev.y;

    return (ABSF(diff_x) <= CRASH_NSP_FLING_WIDTH) && (ABSF(diff_y) <= CRASH_NSP_FLING_HEIGHT);
}

/* spin_detect_item_: returns the first pickable (flag 0x80 of byte 0x2CE = is_allow_pickup, flag 0x01 = light weight)
 * item in range, NULL when there is none. The asm walks the item list at 0x80046700, gGCCommonLinks[nGCCommonLinkIDItem]. */
static GObj *ftRemixCrashNSPFindSpinItem(GObj *fighter_gobj)
{
    GObj *item_gobj = gGCCommonLinks[nGCCommonLinkIDItem];

    while (item_gobj != NULL)
    {
        ITStruct *ip = itGetStruct(item_gobj);

        if (ip->is_allow_pickup && (ip->weight == nITWeightLight) && ftRemixCrashNSPIsItemInSpinRange(fighter_gobj, ip))
        {
            return item_gobj;
        }
        item_gobj = item_gobj->link_next;
    }
    return NULL;
}

/* item_fling_ (CrashSpecial.asm:891-907): disables the pickup flag, puts the item in the air and gives it the empty
 * flung_item_state_table status. */
static void ftRemixCrashNSPSetItemFlung(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->is_allow_pickup = FALSE;
    itMapSetAir(ip);
    itMainSetStatus(item_gobj, &sFTRemixCrashNSPFlungItemStatusDesc, 0);
}

/* The item check shared by ground_main_ and air_main_ (asm 371-396 and 422-446): flings a nearby item away from
 * the fighter at CRASH_NSP_FLING_SPEED_X, positive when fighter x <= item x, and plays the throw sound. */
static void ftRemixCrashNSPSpinItem(GObj *fighter_gobj)
{
    GObj *item_gobj = ftRemixCrashNSPFindSpinItem(fighter_gobj);
    ITStruct *ip;
    f32 fighter_x;
    f32 item_x;

    if (item_gobj == NULL)
    {
        return;
    }
    ip = itGetStruct(item_gobj);
    fighter_x = DObjGetStruct(fighter_gobj)->translate.vec.f.x;
    item_x = DObjGetStruct(item_gobj)->translate.vec.f.x;
    ip->physics.vel_air.x = (fighter_x <= item_x) ? CRASH_NSP_FLING_SPEED_X : -CRASH_NSP_FLING_SPEED_X;
    ftRemixCrashNSPSetItemFlung(item_gobj);
    func_800269C0_275C0(CRASH_NSP_FLING_FGM_ID);
}

/* Character.get_hitbox_collision_flags_ (Character.asm:3322), reduced to the shield test that shield_hit_routine_
 * needs. The asm ORs flag bytes of the active hitboxes. The flag is mapped to GMHitFlags.is_interact_shield,
 * which itprocess.c sets on attack records that hit a shield. See the notes file for this uncertainty. */
static sb32 ftRemixCrashNSPIsShieldCollided(FTStruct *fp)
{
    s32 hitbox_index;
    s32 record_index;

    for (hitbox_index = 0; hitbox_index < CRASH_NSP_SHIELD_HITBOX_NUM; hitbox_index++)
    {
        FTAttackColl *attack_coll = &fp->attack_colls[hitbox_index];

        if (attack_coll->attack_state == 0)
        {
            continue;
        }
        for (record_index = 0; record_index < GMATTACKREC_NUM_MAX; record_index++)
        {
            if (attack_coll->attack_records[record_index].victim_flags.is_interact_shield)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* The asm treats |stick_x| < 11 as neutral. */
static sb32 ftRemixCrashNSPIsStickNeutral(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;

    return (stick_x > -CRASH_NSP_STICK_NEUTRAL_RANGE) && (stick_x < CRASH_NSP_STICK_NEUTRAL_RANGE);
}

/* _apply_movement and _set_velocity of ground_physics_ and air_physics_: moves air x velocity toward
 * target_x by accel once the gap is at least 2.0, then clamps it to MAX_SPEED. */
static void ftRemixCrashNSPApplyVelX(FTStruct *fp, f32 target_x, f32 accel)
{
    f32 diff = target_x - fp->physics.vel_air.x;

    if ((diff >= CRASH_NSP_MIN_SPEED) || (diff <= -CRASH_NSP_MIN_SPEED))
    {
        if (diff >= 0.0F)
        {
            fp->physics.vel_air.x += accel;
        }
        else
        {
            fp->physics.vel_air.x -= accel;
        }
    }
    if (fp->physics.vel_air.x > CRASH_NSP_MAX_SPEED)
    {
        fp->physics.vel_air.x = CRASH_NSP_MAX_SPEED;
    }
    else if (fp->physics.vel_air.x < -CRASH_NSP_MAX_SPEED)
    {
        fp->physics.vel_air.x = -CRASH_NSP_MAX_SPEED;
    }
}

/* ground_to_air_: switches a grounded NSP to its aerial status and keeps the frame and flags. */
static void ftRemixCrashNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPA, nFTRemixCrashKirbyStatusNSPA,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
}

/* ground_to_air_jump_: the grounded NSP jump. Kirby gets the lower jump speed. */
static void ftRemixCrashNSPGroundToAirJump(GObj *fighter_gobj)
{
    FTStruct *fp;
    f32 jump_speed;

    ftRemixCrashNSPGroundToAir(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    /* asm 737-742 compares `at` (the float bits) instead of the reloaded id for J Kirby, so J Kirby never
     * matches and keeps CRASH_NSP_JUMP_SPEED. Kept as is. */
    jump_speed = (fp->fkind == nFTKindKirby) ? CRASH_NSP_KIRBY_JUMP_SPEED : CRASH_NSP_JUMP_SPEED;
    fp->physics.vel_air.y = jump_speed;
    efManagerDustHeavyDoubleMakeEffect(fp->coll_data.p_translate, CRASH_NSP_JUMP_SMOKE_LR, 1.0F);
}

/* air_to_ground_: switches an aerial NSP to its grounded status. */
static void ftRemixCrashNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPG, nFTRemixCrashKirbyStatusNSPG,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
}

/* blocked_ground_to_air_: the blocked grounded NSP becomes the blocked aerial one. */
static void ftRemixCrashNSPBlockedGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPABlocked, nFTRemixCrashKirbyStatusNSPABlocked,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
}

/* blocked_air_to_ground_: the blocked aerial NSP becomes the blocked grounded one. */
static void ftRemixCrashNSPBlockedAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPGBlocked, nFTRemixCrashKirbyStatusNSPGBlocked,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
}

/* blocked_ground_initial_ */
static void ftRemixCrashNSPBlockedGroundInitial(GObj *fighter_gobj)
{
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPGBlocked, nFTRemixCrashKirbyStatusNSPGBlocked,
                                0.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* blocked_air_initial_ */
static void ftRemixCrashNSPBlockedAirInitial(GObj *fighter_gobj)
{
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPABlocked, nFTRemixCrashKirbyStatusNSPABlocked,
                                0.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* shield_hit_routine_: installed as proc_shield. Runs when the NSP hits a shield and turns the
 * move into its blocked form, with the velocities saved for blocked_main_ to restore. */
static void ftRemixCrashNSPShieldHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashNSPShieldVars *shield_vars = &sFTRemixCrashNSPShieldVars[fp->player];

    if (!ftRemixCrashNSPIsShieldCollided(fp))
    {
        return;
    }
    fp->motion_vars.flags.flag1 = 0;
    shield_vars->vel_air_x = fp->physics.vel_air.x * CRASH_NSP_SHIELD_AIR_VEL_X_SCALE;
    shield_vars->vel_air_y = fp->physics.vel_air.y * CRASH_NSP_SHIELD_VEL_Y_SCALE;
    shield_vars->vel_ground_x = fp->physics.vel_ground.x * CRASH_NSP_SHIELD_GROUND_VEL_X_SCALE;
    shield_vars->is_hitlag_over = FALSE;
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_ground.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    if (fp->ga != nMPKineticsGround)
    {
        fp->is_absorb = FALSE;
        fp->absorb_lr = 0;
        fp->is_goto_attack100 = FALSE;
        fp->is_fastfall = FALSE;
        ftRemixCrashNSPBlockedAirInitial(fighter_gobj);
        return;
    }
    ftRemixCrashNSPBlockedGroundInitial(fighter_gobj);
}

/* Shield hitlag hook for Crash and Kirby (the port_fighter_shield_hitlag_skip callback, see fighter_registry.h).
 * Returns TRUE when the NSP is in its blocked form, so the hitlag freeze is skipped (asm `or t6, r0, r0`).
 * Kirby's hat check (asm 941-944) is not repeated: the Kirby blocked ids are only set by the Crash copy. */
int ftRemixCrashNSPShieldHitlagSkip(GObj *fighter_gobj, int status_id)
{
    switch (status_id)
    {
        case nFTRemixCrashStatusNSPGBlocked:
        case nFTRemixCrashStatusNSPABlocked:
        case nFTRemixCrashKirbyStatusNSPGBlocked:
        case nFTRemixCrashKirbyStatusNSPABlocked:
            return TRUE;
        default:
            return FALSE;
    }
}

/* ground_initial_ */
static void ftRemixCrashNSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPG, nFTRemixCrashKirbyStatusNSPG, 0.0F,
                                FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixCrashNSPSpawnSpinGFX(fighter_gobj);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
    fp->physics.vel_air.x *= CRASH_NSP_INITIAL_GROUND_VEL_SCALE;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* air_initial_ */
static void ftRemixCrashNSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPA, nFTRemixCrashKirbyStatusNSPA, 0.0F,
                                FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixCrashNSPSpawnSpinGFX(fighter_gobj);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
    fp->physics.vel_air.x *= CRASH_NSP_INITIAL_AIR_VEL_SCALE;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* ground_main_ */
static void ftRemixCrashNSPGroundProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
        ftRemixCrashNSPSpinItem(fighter_gobj);
        return;
    }
    if ((fp->motion_vars.flags.flag1 == 0) && (ftCommonKneeBendGetInputTypeCommon(fp) != 0))
    {
        ftRemixCrashNSPGroundToAirJump(fighter_gobj);
        return;
    }
    ftRemixCrashNSPSpinItem(fighter_gobj);
}

/* air_main_ */
static void ftRemixCrashNSPAirProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
    ftRemixCrashNSPSpinItem(fighter_gobj);
}

/* blocked_main_ */
static void ftRemixCrashNSPBlockedProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashNSPShieldVars *shield_vars = &sFTRemixCrashNSPShieldVars[fp->player];

    if (fp->motion_vars.flags.flag1 != 0)
    {
        shield_vars->is_hitlag_over = TRUE;
        fp->physics.vel_air.x = shield_vars->vel_air_x;
        fp->physics.vel_air.y = shield_vars->vel_air_y;
        fp->physics.vel_ground.x = shield_vars->vel_ground_x;
        fp->motion_vars.flags.flag1 = 0;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* ground_physics_ */
static void ftRemixCrashNSPGroundProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 target_x = 0.0F;
    f32 accel = CRASH_NSP_ACCELERATION;

    if (fp->motion_vars.flags.flag1 != 0)
    {
        accel = CRASH_NSP_END_ACCELERATION;
    }
    else if (ftRemixCrashNSPIsStickNeutral(fp))
    {
        if ((fp->physics.vel_air.x >= -CRASH_NSP_MIN_SPEED) && (fp->physics.vel_air.x <= CRASH_NSP_MIN_SPEED))
        {
            fp->physics.vel_air.x = 0.0F;
            return;
        }
        accel = CRASH_NSP_NEUTRAL_ACCELERATION;
    }
    else
    {
        target_x = (f32) fp->input.pl.stick_range.x * CRASH_NSP_G_SPEED_MULTIPLIER;
    }
    ftRemixCrashNSPApplyVelX(fp, target_x, accel);
    fp->physics.vel_ground.x = fp->physics.vel_air.x * (f32) fp->lr;
}

/* air_physics_ */
static void ftRemixCrashNSPAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 target_x = 0.0F;
    f32 gravity = CRASH_NSP_GRAVITY;

    if (ftRemixCrashNSPIsStickNeutral(fp))
    {
        if ((fp->physics.vel_air.x >= -CRASH_NSP_MIN_SPEED) && (fp->physics.vel_air.x <= CRASH_NSP_MIN_SPEED))
        {
            fp->physics.vel_air.x = 0.0F;
        }
        else
        {
            ftRemixCrashNSPApplyVelX(fp, target_x, CRASH_NSP_ACCELERATION);
        }
    }
    else
    {
        target_x = (f32) fp->input.pl.stick_range.x * CRASH_NSP_A_SPEED_MULTIPLIER;
        ftRemixCrashNSPApplyVelX(fp, target_x, CRASH_NSP_ACCELERATION);
    }
    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        gravity = CRASH_NSP_KIRBY_GRAVITY;
    }
    ftPhysicsApplyGravityClampTVel(fp, gravity, CRASH_NSP_FALL_SPEED);
}

/* blocked_air_physics_: normal air physics once the hitlag is over. */
static void ftRemixCrashNSPBlockedAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (sFTRemixCrashNSPShieldVars[fp->player].is_hitlag_over)
    {
        ftPhysicsApplyAirVelFriction(fighter_gobj);
    }
}

/* ground_collision_ */
static void ftRemixCrashNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixCrashNSPGroundToAir);
}

/* air_collision_ */
static void ftRemixCrashNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixCrashNSPAirToGround);
}

/* blocked_ground_collision_ */
static void ftRemixCrashNSPBlockedGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixCrashNSPBlockedGroundToAir);
}

/* blocked_air_collision_ */
static void ftRemixCrashNSPBlockedAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixCrashNSPBlockedAirToGround);
}

/* Remix address -> port routine for every CrashNSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixCrashNSPProcs[] =
{
    { REMIX_SYM_CrashNSP__air_collision_, ftRemixCrashNSPAirProcMap },
    { REMIX_SYM_CrashNSP__air_initial_, ftRemixCrashNSPAirInitial },
    { REMIX_SYM_CrashNSP__air_main_, ftRemixCrashNSPAirProcUpdate },
    { REMIX_SYM_CrashNSP__air_physics_, ftRemixCrashNSPAirProcPhysics },
    { REMIX_SYM_CrashNSP__blocked_air_collision_, ftRemixCrashNSPBlockedAirProcMap },
    { REMIX_SYM_CrashNSP__blocked_air_physics_, ftRemixCrashNSPBlockedAirProcPhysics },
    { REMIX_SYM_CrashNSP__blocked_ground_collision_, ftRemixCrashNSPBlockedGroundProcMap },
    { REMIX_SYM_CrashNSP__blocked_main_, ftRemixCrashNSPBlockedProcUpdate },
    { REMIX_SYM_CrashNSP__ground_collision_, ftRemixCrashNSPGroundProcMap },
    { REMIX_SYM_CrashNSP__ground_initial_, ftRemixCrashNSPGroundInitial },
    { REMIX_SYM_CrashNSP__ground_main_, ftRemixCrashNSPGroundProcUpdate },
    { REMIX_SYM_CrashNSP__ground_physics_, ftRemixCrashNSPGroundProcPhysics }
};

const s32 dFTRemixCrashNSPProcsNum = ARRAY_COUNT(dFTRemixCrashNSPProcs);

#endif
