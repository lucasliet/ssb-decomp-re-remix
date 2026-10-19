/*
 * ftremixbury.c - Bury and plunge of Smash Remix (.toolchain/pkgs/smashremix-src/src/Damage.asm, lines 370 to 910).
 * The buried state is the fighter update process ftRemixBuryMain (the asm compares the main routine, 0x09D4).
 * The plunge and the bury use the fighter processes of FTStruct: proc_update (0x09D4), proc_accessory (0x09D8),
 * proc_interrupt (0x09DC), proc_physics (0x09E0) and proc_map (0x09E4), in the order of the C struct.
 * The stun timer of the asm (0x026C) is kept in sFTRemixBuryTimer, one entry per player.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcommon.h>
#include <ft/ftmain.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <gm/gmdef.h>
#include <gm/gmremixtoggle.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <sys/obj.h>
#include "ft/ftremix/ftremixbury.h"

extern void *func_800269C0_275C0(u16 id);

/* Damage.asm: initial_buried_time 99, buried_percent_multiplier 0.7F, BURIED_Y_OFFSET -48.0F, INITIAL_PLUNGE_SPEED -50.0F */
#define FTREMIX_BURY_BASE_TICS 99
#define FTREMIX_BURY_PERCENT_MUL 0.7F
#define FTREMIX_BURY_JOINT_Y_OFFSET (-48.0F)
#define FTREMIX_BURY_SOFT_PLATFORM_DROP 10.0F
#define FTREMIX_BURY_PLUNGE_SPEED (-50.0F)
#define FTREMIX_BURY_PLUNGE_MAX_FALL (-128.0F)
/* Damage.asm bury_player_ and plunge_player_: FGM.play(0x11F) when buried */
#define FTREMIX_BURY_SFX 0x011F
/* Damage.asm bury_player_ reads the floor flags as 16 bits: MAP_VERTEX_COLL_PASS marks a soft platform, the low byte is the surface */
#define FTREMIX_BURY_SURFACE_MASK 0x00FF
#define FTREMIX_BURY_WALL_FLAGS (MAP_FLAG_LWALL | MAP_FLAG_RWALL)
/* Damage.asm bury_player_ and unbury_player_ use the no-clipping ids -1 and -2 */
#define FTREMIX_BURY_NO_LINE_ID -1
#define FTREMIX_BURY_NO_LINE_ID_ALT -2

static s32 sFTRemixBuryTimer[GMCOMMON_PLAYERS_MAX];

static void ftRemixBuryMain(GObj *fighter_gobj);
static void ftRemixBuryCollision(GObj *fighter_gobj);
static void ftRemixBuryPlayer(GObj *fighter_gobj);
static void ftRemixBuryUnbury(GObj *fighter_gobj);
static void ftRemixPlungePlayer(GObj *fighter_gobj);
static void ftRemixPlungeMain(GObj *fighter_gobj);
static void ftRemixPlungeCollision(GObj *fighter_gobj);
static void ftRemixPlungeGroundTransition(GObj *fighter_gobj);

/* The asm interrupt routines of the bury and the plunge are empty; this stands in for them in proc_accessory */
static void ftRemixBuryIgnoreProc(GObj *fighter_gobj)
{
}

/* Damage.asm bury_initial_: the buried timer is 99 plus 0.7 times the damage percent (truncated) */
static void ftRemixBurySetTimer(FTStruct *fp)
{
    f32 percent = (f32) fp->percent_damage;

    sFTRemixBuryTimer[fp->player] = (s32) (percent * FTREMIX_BURY_PERCENT_MUL) + FTREMIX_BURY_BASE_TICS;
}

/* Damage.asm bury_player_: TRUE when the hazard mode bit 0x1 disables the hazards (modes 1 and 3) */
static sb32 ftRemixBuryHazardsDisabled(void)
{
    return (gmRemixToggle(hazard_mode) & 0x0001) != 0;
}

/* Damage.asm buried_main_: the top joint is drawn 48 units lower on every buried tic */
static void ftRemixBurySetJointOffset(FTStruct *fp)
{
    DObj *joint = fp->joints[nFTPartsJointTopN];

    joint->translate.vec.f.y += FTREMIX_BURY_JOINT_Y_OFFSET;
}

/* Damage.asm bury_player_: sets the buried main, interrupt, physics and collision processes */
static void ftRemixBurySetProcs(FTStruct *fp)
{
    fp->proc_update = ftRemixBuryMain;
    fp->proc_accessory = ftRemixBuryIgnoreProc;
    fp->proc_physics = NULL;
    fp->proc_map = ftRemixBuryCollision;
}

/* Damage.asm bury_player_ on a platform: the DownBounceU status and the buried processes */
static void ftRemixBuryOnPlatform(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 line_id = fp->coll_data.floor_line_id;

    if ((line_id == FTREMIX_BURY_NO_LINE_ID) || (line_id == FTREMIX_BURY_NO_LINE_ID_ALT))
    {
        return;
    }
    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTCommonStatusDownBounceU, 0.0F, 0.0F, FTSTATUS_PRESERVE_NONE);

    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_ground.x = 0.0F;
    ftRemixBurySetProcs(fp);
    func_800269C0_275C0(FTREMIX_BURY_SFX);
}

