/**
 * ftremixfootstool.c - Smash Remix "Footstool Jumping" setting (FootStool/FootStool.asm, toggle entry_footstool).
 * A fighter that presses jump in the air above another fighter footstools it: the victim is sent to the footstooled
 * tumble and the jumper starts a jump squat. Every entry point is a no-op while the setting is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftmain.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <mp/mpcommon.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include "gm/gmremixtoggle.h"
#include "remix/remix_roster_data.h"
#include "ftremixhooks.h"

extern void *func_800269C0_275C0(u16 id);

#define FTREMIX_FOOTSTOOL_FALL_SPEED (-25.0F)           /* FootStool.asm FOOTSTOOL_FALL_SPEED (0xC1C8 << 16) */
#define FTREMIX_FOOTSTOOL_FRAME_SPEED 0.5F              /* FootStool.asm FOOTSTOOL_FSM (0x3F00 << 16) */
#define FTREMIX_FOOTSTOOL_SFX 0x04B0                    /* FootStool.asm foot_stool_jump_initial_ FGM.play */
#define FTREMIX_FOOTSTOOL_HEIGHT_HALF 0.5F              /* FootStool.asm check_target_ 0x3F00 << 16 */
#define FTREMIX_FOOTSTOOL_Y_RANGE_RATIO 0.19921875F     /* FootStool.asm check_target_ 0x3E4C << 16 */
#define FTREMIX_FOOTSTOOL_LEDGE_MASK 0x3000             /* FootStool.asm footstooled_main ledge bits of coll mask_stat */
#define FTREMIX_FOOTSTOOL_JUMP_DELAY_TICS 40            /* FootStool.asm FOOTSTOOLED_JUMP_DELAY */
#define FTREMIX_FOOTSTOOL_KO_ACTION_MAX 7               /* FootStool.asm _action_check: ids below 7 are KO actions */
#define FTREMIX_FOOTSTOOL_INTANGIBLE_HITSTATUS 3        /* FootStool.asm check_target_ */
#define FTREMIX_FOOTSTOOL_MASTER_HAND_KIND 0x0C         /* FootStool.asm check_target_ */

/* Action ids from Action.asm: Idle line 15, JumpSquat line 25, LandingHeavy line 37, Tumble line 62, FallSpecial line 63,
 * LandingSpecial line 64, KIRBY Jump2 and Jump6 lines 1166 and 1170 */
enum
{
    nFTRemixFootStoolActionIdle = 0x00A,
    nFTRemixFootStoolActionJumpSquat = 0x014,
    nFTRemixFootStoolActionLandingHeavy = 0x020,
    nFTRemixFootStoolActionTumble = 0x039,
    nFTRemixFootStoolActionFallSpecial = 0x03A,
    nFTRemixFootStoolActionLandingSpecial = 0x03B,
    nFTRemixFootStoolActionKirbyJump2 = 0x0DF,
    nFTRemixFootStoolActionKirbyJump6 = 0x0E3
};

typedef struct FTRemixFootStoolTarget
{
    GObj *gobj;
    f32 x_diff;
} FTRemixFootStoolTarget;

static void ftRemixFootStoolAirCollision(GObj *fighter_gobj);
static void ftRemixFootStoolAirToGround(GObj *fighter_gobj);
static void ftRemixFootStooledProcMap(GObj *fighter_gobj);
static void ftRemixFootStooledProcInterrupt(GObj *fighter_gobj);

/* FootStool.asm _team_check: Remix compares the target team with the jumper GObj byte at 0x0C (link_id), not with the
 * jumper's team. Kept as in Remix, see docs/remix/notes/f8_footstool.md. */
static sb32 ftRemixFootStoolIsTeamBlocked(GObj *jumper_gobj, FTStruct *target_fp)
{
    if (!gSCManagerBattleState->is_team_battle || gSCManagerBattleState->is_team_attack)
    {
        return FALSE;
    }
    return target_fp->team == jumper_gobj->link_id;
}

/* FootStool.asm check_target_: the victim is not a MASTER HAND, is not landing heavy, is not invincible or intangible
 * and is not already footstooled. */
static sb32 ftRemixFootStoolIsVictimStateValid(FTStruct *target_fp)
{
    if (target_fp->fkind == FTREMIX_FOOTSTOOL_MASTER_HAND_KIND)
    {
        return FALSE;
    }
    if (target_fp->status_id == nFTRemixFootStoolActionLandingHeavy)
    {
        return FALSE;
    }
    if ((target_fp->invincible_tics != 0) || (target_fp->star_invincible_tics != 0))
    {
        return FALSE;
    }
    if (target_fp->hitstatus == FTREMIX_FOOTSTOOL_INTANGIBLE_HITSTATUS)
    {
        return FALSE;
    }
    return target_fp->proc_map != ftRemixFootStooledProcMap;
}

/* FootStool.asm _action_check: targets in a KO action are skipped. */
static sb32 ftRemixFootStoolIsNotKO(FTStruct *target_fp)
{
    return target_fp->status_id >= FTREMIX_FOOTSTOOL_KO_ACTION_MAX;
}

