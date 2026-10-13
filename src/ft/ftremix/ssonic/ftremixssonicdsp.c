/**
 * ftremixssonicdsp.c - C ports of Smash Remix Super Sonic Spin Dash entry
 * routines (.toolchain/pkgs/smashremix-src/src/SSonic/SSonic.asm for the
 * emerald effect, Sonic/SonicSpecial.asm scope SSonicDSP for the entries).
 *
 * Both entries play the Fox reflector start animation, spawn the Super Sonic
 * emerald effect, clear the Remix temp variables and speed the player up by
 * 1.25x. The air entry also zeroes the y velocity.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftdef.h>
#include <ft/ftmain.h>
#include <sys/obj.h>
#include <sys/objanim.h>
#include <sys/objdisplay.h>
#include <sys/objman.h>

/* Action.FOX ids from Action.asm. SSonicDSP borrows the Fox reflector animation. */
enum
{
    nFTRemixSSonicDSPFoxReflectorStart = 0x0EC,
    nFTRemixSSonicDSPFoxReflectorStartAir = 0x0F1
};

/* N64 status word that holds the gravity delay (asm `sw t0, 0x0B28(v1)`). */
#define SSONIC_DSP_N64_GRAVITY_DELAY 0x0B28
#define SSONIC_DSP_GRAVITY_DELAY 10
/* 1.25F (0x3FA00000). */
#define SSONIC_DSP_SPEED_MULTIPLIER 1.25F

/* Emerald effect description. Its words follow the Remix layout: flags and
 * dl_link (0x060F0000), file pointer, DObj transform types, then the procs
 * and the file offsets for the DObj setup and the animation joint. */
#define SSONIC_DSP_EMERALD_FLAGS 0x06
#define SSONIC_DSP_EMERALD_DL_LINK 0x0F
#define SSONIC_DSP_EMERALD_DOBJ_SETUP_OFFSET 0x00000930
#define SSONIC_DSP_EMERALD_ANIM_JOINT_OFFSET 0x00001B08
/* Effect vars for the emerald, stored in the N64 EFStruct words 0x18 (index)
 * and 0x1C (status). The values match the Fox reflector effect. */
#define SSONIC_DSP_EMERALD_REFLECTOR_INDEX 0
#define SSONIC_DSP_EMERALD_REFLECTOR_STATUS 4

static void ftRemixSSonicEmeraldProcUpdate(GObj *effect_gobj);

static EFDesc sFTRemixSSonicEmeraldDesc =
{
    .flags = SSONIC_DSP_EMERALD_FLAGS,
    .dl_link = SSONIC_DSP_EMERALD_DL_LINK,
    .transform_types1 = { 0x4F, 0x00, 0x00 },
    .transform_types2 = { nGCMatrixKindTra, 0x2C, 0x00 },
    .proc_update = ftRemixSSonicEmeraldProcUpdate,
    .proc_display = gcDrawDObjTreeForGObj,
    .o_dobjsetup = SSONIC_DSP_EMERALD_DOBJ_SETUP_OFFSET,
    .o_mobjsub = 0x0,
    .o_anim_joint = SSONIC_DSP_EMERALD_ANIM_JOINT_OFFSET,
    .o_matanim_joint = 0x0
};

/* Emerald effect GObj per player. Remix keeps it in the status word at N64 0x0B24.
 * Nothing in the SSonicDSP scope reads it back, so it is only stored here. */
static GObj *sFTRemixSSonicDSPEmeraldGObjs[MAXCONTROLLERS];

/** SSonic.emerald_graphic_update_: plays the emerald animation outside hit lag and ejects it when the animation ends. */
static void ftRemixSSonicEmeraldProcUpdate(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    FTStruct *fp = ftGetStruct(ep->fighter_gobj);

    if (fp->hitlag_tics == 0)
    {
        gcPlayAnimAll(effect_gobj);
    }
    if (effect_gobj->anim_frame > 0.0F)
    {
        return;
    }
    efManagerSetPrevStructAlloc(ep);
    gcEjectGObj(effect_gobj);
}

/**
 * SSonic.emerald_graphic_init_: spawns the emerald effect on the player's top
 * joint. Returns NULL when the effect could not be created.
 */
static GObj *ftRemixSSonicEmeraldInit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *effect_gobj;
    EFStruct *ep;

    sFTRemixSSonicEmeraldDesc.file_head = fp->data->p_file_special2;
    effect_gobj = efManagerMakeEffectForce(&sFTRemixSSonicEmeraldDesc);
    if (effect_gobj == NULL)
    {
        return NULL;
    }

    ep = efGetStruct(effect_gobj);
    ep->fighter_gobj = fighter_gobj;
    DObjGetStruct(effect_gobj)->user_data.p = fp->joints[nFTPartsJointTopN];
    ep->effect_vars.reflector.index = SSONIC_DSP_EMERALD_REFLECTOR_INDEX;
    ep->effect_vars.reflector.status = SSONIC_DSP_EMERALD_REFLECTOR_STATUS;

    gcPlayAnimAll(effect_gobj);
    return effect_gobj;
}

/**
 * Shared start of both entries: change action, play the first animation
 * frame, then spawn the emerald. Returns the emerald GObj, or NULL.
 */
static GObj *ftRemixSSonicDSPStartReflector(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    return ftRemixSSonicEmeraldInit(fighter_gobj);
}

/** Clears the three Remix temp variables and sets the gravity delay. */
static void ftRemixSSonicDSPResetVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    *ftRemixFighterWord(fp, SSONIC_DSP_N64_GRAVITY_DELAY) = SSONIC_DSP_GRAVITY_DELAY;
}

/**
 * Records the emerald on the player and sets is_effect_attach. The asm does
 * this only when the effect was created.
 */
static void ftRemixSSonicDSPAttachEmerald(FTStruct *fp, GObj *emerald_gobj)
{
    if (emerald_gobj == NULL)
    {
        return;
    }
    fp->is_effect_attach = TRUE;
    sFTRemixSSonicDSPEmeraldGObjs[fp->player] = emerald_gobj;
}

/** SSonicDSP.ground_initial_: Spin Dash entry on the ground. */
static void ftRemixSSonicDSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *emerald_gobj = ftRemixSSonicDSPStartReflector(fighter_gobj, nFTRemixSSonicDSPFoxReflectorStart);

    ftRemixSSonicDSPResetVars(fp);
    fp->physics.vel_ground.x *= SSONIC_DSP_SPEED_MULTIPLIER;
    ftRemixSSonicDSPAttachEmerald(fp, emerald_gobj);
}

/** SSonicDSP.air_initial_: Spin Dash entry in the air. */
static void ftRemixSSonicDSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *emerald_gobj = ftRemixSSonicDSPStartReflector(fighter_gobj, nFTRemixSSonicDSPFoxReflectorStartAir);

    ftRemixSSonicDSPResetVars(fp);
    fp->physics.vel_air.x *= SSONIC_DSP_SPEED_MULTIPLIER;
    fp->physics.vel_air.y = 0.0F;
    ftRemixSSonicDSPAttachEmerald(fp, emerald_gobj);
}

const FTRemixProc dFTRemixSSonicDSPProcs[] =
{
    { REMIX_SYM_SSonicDSP__ground_initial_, ftRemixSSonicDSPGroundInitial },
    { REMIX_SYM_SSonicDSP__air_initial_, ftRemixSSonicDSPAirInitial }
};

const s32 dFTRemixSSonicDSPProcsNum = ARRAY_COUNT(dFTRemixSSonicDSPProcs);

#endif
