/**
 * ftremixbowser.c - C ports of Smash Remix Bowser routines
 * (.toolchain/pkgs/smashremix-src/src/Bowser/BowserSpecial.asm).
 *
 * BowserNSP.main_ follows the decoded instructions of the ROM, which splice
 * vanilla Fire Flower code through OS.copy_segment. Its flame is the vanilla
 * Fire Flower flame weapon (itFFlowerWeaponFlameMakeWeapon). See
 * build/remixasm/notes/34_BOWSER.md.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftchar/ftyoshi/ftyoshi.h>
#include <ft/ftparam.h>
#include <gm/gmcollision.h>
#include <it/item.h>
#include <it/itcommon/itfflower.h>
#include <mp/mpcommon.h>
#include <reloc_data.h>
#include <sys/objanim.h>
#include <ft/ftchar/ftcaptain/ftcaptainfunctions.h>

extern ITDesc dITFFlowerItemDesc;
extern void *func_800269C0_275C0(u16 id);

#define BOWSER_USP_INITIAL_SPEED 60.0F          /* 0x42700000 */
#define BOWSER_USP_G_INITIAL_SPEED 80.0F        /* 0x42A00000 */
#define BOWSER_USP_GROUND_MAX_SPEED_LOW 10.0F   /* 0x41200000 */
#define BOWSER_USP_GROUND_MAX_SPEED_HIGH 44.0F  /* 0x42300000 */
#define BOWSER_USP_GROUND_ACCEL 0.05415039137005806F /* 0x3D5DCCCD */
#define BOWSER_USP_GRAVITY 1.125F               /* 0x3F900000 */
#define BOWSER_USP_X_ACCEL 0.03125F             /* 0x3D000000 */
#define BOWSER_USP_X_MAX_SPEED 40.0F            /* 0x42200000 */
#define BOWSER_USP_G_X_ACCEL 0.039794921875F    /* 0x3D230000 */
#define BOWSER_USP_G_X_MAX_SPEED 60.0F          /* 0x42700000 */
#define BOWSER_USP_X_STICK_RANGE_MIN 8
#define BOWSER_NSP_G_AMMO 0x28
#define BOWSER_NSP_FLAME_VEL 30.0F                         /* 0x41F00000, ITFFLOWER_AMMO_VEL */
#define BOWSER_NSP_FLAME_SIN_ARG_GBOWSER (-0.5F)           /* 0xBF000000, G-Bowser's alternate direction */
#define BOWSER_NSP_FLAME_SPAWN_X_OFFSET 100.0F             /* 0x42C80000, times fighter lr */
#define BOWSER_NSP_FLAME_SPAWN_Y_OFFSET (-200.0F)          /* 0xC3480000 */
#define BOWSER_NSP_FLAME_ORIGIN_X 60.0F                    /* Fire Flower make_flame_offset, x */
#define BOWSER_NSP_FLAME_ORIGIN_Y 100.0F                   /* Fire Flower make_flame_offset, y */
#define BOWSER_NSP_FLAME_ORIGIN_Z_BOWSER 120.0F            /* 0x42F00000, replaces the table z */
#define BOWSER_NSP_FLAME_ORIGIN_Z_KIRBY (-20.0F)           /* 0xC1A00000 */
#define BOWSER_NSP_FLAME_JOINT 7
#define BOWSER_NSP_FLAME_JOINT_KIRBY 13
#define BOWSER_NSP_KIRBY_SCALE_NUMERATOR 2.0F              /* 0x40000000, Kirby origin scale */
#define BOWSER_NSP_SMOKE_JOINT_KIRBY 0x10
#define BOWSER_NSP_LOOP_RELOAD 12
#define BOWSER_NSP_LOOP_STEPS 8
#define BOWSER_NSP_FLAME_INDEX_MAX 8
#define BOWSER_NSP_FLAME_INDEX_MIRROR 5
#define BOWSER_NSP_RELEASE_FRAMES 0x14
#define BOWSER_NSP_ACTIVE_FRAMES 5
#define BOWSER_NSP_FRAMES_MAX 0x10000
#define BOWSER_NSP_FGM_SMOKE_OUT 0x30                      /* Remix FGM id, out of ammo */
#define BOWSER_NSP_FGM_SMOKE_HAVE_AMMO 0x1A                /* Remix FGM id, with ammo */
#define BOWSER_NSP_RUMBLE_ID 6

