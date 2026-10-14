/**
 * ftremixdragonkingnsp.c - C ports of Smash Remix Dragon King's neutral special
 * routines (.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm,
 * scope DragonKingNSP).
 *
 * Temp variables: flag0 (0x17C) is the dragon ball spawn request, flag1 (0x180)
 * is the aerial control lock. Scratch word 0x0B20 holds the dragon ball object.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ef/effect.h>
#include <ft/ftanimend.h>
#include <ft/ftchar/ftcaptain/ftcaptainfunctions.h>
#include <ft/ftchar/ftpikachu/ftpikachu.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <sys/objanim.h>
#include <sys/objdisplay.h>

enum
{
    nFTRemixDragonKingStatusNSPGround = 0xE4,
    nFTRemixDragonKingStatusNSPAir = 0xE5
};

/* Kirby and J Kirby copy statuses, from the KIRBY add_new_action order in Kirby.asm (0xDC + 83 + index). */
enum
{
    nFTRemixDragonKingKirbyStatusNSPGround = 0x1AB,
    nFTRemixDragonKingKirbyStatusNSPAir = 0x1AC
};

#define DRAGONKING_NSP_OFF_BALL_OBJECT 0x0B20
#define DRAGONKING_NSP_KIRBY_TRANSITION_FLAGS \
    (FTSTATUS_PRESERVE_RUMBLE | FTSTATUS_PRESERVE_EFFECT | FTSTATUS_PRESERVE_COLANIM)


/* dragon_ball_ / augment_ball_update_routine_ hand-attachment offsets, in the part 0xC (right
 * hand) joint's local space. The asm constant names are misleading: OFFSET_X (0x437A = 250.0F)
 * is stored in the z slot and OFFSET_Y (0xC1F0 = -30.0F) in the x slot; the port follows the
 * slots, not the names. Kirby and J Kirby use their own pair. */
#define DRAGONKING_NSP_BALL_GFX_TIMER 0x1D
#define DRAGONKING_NSP_BALL_SCALE 0.75F              /* lui 0x3F40 */
#define DRAGONKING_NSP_BALL_SCALE_KIRBY 0.5F         /* lui 0x3F00 */
#define DRAGONKING_NSP_BALL_OFF_X (-30.0F)           /* asm OFFSET_Y */
#define DRAGONKING_NSP_BALL_OFF_Y 0.0F
#define DRAGONKING_NSP_BALL_OFF_Z 250.0F             /* asm OFFSET_X */
#define DRAGONKING_NSP_BALL_KIRBY_OFF_X 310.0F       /* asm KIRBY_OFFSET_X */
#define DRAGONKING_NSP_BALL_KIRBY_OFF_Y 0.0F
#define DRAGONKING_NSP_BALL_KIRBY_OFF_Z 100.0F       /* asm KIRBY_OFFSET_Y */

/* Overlays the player's status_vars union at the N64 0x0B20 scratch word, which holds the ball
 * effect object. A real pointer field keeps the port LP64-safe (see ftremixcrashdsp.c). */
typedef struct FTRemixDragonKingNSPStatusVars
{
    u8 pad[0x0B20 - 0x0B18];
    GObj *ball_gobj; /* 0x0B20 */
} FTRemixDragonKingNSPStatusVars;

static FTRemixDragonKingNSPStatusVars *ftRemixDragonKingNSPVars(FTStruct *fp)
{
    return (FTRemixDragonKingNSPStatusVars *)&fp->status_vars;
}

/* TRUE for the Kirby copies, which hold the ball with different offsets and scale. */
static sb32 ftRemixDragonKingNSPIsKirbyCopy(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Positions the ball GFX part 0 at the fighter's right hand (part 0xC) with the asm offsets.
 * gmCollisionGetFighterPartsWorldPosition transforms the offset from joint space to world. */
static void ftRemixDragonKingNSPSetBallPosition(FTStruct *fp, GObj *ball_gobj)
{
    Vec3f offset;
    DObj *part0 = DObjGetStruct(ball_gobj);

    if (ftRemixDragonKingNSPIsKirbyCopy(fp) != FALSE)
    {
        offset.x = DRAGONKING_NSP_BALL_KIRBY_OFF_X;
        offset.y = DRAGONKING_NSP_BALL_KIRBY_OFF_Y;
        offset.z = DRAGONKING_NSP_BALL_KIRBY_OFF_Z;
    }
    else
    {
        offset.x = DRAGONKING_NSP_BALL_OFF_X;
        offset.y = DRAGONKING_NSP_BALL_OFF_Y;
        offset.z = DRAGONKING_NSP_BALL_OFF_Z;
    }
    gmCollisionGetFighterPartsWorldPosition(fp->joints[0xC], &offset);

    part0->translate.vec.f = offset;
}

/* DragonKingNSP.destroy_ball_object_ (patch at vanilla 0x80101810): frees the ball effect and
 * clears the references the fighter and the on-hit hook hold to it. */
static void ftRemixDragonKingNSPDestroyBall(GObj *ball_gobj)
{
    EFStruct *ep = efGetStruct(ball_gobj);
    FTStruct *fp = (FTStruct *)ep->fighter_gobj;

    ftRemixDragonKingNSPVars(fp)->ball_gobj = NULL;
    fp->proc_damage = NULL;

    efManagerSetPrevStructAlloc(ep);
    gcEjectGObj(ball_gobj);
}

/* DragonKingNSP.destroy_ball_on_hit_ (installed as proc_damage by the transitions): destroys
 * the ball when the fighter is damaged; no-op when no ball is out. */
static void ftRemixDragonKingNSPDestroyBallOnHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *ball_gobj = ftRemixDragonKingNSPVars(fp)->ball_gobj;

    if (ball_gobj == NULL)
    {
        return;
    }
    ftRemixDragonKingNSPDestroyBall(ball_gobj);
}