/* FootStool.asm check_target_ x range: |jumper x - target x| must be within the target's scaled BB width. */
static sb32 ftRemixFootStoolIsInRangeX(GObj *jumper_gobj, GObj *target_gobj, f32 *x_diff)
{
    FTStruct *target_fp = ftGetStruct(target_gobj);
    f32 size_multiplier = ftRemixSizeGetMultiplier(target_fp->player);
    f32 width = target_fp->attr->map_coll.width * size_multiplier;

    *x_diff = ABS(DObjGetStruct(jumper_gobj)->translate.vec.f.x - DObjGetStruct(target_gobj)->translate.vec.f.x);
    return *x_diff <= width;
}

/* FootStool.asm check_target_ y range: the jumper is not below the target, and the height difference is between 0.2 and
 * 1.5 times the target's scaled height (the 0.2 factor is 0x3E4C0000 in the asm). */
static sb32 ftRemixFootStoolIsInRangeY(GObj *jumper_gobj, GObj *target_gobj)
{
    FTStruct *target_fp = ftGetStruct(target_gobj);
    f32 size_multiplier = ftRemixSizeGetMultiplier(target_fp->player);
    f32 jumper_y = DObjGetStruct(jumper_gobj)->translate.vec.f.y;
    f32 target_y = DObjGetStruct(target_gobj)->translate.vec.f.y;
    f32 y_diff = jumper_y - target_y;
    f32 height = target_fp->attr->map_coll.top * size_multiplier;
    f32 upper_limit = height + (height * FTREMIX_FOOTSTOOL_HEIGHT_HALF);
    f32 lower_limit = upper_limit * FTREMIX_FOOTSTOOL_Y_RANGE_RATIO;

    if (target_y > jumper_y)
    {
        return FALSE;
    }
    if (y_diff > upper_limit)
    {
        return FALSE;
    }
    return lower_limit <= y_diff;
}

static sb32 ftRemixFootStoolIsCandidate(GObj *jumper_gobj, GObj *other_gobj)
{
    FTStruct *other_fp;

    if (other_gobj == jumper_gobj)
    {
        return FALSE;
    }
    other_fp = ftGetStruct(other_gobj);
    if (ftRemixFootStoolIsTeamBlocked(jumper_gobj, other_fp))
    {
        return FALSE;
    }
    if (ftRemixFootStoolIsNotKO(other_fp) == FALSE)
    {
        return FALSE;
    }
    return ftRemixFootStoolIsVictimStateValid(other_fp);
}

/* FootStool.asm foot_stool_check_ loop: the closest candidate in range is the target; ties go to the later fighter. */
static sb32 ftRemixFootStoolFindTarget(GObj *jumper_gobj, FTRemixFootStoolTarget *target)
{
    GObj *other_gobj;
    f32 x_diff = 0.0F;

    target->gobj = NULL;
    target->x_diff = 0.0F;
    for (other_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; other_gobj != NULL; other_gobj = other_gobj->link_next)
    {
        if (ftRemixFootStoolIsCandidate(jumper_gobj, other_gobj) == FALSE)
        {
            continue;
        }
        if ((ftRemixFootStoolIsInRangeX(jumper_gobj, other_gobj, &x_diff) == FALSE) ||
            (ftRemixFootStoolIsInRangeY(jumper_gobj, other_gobj) == FALSE))
        {
            continue;
        }
        if ((target->gobj == NULL) || (x_diff <= target->x_diff))
        {
            target->gobj = other_gobj;
            target->x_diff = x_diff;
        }
    }
    return target->gobj != NULL;
}

/* FootStool.asm foot_stool_initial_: the Kirby family can only be footstooled in its aerial jump actions. */
static sb32 ftRemixFootStoolIsKirbyFamily(s32 fkind)
{
    switch (fkind)
    {
    case nFTKindKirby:
    case REMIX_ID_JKIRBY:
    case nFTKindNKirby:
    case nFTKindPurin:
    case REMIX_ID_JPUFF:
    case REMIX_ID_EPUFF:
    case nFTKindNPurin:
    case REMIX_ID_DEDEDE:
        return TRUE;
    default:
        return FALSE;
    }
}

/* FootStool.asm foot_stool_initial_: no victim below Idle (reviving), and no victim from LandingSpecial on except the
 * Kirby family in its Jump2 to Jump6 actions. */
static sb32 ftRemixFootStoolCanVictimBeStooled(FTStruct *fp)
{
    s32 action = fp->status_id;

    if (action < nFTRemixFootStoolActionIdle)
    {
        return FALSE;
    }
    if (action < nFTRemixFootStoolActionLandingSpecial)
    {
        return TRUE;
    }
    if (ftRemixFootStoolIsKirbyFamily(fp->fkind) == FALSE)
    {
        return FALSE;
    }
    return (action >= nFTRemixFootStoolActionKirbyJump2) && (action <= nFTRemixFootStoolActionKirbyJump6);
}