enum
{
    nFTRemixBowserStatusWhirlingFortressAir = 0xDF,
    nFTRemixBowserStatusFireBreath = 0xE4,
    nFTRemixBowserStatusFireBreathAir = 0xE7,
    nFTRemixBowserStatusFThrowStep1 = 0xE6,
    nFTRemixBowserStatusFThrowStep2 = 0xE8,
    nFTRemixBowserKirbyStatusFireBreathGround = 0x13E,
    nFTRemixBowserKirbyStatusFireBreathAir = 0x13F
};

/* Bowser keeps untyped scratch words at N64 FTStruct offsets 0x0B18..0x0B30
 * (see struct_offsets.txt). They hold integers, so they are read and written
 * through an s32 pointer at the same offset, as the Remix code does. */
static s32 *ftRemixBowserWordAt(FTStruct *fp, u32 n64_offset)
{
    return ftRemixFighterWord(fp, n64_offset);
}

/* Kirby and J Kirby use their own action ids for the same special. */
static s32 ftRemixBowserNSPStatusId(FTStruct *fp, s32 bowser_status, s32 kirby_status)
{
    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return kirby_status;
    }
    return bowser_status;
}

/* Clears the NSP counters at 0x0B18..0x0B2C and sets Giga Bowser's ammo. */
static void ftRemixBowserNSPInitVars(FTStruct *fp)
{
    *ftRemixBowserWordAt(fp, 0x0B18) = 0;
    *ftRemixBowserWordAt(fp, 0x0B1C) = 1;
    *ftRemixBowserWordAt(fp, 0x0B20) = 1;
    *ftRemixBowserWordAt(fp, 0x0B24) = 0;
    *ftRemixBowserWordAt(fp, 0x0B28) = 0;
    *ftRemixBowserWordAt(fp, 0x0B2C) = 0;
    if (fp->fkind == REMIX_ID_GBOWSER)
    {
        *ftRemixBowserWordAt(fp, 0x0B30) = BOWSER_NSP_G_AMMO;
    }
}

/* Shared body of BowserNSP.ground_initial_ and BowserNSP.air_initial_. */
static void ftRemixBowserNSPSetStatus(GObj *fighter_gobj, s32 bowser_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->motion_vars.flags.flag0 = 0;
    ftRemixBowserNSPInitVars(fp);
    ftMainSetStatus(fighter_gobj, ftRemixBowserNSPStatusId(fp, bowser_status, kirby_status), 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* BowserNSP.ground_initial_ */
static void ftRemixBowserNSPGroundSetStatus(GObj *fighter_gobj)
{
    ftRemixBowserNSPSetStatus(fighter_gobj, nFTRemixBowserStatusFireBreath, nFTRemixBowserKirbyStatusFireBreathGround);
}

/* BowserNSP.air_initial_ */
static void ftRemixBowserNSPAirSetStatus(GObj *fighter_gobj)
{
    ftRemixBowserNSPSetStatus(fighter_gobj, nFTRemixBowserStatusFireBreathAir, nFTRemixBowserKirbyStatusFireBreathAir);
}

/* BowserUSP.air_initial_: starts the aerial up special with a fresh state. */
static void ftRemixBowserUSPAirSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    *ftRemixBowserWordAt(fp, 0x0B18) = 0;
    *ftRemixBowserWordAt(fp, 0x0B1C) = 0;
    *ftRemixBowserWordAt(fp, 0x0B20) = 0;
    *ftRemixBowserWordAt(fp, 0x0B24) = 0;
    *ftRemixBowserWordAt(fp, 0x0B28) = 0;
    *ftRemixBowserWordAt(fp, 0x0B2C) = 0;
    ftMainSetStatus(fighter_gobj, nFTRemixBowserStatusWhirlingFortressAir, 0.0F, 1.0F, 0);

    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
    fp->jumps_used = (u8)fp->attr->jumps_max;
    fp->physics.vel_air.y = (fp->fkind == REMIX_ID_GBOWSER) ? BOWSER_USP_G_INITIAL_SPEED : BOWSER_USP_INITIAL_SPEED;
}

/* BowserUSP.ground_physics_: ground horizontal movement, capped lower once temp variable 2 is set. */
static void ftRemixBowserUSPGroundProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 max_speed = (fp->motion_vars.flags.flag1 == 0) ? BOWSER_USP_GROUND_MAX_SPEED_HIGH : BOWSER_USP_GROUND_MAX_SPEED_LOW;

    ftPhysicsApplyClampGroundVelStickRange(fp, 0, BOWSER_USP_GROUND_ACCEL, max_speed);
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/* BowserUSP.air_physics_: aerial gravity, air drift and air friction. */
static void ftRemixBowserUSPAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    ftPhysicsApplyGravityClampTVel(fp, BOWSER_USP_GRAVITY, attr->tvel_base);
    if (fp->fkind == REMIX_ID_GBOWSER)
    {
        ftPhysicsClampAirVelXStickRange(fp, BOWSER_USP_X_STICK_RANGE_MIN, BOWSER_USP_G_X_ACCEL, BOWSER_USP_G_X_MAX_SPEED);
    }
    else
    {
        ftPhysicsClampAirVelXStickRange(fp, BOWSER_USP_X_STICK_RANGE_MIN, BOWSER_USP_X_ACCEL, BOWSER_USP_X_MAX_SPEED);
    }
    ftPhysicsApplyAirVelXFriction(fp, attr);
}