/* Ball effect update (the desc proc): vanilla func_ovl2_801017E8 plus the
 * augment_ball_update_routine_ patch. The ball follows the hand every frame and freezes
 * (no anim, no timer) during hitlag; when the timer set by dragon_ball_ runs out the effect
 * is freed through ftRemixDragonKingNSPDestroyBall. */
static void ftRemixDragonKingNSPBallProcUpdate(GObj *ball_gobj)
{
    EFStruct *ep = efGetStruct(ball_gobj);
    FTStruct *fp = (FTStruct *)ep->fighter_gobj;

    if (fp->hitlag_tics != 0)
    {
        return;
    }
    ftRemixDragonKingNSPSetBallPosition(fp, ball_gobj);
    gcPlayAnimAll(ball_gobj);

    if (ep->effect_vars.unknown1.efvars_unk1_0x0 == 0)
    {
        ftRemixDragonKingNSPDestroyBall(ball_gobj);
        return;
    }
    ep->effect_vars.unknown1.efvars_unk1_0x0--;
}

/* Descriptor body, written after the update proc it names. */
static EFDesc sFTRemixDragonKingNSPBallGfxDesc =
{
    .flags = 0x4 | EFFECT_FLAG_USERDATA | 0x1,
    .dl_link = 15,
    .file_head = &gFTDataPikachuSpecial2,
    .transform_types1 = { 0x28, 0x1A, 0x00 },
    .transform_types2 = { 0x1C, 0x00, 0x00 },
    .proc_update = ftRemixDragonKingNSPBallProcUpdate,
    .proc_display = gcDrawDObjTreeForGObj,  /* REMIX-TODO(Size): the Size.asm dragonking.nsp
                                             * render_routine_ scale wrapper is not applied. */
    .o_dobjsetup = 0x800,
    .o_mobjsub = 0x640,
    .o_anim_joint = 0x890,
    .o_matanim_joint = 0x900
};

/* DragonKingNSP.dragon_ball_ (called by main_): spawns the lightning ball effect, scales it
 * (REMIX-TODO(Size): the Size.multiplier_table factor is not applied, multiplier 1.0F),
 * attaches it to the right hand, saves it in the player struct word 0x0B20 and arms the
 * on-hit destroy hook (0x09EC = proc_damage). The spawn request (temp variable 1) is consumed
 * either way, as the asm does. */
static void ftRemixDragonKingNSPDragonBall(FTStruct *fp)
{
    GObj *ball_gobj;
    EFStruct *ep;
    DObj *model_joint;
    f32 scale;

    if (fp->motion_vars.flags.flag0 != 1)
    {
        return;
    }
    fp->motion_vars.flags.flag0 = 0;

    ball_gobj = efManagerMakeEffectNoForce(&sFTRemixDragonKingNSPBallGfxDesc);
    if (ball_gobj == NULL)
    {
        return;
    }
    ep = efGetStruct(ball_gobj);
    ep->effect_vars.unknown1.efvars_unk1_0x0 = DRAGONKING_NSP_BALL_GFX_TIMER;

    /* The asm stores the player struct at 0x04 of the GFX special struct (the EFStruct
     * fighter_gobj slot); the update and destroy routines read it back. */
    ep->fighter_gobj = (GObj *)fp;

    scale = (ftRemixDragonKingNSPIsKirbyCopy(fp) != FALSE) ? DRAGONKING_NSP_BALL_SCALE_KIRBY : DRAGONKING_NSP_BALL_SCALE;
    model_joint = DObjGetStruct(ball_gobj)->child;
    model_joint->scale.vec.f.x = scale;
    model_joint->scale.vec.f.y = scale;
    model_joint->scale.vec.f.z = scale;

    ftRemixDragonKingNSPVars(fp)->ball_gobj = ball_gobj;
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;

    ftRemixDragonKingNSPSetBallPosition(fp, ball_gobj);
}