/* FootStool.asm foot_stool_initial_ reads the word at 0x0148 as the kinetic state, which is jumps_used in the first byte.
 * Ported as jumps_used != 0; GObj ga is at 0x014C if the lead wants the field the comment names. */
static sb32 ftRemixFootStoolIsAerial(FTStruct *fp)
{
    return fp->jumps_used != 0;
}

/* FootStool.asm foot_stool_initial_: the victim falls at FOOTSTOOL_FALL_SPEED and enters the footstooled tumble. Aerial
 * victims also get the footstooled interrupt and map routines. */
static void ftRemixFootStoolVictimInit(GObj *victim_gobj)
{
    FTStruct *fp = ftGetStruct(victim_gobj);
    s32 status_id;

    if (ftRemixFootStoolCanVictimBeStooled(fp) == FALSE)
    {
        return;
    }
    fp->physics.vel_air.y = FTREMIX_FOOTSTOOL_FALL_SPEED;

    status_id = ftRemixFootStoolIsAerial(fp) ? nFTRemixFootStoolActionTumble : nFTRemixFootStoolActionLandingHeavy;
    ftMainSetStatus(victim_gobj, status_id, 0.0F, FTREMIX_FOOTSTOOL_FRAME_SPEED, FTSTATUS_PRESERVE_NONE);

    if (ftRemixFootStoolIsAerial(fp))
    {
        fp->proc_map = ftRemixFootStooledProcMap;
        fp->proc_interrupt = ftRemixFootStooledProcInterrupt;
    }
}

/* FootStool.asm foot_stool_jump_initial_: the jumper starts a jump squat that copies the kneebend fields of a jump input,
 * so its jump is resolved by the kneebend jump squat code. */
static void ftRemixFootStoolJumperInit(GObj *jumper_gobj, s32 jump_type)
{
    FTStruct *fp = ftGetStruct(jumper_gobj);

    ftMainSetStatus(jumper_gobj, nFTRemixFootStoolActionJumpSquat, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    fp->proc_map = ftRemixFootStoolAirCollision;
    fp->proc_interrupt = NULL;

    fp->status_vars.common.kneebend.jump_force = fp->input.pl.stick_range.y;
    fp->status_vars.common.kneebend.anim_frame = 0.0F;
    fp->status_vars.common.kneebend.input_source = jump_type;
    fp->status_vars.common.kneebend.is_shorthop = FALSE;
    fp->is_special_interrupt = TRUE;

    func_800269C0_275C0(FTREMIX_FOOTSTOOL_SFX);
}

/* FootStool.asm foot_stool_check_: searches the closest target for an aerial jump and, when one is found, starts the
 * footstool. The caller must skip its own jump when this returns TRUE. */
sb32 ftRemixFootStoolTryAerialJump(GObj *fighter_gobj, s32 jump_type)
{
    FTRemixFootStoolTarget target;
    FTStruct *jumper_fp = ftGetStruct(fighter_gobj);

    if (!gmRemixToggle(footstool))
    {
        return FALSE;
    }
    if (jumper_fp->status_id == nFTRemixFootStoolActionFallSpecial)
    {
        return FALSE;
    }
    if (ftRemixFootStoolFindTarget(fighter_gobj, &target) == FALSE)
    {
        return FALSE;
    }
    ftRemixFootStoolVictimInit(target.gobj);
    ftRemixFootStoolJumperInit(fighter_gobj, jump_type);
    return TRUE;
}

/* FootStool.asm footstool_air_collision_: the jumper lands through the footstool landing routine. */
static void ftRemixFootStoolAirCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixFootStoolAirToGround);
}

/* FootStool.asm footstool_air_to_ground_: the jumper lands in JumpSquat and keeps its hitbox. */
static void ftRemixFootStoolAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixFootStoolActionJumpSquat, fighter_gobj->anim_frame, 1.0F,
                    FTSTATUS_PRESERVE_HIT);
}

/* FootStool.asm footstooled_main: the victim lands on the ground unless it hits a ledge collision. */
static void ftRemixFootStooledProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (mpCommonCheckFighterCliff(fighter_gobj) == FALSE)
    {
        return;
    }
    if (fp->coll_data.mask_stat & FTREMIX_FOOTSTOOL_LEDGE_MASK)
    {
        return;
    }
    ftCommonDownBounceSetStatus(fighter_gobj);
}

/* FootStool.asm footstooled_main_2: the victim can leave the footstooled tumble after FOOTSTOOLED_JUMP_DELAY tics. */
static void ftRemixFootStooledProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->status_total_tics < FTREMIX_FOOTSTOOL_JUMP_DELAY_TICS)
    {
        return;
    }
    ftCommonDamageFallProcInterrupt(fighter_gobj);
}

/**
 * FootStool.asm num_jumps_no_reset: the footstool jump squat keeps the jumps used when the fighter goes back to the air.
 * @param fp Fighter whose air state was just set by mpCommonSetFighterAir.
 * @return TRUE if jumps_used must not be reset.
 */
sb32 ftRemixFootStoolPreservesJumps(FTStruct *fp)
{
    return fp->proc_map == ftRemixFootStoolAirCollision;
}

#endif