/* BowserDSP.air_physics_: copies the joint 0 Z scale onto X and Y, then runs the Y/Z transfer. */
static void ftRemixBowserDSPAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    DObj *joint = fp->joints[0];

    joint->scale.vec.f.y = joint->scale.vec.f.z;
    joint->scale.vec.f.x = joint->scale.vec.f.z;
    ftPhysicsApplyAirVelTransNYZ(fighter_gobj);
}

/* Grounded NSP reached from the aerial NSP. The ROM picks the Kirby action for
 * Kirby and J Kirby, and 0xE4 for everyone else. */
static void ftRemixBowserNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixBowserNSPStatusId(fp, nFTRemixBowserStatusFireBreath, nFTRemixBowserKirbyStatusFireBreathGround);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, action, fighter_gobj->anim_frame, DObjGetStruct(fighter_gobj)->anim_speed, FTSTATUS_PRESERVE_NONE);
}

/* G-Bowser's landing into air_to_ground_flame_cancel: the grounded NSP at
 * max(anim_frame, 16), with no Kirby branch. */
static void ftRemixBowserGBowserNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 frame = fighter_gobj->anim_frame;

    if (frame < 16.0F)
    {
        frame = 16.0F;
    }
    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixBowserStatusFireBreath, frame, DObjGetStruct(fighter_gobj)->anim_speed, FTSTATUS_PRESERVE_NONE);
}

/* Shared body of BowserNSP.air_collision_ and BowserNSP.gbowser_air_collision_:
 * lands as proc_cancel when the animation is before frame 20 or B is held;
 * otherwise it lands normally. */
static void ftRemixBowserNSPAirProcMapLanding(GObj *fighter_gobj, void (*proc_cancel)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    void (*proc_landing)(GObj *) = proc_cancel;

    if ((fighter_gobj->anim_frame >= 20.0F) && !(fp->input.pl.button_hold & B_BUTTON))
    {
        proc_landing = mpCommonSetFighterWaitOrLanding;
    }
    mpCommonProcFighterLanding(fighter_gobj, proc_landing);
}

/* BowserNSP.air_collision_ */
static void ftRemixBowserNSPAirProcMap(GObj *fighter_gobj)
{
    ftRemixBowserNSPAirProcMapLanding(fighter_gobj, ftRemixBowserNSPAirToGround);
}

/* BowserNSP.gbowser_air_collision_ */
static void ftRemixBowserGBowserNSPAirProcMap(GObj *fighter_gobj)
{
    ftRemixBowserNSPAirProcMapLanding(fighter_gobj, ftRemixBowserGBowserNSPAirToGround);
}

/* BowserUSP.air_direction_: turns the fighter once when temp variable 2 is 2
 * (moveset command 58000002), then sets it to 1. */
static void ftRemixBowserUSPAirDirection(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 2)
    {
        return;
    }
    /* 0x80160370: Captain Falcon's turn subroutine, copied by the Remix code. */
    ftCaptainSpecialHiProcInterrupt(fighter_gobj);
    fp->motion_vars.flags.flag1 = 1;
}