/* Damage.asm bury_player_ on a soft platform: the fighter is put under it, as a plunge, 10 units lower */
static void ftRemixBurySoftPlatform(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixPlungePlayer(fighter_gobj);
    DObjGetStruct(fighter_gobj)->translate.vec.f.y -= FTREMIX_BURY_SOFT_PLATFORM_DROP;
}

/* Damage.asm bury_player_: a grounded fighter is buried, unless the hazard mode keeps the surface a hazard */
static void ftRemixBuryPlayer(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 surface = fp->coll_data.floor_flags & FTREMIX_BURY_SURFACE_MASK;

    if (((fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS) != 0) && (fp->status_id != nFTCommonStatusShieldBreakFall))
    {
        ftRemixBurySoftPlatform(fighter_gobj);
        return;
    }
    if ((surface != 0) && !ftRemixBuryHazardsDisabled())
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
        return;
    }
    ftRemixBuryOnPlatform(fighter_gobj);
}

/* Damage.asm unbury_player_: a short hop from the bury, as a button jump with the short hop flag */
static void ftRemixBuryUnbury(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->status_vars.common.kneebend.jump_force = 0.0F;
    fp->status_vars.common.kneebend.input_source = FTCOMMON_KNEEBEND_INPUT_TYPE_BUTTON;
    fp->status_vars.common.kneebend.is_shorthop = TRUE;
    ftCommonJumpSetStatus(fighter_gobj);
}

/* Damage.asm plunge_player_: the aerial fighter falls at -50 in the PLUNGE status, then is buried on landing */
static void ftRemixPlungePlayer(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTCommonStatusShieldBreakFall, 0.0F, 0.0F, FTSTATUS_PRESERVE_NONE);

    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = FTREMIX_BURY_PLUNGE_SPEED;
    fp->physics.vel_damage_air.x = 0.0F;
    fp->physics.vel_damage_air.y = 0.0F;
    fp->physics.vel_ground.x = 0.0F;

    fp->proc_update = ftRemixPlungeMain;
    fp->proc_accessory = ftRemixBuryIgnoreProc;
    fp->proc_interrupt = NULL;
    fp->proc_physics = ftRemixPlungeCollision;
    fp->proc_map = ftRemixPlungeGroundTransition;
}

/* Damage.asm buried_main_: the timer counts down and grows with the mash inputs; the fighter is unburied at zero */
static void ftRemixBuryMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 previous_tics;
    s32 tics;

    sFTRemixBuryTimer[fp->player]--;
    previous_tics = sFTRemixBuryTimer[fp->player];

    ftCommonCaptureTrappedUpdateBreakoutVars(fp);

    tics = sFTRemixBuryTimer[fp->player];
    sFTRemixBuryTimer[fp->player] = tics + 3 * (tics - previous_tics);

    if (tics > 0)
    {
        ftRemixBurySetJointOffset(fp);
        return;
    }
    ftRemixBuryUnbury(fighter_gobj);
}

/* Damage.asm buried_collision_: a buried fighter that moves into a wall, or is aerial, is unburied */
static void ftRemixBuryCollision(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (mpCommonProcFighterOnFloor(fighter_gobj, ftRemixBuryUnbury) == FALSE)
    {
        return;
    }
    if (fp->status_id != nFTCommonStatusDownBounceU)
    {
        return;
    }
    if ((fp->coll_data.mask_curr & FTREMIX_BURY_WALL_FLAGS) == 0)
    {
        return;
    }
    ftRemixBuryUnbury(fighter_gobj);
}

/* Damage.asm plunge_main_: the fall speed is capped at -128 */
static void ftRemixPlungeMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->physics.vel_air.y < FTREMIX_BURY_PLUNGE_MAX_FALL)
    {
        fp->physics.vel_air.y = FTREMIX_BURY_PLUNGE_MAX_FALL;
    }
}

/* Damage.asm plunge_collision_: landing through the common landing, which buries the fighter */
static void ftRemixPlungeCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixBuryPlayer);
}

/* Damage.asm plunge_ground_transition: the same landing, kept as the map process of the plunge */
static void ftRemixPlungeGroundTransition(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixBuryPlayer);
}

/* Damage.asm bury_or_plunge_initial_: an aerial fighter plunges, a grounded one is buried; is_start FALSE skips the change */
void ftRemixDamageBuryOrPlungeInitial(GObj *fighter_gobj, sb32 is_start)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (is_start == FALSE)
    {
        return;
    }
    ftRemixBurySetTimer(fp);
    if (fp->ga == nMPKineticsAir)
    {
        ftRemixPlungePlayer(fighter_gobj);
        return;
    }
    ftRemixBuryPlayer(fighter_gobj);
}

/* Damage.asm bury_initial_: the buried timer, then the bury, for a fighter on the ground */
void ftRemixDamageBuryInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixBurySetTimer(fp);
    ftRemixBuryPlayer(fighter_gobj);
}

/* TRUE while the update process is buried main, which is the asm comparison of 0x09D4 with buried_main_ */
sb32 ftRemixDamageIsBuried(FTStruct *fp)
{
    return fp->proc_update == ftRemixBuryMain;
}

#endif
