/**
 * ftremixsizemove.c - Smash Remix body size subsystem, phase (b) of Size.asm: movement physics.
 *
 * Scales walk, brake, dash, run and jump values by the size multiplier of the fighter, as the
 * Size.asm adjust_player_*_ and adjust_jumping_height_multiplier_* patches do. Every helper returns
 * its vanilla value while the multiplier is 1.0F.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "it/itremix/itremixpwing.h"

#define FTREMIX_SIZE_PWING_JUMP_SCALE 0.8F

/** Walking speed (Size.asm adjust_player_walking_speed_, RAM 0x800D8A74, ftPhysicsSetGroundVelAbsStickRange). */
f32 ftRemixSizeWalkingSpeed(FTStruct *fp, f32 vanilla_speed)
{
    return vanilla_speed * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_WALKING_SPEED);
}

/** Brake force of walking (Size.asm adjust_player_walking_speed_, same patch, friction argument). */
f32 ftRemixSizeBrakeForce(FTStruct *fp, f32 vanilla_force)
{
    return vanilla_force * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_BRAKE_FORCE);
}

/** Dash deceleration (Size.asm adjust_player_dash_deceleration_, RAM 0x800D897C, ftPhysicsSetGroundVelFriction). */
f32 ftRemixSizeDashDecel(FTStruct *fp, f32 vanilla_decel)
{
    return vanilla_decel * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_DASH_DECEL);
}

/** Initial dash speed (Size.asm adjust_player_initial_dash_speed_, RAM 0x8013ED40, ftCommonDashSetStatus). */
f32 ftRemixSizeInitialDashSpeed(FTStruct *fp, f32 vanilla_speed)
{
    return vanilla_speed * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_DASHING_SPEED);
}

/** Running speed (Size.asm adjust_player_running_speed_, RAM 0x8013EF14, ftCommonRunSetStatus). */
f32 ftRemixSizeRunSpeed(FTStruct *fp, f32 vanilla_speed)
{
    return vanilla_speed * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_RUNNING_SPEED);
}

/**
 * Jump squat length in frames (Size.asm adjust_player_jump_squat_, RAM 0x8013F308, ftCommonKneeBendProcUpdate).
 * Giant fighters (multiplier above 1.0F) get one extra frame.
 */
f32 ftRemixSizeJumpSquatLength(FTStruct *fp, f32 vanilla_length)
{
    if (ftRemixSizeGetMultiplier(fp->player) <= 1.0F)
    {
        return vanilla_length;
    }
    return vanilla_length + 1.0F;
}

/** Jumping height multiplier (Size.asm adjust_jumping_height_multiplier_1, RAM 0x8013F94C, ftCommonJumpSetStatus). */
f32 ftRemixSizeJumpHeightMul(FTStruct *fp, f32 vanilla_mul)
{
    return vanilla_mul * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_JUMPING_HEIGHT);
}

/** Base jumping height (Size.asm adjust_jumping_height_multiplier_1, same patch, base height). */
f32 ftRemixSizeJumpHeightBase(FTStruct *fp, f32 vanilla_base)
{
    return vanilla_base * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_BASE_JUMPING_HEIGHT);
}

/**
 * Aerial jump height scale of the multi-jump fighters (Size.asm adjust_jumping_height_multiplier_kirby at RAM 0x80140014,
 * ftCommonJumpAerialMultiSetStatus). Pwing does not change it.
 */
f32 ftRemixSizeAerialJumpHeight(FTStruct *fp, f32 vanilla_height)
{
    return vanilla_height * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_SECOND_JUMPING_HEIGHT);
}

/**
 * Aerial jump height scale of the single aerial jump (Size.asm adjust_jumping_height_multiplier_2 at RAM 0x8013FE64,
 * ftCommonJumpAerialSetStatus), 0.8 times lower while the fighter jumps with Pwing (_check_pwing).
 */
f32 ftRemixSizeAerialJumpHeightSingle(FTStruct *fp, f32 vanilla_height)
{
    f32 height = ftRemixSizeAerialJumpHeight(fp, vanilla_height);

    if (itRemixPwingIsJumping(fp) != FALSE)
    {
        height *= FTREMIX_SIZE_PWING_JUMP_SCALE;
    }
    return height;
}

#endif