/* BowserFThrow.transition_1_: thrower enters status 0xE6 and the grabbed
 * fighter stops ignoring dead states. The grabbed fighter is read before the
 * status change, as the ROM does. */
static void ftRemixBowserFThrowTransition1(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTStruct *catch_fp = ftGetStruct(fp->catch_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixBowserStatusFThrowStep1, 0.0F, 1.0F, FTSTATUS_PRESERVE_TEXTUREPART);
    catch_fp->is_ignore_dead = FALSE;
}

/* BowserFThrow.transition_2_: thrower lands and enters status 0xE8. */
static void ftRemixBowserFThrowTransition2(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixBowserStatusFThrowStep2, 0.0F, 1.0F, FTSTATUS_PRESERVE_TEXTUREPART);
}

/* BowserFThrow.main_: moves to transition_1_ once the animation has ended. */
static void ftRemixBowserFThrowProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixBowserFThrowTransition1);
}

/* BowserFThrow.collision_: after landing, moves to transition_2_ while the
 * thrower's vertical velocity is negative. */
static void ftRemixBowserFThrowProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!mpCommonCheckFighterLanding(fighter_gobj))
    {
        return;
    }
    if (fp->physics.vel_air.y < 0.0F)
    {
        ftRemixBowserFThrowTransition2(fighter_gobj);
    }
}

static sb32 ftRemixBowserIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Fire breath ammo: a halfword at 0x0ADE (Bowser) or 0x0AE2 (Kirby), a word at 0x0B30 (G-Bowser). */
static s32 *ftRemixBowserAmmoWord(FTStruct *fp)
{
    if (fp->fkind == REMIX_ID_GBOWSER)
    {
        return ftRemixBowserWordAt(fp, 0x0B30);
    }
    if (fp->fkind == REMIX_ID_BOWSER)
    {
        return ftRemixBowserWordAt(fp, 0x0ADC);
    }
    return ftRemixBowserWordAt(fp, 0x0AE0);
}

/* Reads the ammo as the ROM does: an unsigned halfword for Bowser and Kirby. */
static s32 ftRemixBowserAmmo(FTStruct *fp)
{
    s32 *ammo = ftRemixBowserAmmoWord(fp);

    if (fp->fkind == REMIX_ID_GBOWSER)
    {
        return *ammo;
    }
    return (u16) *ammo;
}

/* Subtracts `amount` from the halfword ammo. G-Bowser's ammo is never subtracted (the ROM skips it). */
static void ftRemixBowserAmmoSub(FTStruct *fp, s32 amount)
{
    *ftRemixBowserAmmoWord(fp) = (u16) (ftRemixBowserAmmo(fp) - amount);
}

/* BowserNSP.idletransition_: returns to the wait or fall status when the animation ends. */
static void ftRemixBowserNSPIdleTransition(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, mpCommonSetFighterWaitOrFall);
}

/* BowserNSP.main_ button part: a B release starts the wind-down, and the counter runs
 * for BOWSER_NSP_RELEASE_FRAMES frames after the release. */
static void ftRemixBowserNSPUpdateRelease(FTStruct *fp)
{
    s32 *release_frames = ftRemixBowserWordAt(fp, 0x0B2C);
    u16 b_mask = fp->input.button_mask_b;

    if ((fp->input.pl.button_hold & b_mask) == 0)
    {
        *ftRemixBowserWordAt(fp, 0x0B28) = 1;
    }
    if (*release_frames < BOWSER_NSP_RELEASE_FRAMES)
    {
        (*release_frames)++;
    }
    if ((*release_frames < BOWSER_NSP_RELEASE_FRAMES) && ((fp->input.pl.button_tap & b_mask) != 0))
    {
        *release_frames = 0;
    }
}

