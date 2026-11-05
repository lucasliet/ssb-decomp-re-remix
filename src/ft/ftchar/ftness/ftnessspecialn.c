#include <ft/fighter.h>
#include <wp/weapon.h>
#ifdef REGION_REMIX
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixnessshared.h"
#endif
// // // // // // // // // // // //
//                               //
//             MACROS            //
//                               //
// // // // // // // // // // // //

#define FTNESS_SPECIALN_STATUS_FLAGS (FTSTATUS_PRESERVE_TEXTUREPART | FTSTATUS_PRESERVE_HITSTATUS | FTSTATUS_PRESERVE_EFFECT | FTSTATUS_PRESERVE_COLANIM)

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80153950
/* Size.asm ness.nsp.adjust_pkfire_position_ (0x801539B4): the X and Y spawn offsets are scaled by the owner multiplier. */
void ftNessSpecialNProcAccessory(GObj *fighter_gobj) // PK Fire setup
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos;
    Vec3f vel;
    f32 angle;
    sb32 is_air = (fp->ga == nMPKineticsAir);
    f32 speed_air = FTNESS_PKFIRE_SPARK_VEL_AIR;
    f32 speed_ground = FTNESS_PKFIRE_SPARK_VEL_GROUND;

    if (fp->motion_vars.flags.flag0 != FALSE) // Check if flag to summon PK Fire is true
    {
        fp->motion_vars.flags.flag0 = FALSE; // Revert to 0 if PK Fire is summoned, so it doesn't repeat on every frame of the move

        pos.x = 0.0F;
        pos.y = 0.0F;
        pos.z = 0.0F;

        gmCollisionGetFighterPartsWorldPosition(fp->joints[FTNESS_PKFIRE_SPAWN_JOINT], &pos);

#ifdef REGION_REMIX
        pos.x += FTNESS_PKFIRE_SPAWN_OFF_X * fp->lr * ftRemixSizeGetMultiplier(fp->player);
        pos.y += FTNESS_PKFIRE_SPAWN_OFF_Y * ftRemixSizeGetMultiplier(fp->player);
#else
        pos.x += FTNESS_PKFIRE_SPAWN_OFF_X * fp->lr;
        pos.y += FTNESS_PKFIRE_SPAWN_OFF_Y;
#endif
        pos.z = 0.0F;

#ifdef REGION_REMIX
        FTRemixNessPKFireLaunch launch = { is_air, speed_air, speed_ground };

        ftRemixNessSharedPKFireLaunch(fp, &launch);

        is_air = launch.is_air;
        speed_air = launch.speed_air;
        speed_ground = launch.speed_ground;
#endif
        if (is_air)
        {
            vel.z = 0.0F;
            angle = FTNESS_PKFIRE_SPARK_ANGLE_AIR;
            vel.x = __cosf(FTNESS_PKFIRE_SPARK_ANGLE_AIR) * speed_air * fp->lr;
            vel.y = __sinf(FTNESS_PKFIRE_SPARK_ANGLE_AIR) * speed_air;
        }
        else
        {
            vel.z = 0.0F;
            angle = FTNESS_PKFIRE_SPARK_ANGLE_GROUND;
            vel.x = __cosf(FTNESS_PKFIRE_SPARK_ANGLE_GROUND) * speed_ground * fp->lr;
            vel.y = __sinf(FTNESS_PKFIRE_SPARK_ANGLE_GROUND) * speed_ground;
        }
        wpNessPKFireMakeWeapon(fighter_gobj, &pos, &vel, angle); // Spawn PK Fire
    }
}

// 0x80153AC0
void ftNessSpecialNProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftNessSpecialNSwitchStatusAir);
}

// 0x80153AE4
void ftNessSpecialAirNProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftNessSpecialAirNSwitchStatusGround);
}

// 0x80153B08
void ftNessSpecialAirNSwitchStatusGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);

    ftMainSetStatus(fighter_gobj, nFTNessStatusSpecialN, fighter_gobj->anim_frame, 1.0F, FTNESS_SPECIALN_STATUS_FLAGS); // Action State Change

    fp->proc_accessory = ftNessSpecialNProcAccessory;
}

// 0x80153B5C
void ftNessSpecialNSwitchStatusAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);

    ftMainSetStatus(fighter_gobj, nFTNessStatusSpecialAirN, fighter_gobj->anim_frame, 1.0F, FTNESS_SPECIALN_STATUS_FLAGS); // Action State Change

    ftPhysicsClampAirVelXMax(fp);

    fp->proc_accessory = ftNessSpecialNProcAccessory;
}

// 0x80153BB8
void ftNessSpecialNInitStatusVars(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->motion_vars.flags.flag0 = FALSE;
    fp->proc_accessory = ftNessSpecialNProcAccessory;
}

// 0x80153BD0
void ftNessSpecialNSetStatus(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTNessStatusSpecialN, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftNessSpecialNInitStatusVars(fighter_gobj);
}

// 0x80153C10
void ftNessSpecialAirNSetStatus(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTNessStatusSpecialAirN, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftNessSpecialNInitStatusVars(fighter_gobj);
}