/* Shared body of DragonKingNSP.ground_initial_ and air_initial_. */
static void ftRemixDragonKingNSPSetDragonBallStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftCaptainSpecialNInitStatusVars(fighter_gobj);
    *ftRemixFighterWord(fp, DRAGONKING_NSP_OFF_BALL_OBJECT) = 0;
}

/* DragonKingNSP.ground_initial_ */
static void ftRemixDragonKingNSPGroundInitial(GObj *fighter_gobj)
{
    ftRemixDragonKingNSPSetDragonBallStatus(fighter_gobj, nFTRemixDragonKingStatusNSPGround);
}

/* DragonKingNSP.air_initial_ */
static void ftRemixDragonKingNSPAirInitial(GObj *fighter_gobj)
{
    ftRemixDragonKingNSPSetDragonBallStatus(fighter_gobj, nFTRemixDragonKingStatusNSPAir);
}

/* DragonKingNSP.ground_to_air_ (transition from ground collision). */
static void ftRemixDragonKingNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftCaptainSpecialNSwitchStatusAir(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.air_to_ground_ (transition from air collision). */
static void ftRemixDragonKingNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftCaptainSpecialAirNSwitchStatusGround(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.ground_collision_ */
static void ftRemixDragonKingNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixDragonKingNSPGroundToAir);
}

/* DragonKingNSP.air_collision_ */
static void ftRemixDragonKingNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDragonKingNSPAirToGround);
}

/* REMIX-TODO(lag): kirby_ground_to_air_ and kirby_air_to_ground_ also store 0x800F9C8C and 0x800F9CC4 in
 * proc_lagstart and proc_lagend. Neither address names a decomp function (0x800F9C8C lies just after
 * mpCollisionCheckProjectRWall at 0x800F9C30), so the lag procs are not set here. See the notes file. */
/* DragonKingNSP.kirby_ground_to_air_: moves a grounded Kirby special to its aerial status. */
static void ftRemixDragonKingNSPKirbyGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingKirbyStatusNSPAir, fighter_gobj->anim_frame, 1.0F,
        DRAGONKING_NSP_KIRBY_TRANSITION_FLAGS);
    ftPhysicsClampAirVelXMax(fp);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.kirby_air_to_ground_: moves an aerial Kirby special to its grounded status. */
static void ftRemixDragonKingNSPKirbyAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingKirbyStatusNSPGround, fighter_gobj->anim_frame, 1.0F,
        DRAGONKING_NSP_KIRBY_TRANSITION_FLAGS);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.kirby_ground_collision_ */
static void ftRemixDragonKingNSPKirbyGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDragonKingNSPKirbyGroundToAir);
}

/* DragonKingNSP.kirby_air_collision_ */
static void ftRemixDragonKingNSPKirbyAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDragonKingNSPKirbyAirToGround);
}

/* DragonKingNSP.main_ (the proc update). Once the animation ends, the fighter waits on the
 * ground or falls in the air, chosen by the kinetics (ga). */
static void ftRemixDragonKingNSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    void (*proc_status)(GObj *);

    ftRemixDragonKingNSPDragonBall(fp);

    proc_status = (fp->ga == nMPKineticsGround) ? ftCommonWaitSetStatus : ftCommonFallSetStatus;
    ftAnimEndCheckSetStatus(fighter_gobj, proc_status);
}

/* DragonKingNSP.physics_aerial_ (temp variable 2 locks aerial control). */
static void ftRemixDragonKingNSPAerialProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
        return;
    }
    ftPhysicsApplyAirVelFriction(fighter_gobj);
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
}

const FTRemixProc dFTRemixDragonKingNSPProcs[] =
{
    { REMIX_SYM_DragonKingNSP__air_collision_, ftRemixDragonKingNSPAirProcMap },
    { REMIX_SYM_DragonKingNSP__air_initial_, ftRemixDragonKingNSPAirInitial },
    { REMIX_SYM_DragonKingNSP__ground_collision_, ftRemixDragonKingNSPGroundProcMap },
    { REMIX_SYM_DragonKingNSP__ground_initial_, ftRemixDragonKingNSPGroundInitial },
    { REMIX_SYM_DragonKingNSP__main_, ftRemixDragonKingNSPProcUpdate },
    { REMIX_SYM_DragonKingNSP__physics_aerial_, ftRemixDragonKingNSPAerialProcPhysics },
    { REMIX_SYM_DragonKingNSP__kirby_ground_collision_, ftRemixDragonKingNSPKirbyGroundProcMap },
    { REMIX_SYM_DragonKingNSP__kirby_air_collision_, ftRemixDragonKingNSPKirbyAirProcMap },
};

const s32 dFTRemixDragonKingNSPProcsNum = ARRAY_COUNT(dFTRemixDragonKingNSPProcs);

#endif