/* Out-of-ammo smoke. The positions are hard-coded in the ROM per fighter. */
static void ftRemixBowserNSPMakeSmokeOut(GObj *fighter_gobj, FTStruct *fp)
{
    Vec3f smoke_pos = { 81.0F, 304.0F, 260.0F };
    s32 joint_id = fp->attr->joint_itemlight_id;

    if (fp->fkind == REMIX_ID_GBOWSER)
    {
        smoke_pos.x = -30.0F;
        smoke_pos.y = 424.0F;
    }
    else if (ftRemixBowserIsKirby(fp))
    {
        smoke_pos.x = -300.0F;
        smoke_pos.y = -97.5F;
        smoke_pos.z = 0.0F;
        joint_id = BOWSER_NSP_SMOKE_JOINT_KIRBY;
    }
    ftParamMakeEffect(fighter_gobj, nEFKindDustLight, joint_id, &smoke_pos, NULL, -fp->lr, TRUE, FALSE);
    func_800269C0_275C0(BOWSER_NSP_FGM_SMOKE_OUT);
}

/* Smoke while the ammo lasts; the position is the Fire Flower haveammo offset. */
static void ftRemixBowserNSPMakeSmokeHaveAmmo(GObj *fighter_gobj, FTStruct *fp)
{
    Vec3f smoke_pos = { 0.0F, 0.0F, -180.0F };

    ftParamMakeEffect(fighter_gobj, nEFKindDustLight, 0, &smoke_pos, NULL, fp->lr, FALSE, FALSE);
    func_800269C0_275C0(BOWSER_NSP_FGM_SMOKE_HAVE_AMMO);
}

/* BowserNSP.main_ smoke timer: every BOWSER_NSP_LOOP_RELOAD frames, puffs smoke chosen by the ammo. */
static void ftRemixBowserNSPUpdateSmoke(GObj *fighter_gobj, FTStruct *fp)
{
    s32 *loop_timer = ftRemixBowserWordAt(fp, 0x0B20);

    (*loop_timer)--;
    if (*loop_timer != 0)
    {
        return;
    }
    *loop_timer = BOWSER_NSP_LOOP_RELOAD;
    if (ftRemixBowserAmmo(fp) == 0)
    {
        ftRemixBowserNSPMakeSmokeOut(fighter_gobj, fp);
        return;
    }
    ftRemixBowserNSPMakeSmokeHaveAmmo(fighter_gobj, fp);
}

/* Flame angle for `index`, from the Fire Flower angle table in the item's loaded file. */
static f32 ftRemixBowserNSPFlameAngle(s32 index)
{
    f32 *angles = (f32 *) ((uintptr_t) *dITFFlowerItemDesc.p_file + (intptr_t) llITCommonDataFFlowerFlameAngles);

    return angles[index];
}

/* Hitbox step of the Remix flame: moves the spawn point in front of Bowser, then makes the Fire Flower flame. */
static void ftRemixBowserNSPMakeFlame(GObj *fighter_gobj, Vec3f *flame_pos, Vec3f *flame_vel)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    /* REMIX-TODO(PokemonAnnouncer): flamethrower_announcement_ is skipped, not ported. */
    flame_pos->x += (f32) fp->lr * BOWSER_NSP_FLAME_SPAWN_X_OFFSET;
    flame_pos->y += BOWSER_NSP_FLAME_SPAWN_Y_OFFSET;
    itFFlowerWeaponFlameMakeWeapon(fighter_gobj, flame_pos, flame_vel);
}

/* BowserNSP.projectile_spawn_ and gbowser_projectile_spawn: launches one flame for `index`. G-Bowser
 * uses a fixed sine (-0.5 rad) and does not spend ammo. */
static void ftRemixBowserNSPShootFlame(GObj *fighter_gobj, Vec3f *flame_pos, s32 index, s32 ammo_sub)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 angle = ftRemixBowserNSPFlameAngle(index);
    f32 sin_arg = (fp->fkind == REMIX_ID_GBOWSER) ? BOWSER_NSP_FLAME_SIN_ARG_GBOWSER : angle;
    Vec3f flame_vel;

    flame_vel.x = __cosf(angle) * BOWSER_NSP_FLAME_VEL;
    flame_vel.y = __sinf(sin_arg) * BOWSER_NSP_FLAME_VEL;
    flame_vel.z = 0.0F;

    ftRemixBowserNSPMakeFlame(fighter_gobj, flame_pos, &flame_vel);
    if (fp->fkind != REMIX_ID_GBOWSER)
    {
        ftRemixBowserAmmoSub(fp, ammo_sub);
    }
}

/* Flame origin: the Fire Flower make_flame_offset scaled by 1 / size (2 / size for Kirby). The z
 * component comes from the ROM, not the table. */
static void ftRemixBowserNSPSetFlameOrigin(FTStruct *fp, Vec3f *flame_pos)
{
    sb32 is_kirby = ftRemixBowserIsKirby(fp);
    f32 numerator = is_kirby ? BOWSER_NSP_KIRBY_SCALE_NUMERATOR : 1.0F;
    f32 scale = numerator / fp->attr->size;
    f32 origin_z = is_kirby ? BOWSER_NSP_FLAME_ORIGIN_Z_KIRBY : BOWSER_NSP_FLAME_ORIGIN_Z_BOWSER;

    flame_pos->x = BOWSER_NSP_FLAME_ORIGIN_X * scale;
    flame_pos->y = BOWSER_NSP_FLAME_ORIGIN_Y * scale;
    flame_pos->z = origin_z * scale;
}

/* Spawns one flame from the flame joint (joint 13 for Kirby) at the flame origin. Index mirrors past 5. */
static void ftRemixBowserNSPSpawnFlame(GObj *fighter_gobj, FTStruct *fp, s32 ammo_sub)
{
    Vec3f flame_pos;
    s32 joint_id = ftRemixBowserIsKirby(fp) ? BOWSER_NSP_FLAME_JOINT_KIRBY : BOWSER_NSP_FLAME_JOINT;
    s32 index;

    ftRemixBowserNSPSetFlameOrigin(fp, &flame_pos);
    gmCollisionGetFighterPartsWorldPosition(fp->joints[joint_id], &flame_pos);

    index = *ftRemixBowserWordAt(fp, 0x0B18);
    if (index >= BOWSER_NSP_FLAME_INDEX_MIRROR)
    {
        index = BOWSER_NSP_FLAME_INDEX_MAX - index;
    }
    ftRemixBowserNSPShootFlame(fighter_gobj, &flame_pos, index, ammo_sub);
    ftParamMakeRumble(fp, BOWSER_NSP_RUMBLE_ID, 0);
}

/* Post-spawn tail of BowserNSP.part2_: advances the frame count (capped) and the flame index, and
 * on the wrap sets the Fire Flower shoot attack. */
static void ftRemixBowserNSPAdvanceCounters(FTStruct *fp)
{
    s32 *active_frames = ftRemixBowserWordAt(fp, 0x0B24);
    s32 *flame_index = ftRemixBowserWordAt(fp, 0x0B18);
    s32 next_frames = *active_frames + 1;
    s32 next_index = *flame_index + 1;

    *active_frames = (next_frames < BOWSER_NSP_FRAMES_MAX + 1) ? next_frames : BOWSER_NSP_FRAMES_MAX;
    if (next_index < BOWSER_NSP_FLAME_INDEX_MAX)
    {
        *flame_index = next_index;
        return;
    }
    *flame_index = 0;
    ftParamSetMotionID(fp, nFTMotionAttackIDFireFlowerShoot);
    ftParamSetStatUpdate(fp, fp->stat_flags.halfword);
    ftParamUpdate1PGameAttackStats(fp, 0);
}

/* BowserNSP.part2_: spawns a flame while the ammo covers `ammo_sub`, then advances the counters. */
static void ftRemixBowserNSPPart2(GObj *fighter_gobj, s32 ammo_sub)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixBowserAmmo(fp) >= ammo_sub)
    {
        ftRemixBowserNSPSpawnFlame(fighter_gobj, fp, ammo_sub);
    }
    ftRemixBowserNSPAdvanceCounters(fp);
}

/* BowserNSP.main_ loop step: counts down the step, and on the wrap runs BowserNSP.part2_. */
static void ftRemixBowserNSPUpdateStep(GObj *fighter_gobj, FTStruct *fp, s32 ammo_sub)
{
    s32 *loop_step = ftRemixBowserWordAt(fp, 0x0B1C);

    (*loop_step)--;
    if (*loop_step != 0)
    {
        return;
    }
    *loop_step = BOWSER_NSP_LOOP_STEPS;
    ftRemixBowserNSPPart2(fighter_gobj, ammo_sub);
}

/* First-frame effect and the switch to flag 2. The dust kick plays only when the ammo covers the first shot. */
static void ftRemixBowserNSPStartFlame(GObj *fighter_gobj, FTStruct *fp, s32 ammo_sub)
{
    Vec3f dust_pos = { 0.0F, 0.0F, -180.0F };

    if (ftRemixBowserAmmo(fp) >= ammo_sub)
    {
        ftParamMakeEffect(fighter_gobj, nEFKindDustDashSmall, 0, &dust_pos, NULL, fp->lr, FALSE, FALSE);
    }
    fp->motion_vars.flags.flag0 = 2;
    gcSetAnimSpeed(fighter_gobj, 0.0F);
}

/* Flame frame, run while flag 0 is set: loop timer and smoke, the loop step, then the first-frame
 * effect. ammo_sub is 2 on the first frame and 1 after, as the Fire Flower shoot. */
static void ftRemixBowserNSPUpdateFlame(GObj *fighter_gobj, FTStruct *fp)
{
    s32 ammo_sub = (*ftRemixBowserWordAt(fp, 0x0B24) != 0) ? 1 : 2;

    ftRemixBowserNSPUpdateSmoke(fighter_gobj, fp);
    ftRemixBowserNSPUpdateStep(fighter_gobj, fp, ammo_sub);
    if (fp->motion_vars.flags.flag0 == 1)
    {
        ftRemixBowserNSPStartFlame(fighter_gobj, fp, ammo_sub);
    }
}

/* Ends the flame once it has run BOWSER_NSP_ACTIVE_FRAMES frames and B was released long enough. */
static void ftRemixBowserNSPUpdateEnd(GObj *fighter_gobj, FTStruct *fp)
{
    if (*ftRemixBowserWordAt(fp, 0x0B24) < BOWSER_NSP_ACTIVE_FRAMES)
    {
        return;
    }
    if (*ftRemixBowserWordAt(fp, 0x0B28) == 0)
    {
        return;
    }
    if (*ftRemixBowserWordAt(fp, 0x0B2C) < BOWSER_NSP_RELEASE_FRAMES)
    {
        return;
    }
    fp->motion_vars.flags.flag0 = 0;
    gcSetAnimSpeed(fighter_gobj, 1.0F);
}

/* BowserNSP.main_: Fire Breath main proc. Holding B keeps the flame going; a release winds it down. */
static void ftRemixBowserNSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixBowserNSPIdleTransition(fighter_gobj);
    ftRemixBowserNSPUpdateRelease(fp);
    if (fp->motion_vars.flags.flag0 != 0)
    {
        ftRemixBowserNSPUpdateFlame(fighter_gobj, fp);
    }
    ftRemixBowserNSPUpdateEnd(fighter_gobj, fp);
}

/* Remix address -> port routine for every Bowser routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixBowserProcs[] =
{
    { REMIX_SYM_BowserNSP__ground_initial_, ftRemixBowserNSPGroundSetStatus },
    { REMIX_SYM_BowserNSP__air_initial_, ftRemixBowserNSPAirSetStatus },
    { REMIX_SYM_BowserNSP__air_collision_, ftRemixBowserNSPAirProcMap },
    { REMIX_SYM_BowserNSP__gbowser_air_collision_, ftRemixBowserGBowserNSPAirProcMap },
    { REMIX_SYM_BowserUSP__air_initial_, ftRemixBowserUSPAirSetStatus },
    { REMIX_SYM_BowserUSP__air_direction_, ftRemixBowserUSPAirDirection },
    { REMIX_SYM_BowserUSP__ground_physics_, ftRemixBowserUSPGroundProcPhysics },
    { REMIX_SYM_BowserUSP__air_physics_, ftRemixBowserUSPAirProcPhysics },
    { REMIX_SYM_BowserDSP__air_physics_, ftRemixBowserDSPAirProcPhysics },
    { REMIX_SYM_BowserFThrow__main_, ftRemixBowserFThrowProcUpdate },
    { REMIX_SYM_BowserFThrow__collision_, ftRemixBowserFThrowProcMap },
    { REMIX_SYM_BowserNSP__main_, ftRemixBowserNSPProcUpdate }
};

const s32 dFTRemixBowserProcsNum = ARRAY_COUNT(dFTRemixBowserProcs);

#endif
