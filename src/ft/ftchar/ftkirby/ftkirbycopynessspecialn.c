#include <ft/fighter.h>
#include <wp/weapon.h>
#ifdef REGION_REMIX
#include "ft/ftremix/ftremixnessshared.h"
#endif
// // // // // // // // // // // //
//                               //
//             MACROS            //
//                               //
// // // // // // // // // // // //

#define FTKIRBY_COPYNESS_SPECIALN_STATUS_FLAGS (FTSTATUS_PRESERVE_TEXTUREPART | FTSTATUS_PRESERVE_HITSTATUS | FTSTATUS_PRESERVE_EFFECT | FTSTATUS_PRESERVE_COLANIM)

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80155B40
void ftKirbyCopyNessSpecialNProcAccessory(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos;
    Vec3f vel;
    f32 angle;
    sb32 is_air = (fp->ga == nMPKineticsAir);
    f32 speed_air = FTKIRBY_COPYNESS_PKFIRE_SPARK_VEL_AIR;
    f32 speed_ground = FTKIRBY_COPYNESS_PKFIRE_SPARK_VEL_GROUND;

    if (fp->motion_vars.flags.flag0 != FALSE) // Check if flag to summon PK Fire is true
    {
        fp->motion_vars.flags.flag0 = FALSE; // Revert to 0 if PK Fire is summoned, so it doesn't repeat on every frame of the move

        pos.x = 0.0F;
        pos.y = 0.0F;
        pos.z = 0.0F;

        gmCollisionGetFighterPartsWorldPosition(fp->joints[FTKIRBY_COPYNESS_PKFIRE_SPAWN_JOINT], &pos);

        pos.x += FTKIRBY_COPYNESS_PKFIRE_SPAWN_OFF_X * fp->lr;
        pos.y += FTKIRBY_COPYNESS_PKFIRE_SPAWN_OFF_Y;

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
            angle = FTKIRBY_COPYNESS_PKFIRE_SPARK_ANGLE_AIR;
            vel.x = __cosf(FTKIRBY_COPYNESS_PKFIRE_SPARK_ANGLE_AIR) * speed_air * fp->lr;
            vel.y = __sinf(FTKIRBY_COPYNESS_PKFIRE_SPARK_ANGLE_AIR) * speed_air;
        }
        else
        {
            vel.z = 0.0F;
            angle = FTKIRBY_COPYNESS_PKFIRE_SPARK_ANGLE_GROUND;
            vel.x = __cosf(FTKIRBY_COPYNESS_PKFIRE_SPARK_ANGLE_GROUND) * speed_ground * fp->lr;
            vel.y = __sinf(FTKIRBY_COPYNESS_PKFIRE_SPARK_ANGLE_GROUND) * speed_ground;
        }

        wpNessPKFireMakeWeapon(fighter_gobj, &pos, &vel, angle); // Spawn PK Fire
    }
}

// 0x80155CAC
void ftKirbyCopyNessSpecialNProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftKirbyCopyNessSpecialNSwitchStatusAir);
}

// 0x80155CD0
void ftKirbyCopyNessSpecialAirNProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftKirbyCopyNessSpecialAirNSwitchStatusGround);
}

// 0x80155CF4
void ftKirbyCopyNessSpecialAirNSwitchStatusGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTKirbyStatusCopyNessSpecialN, fighter_gobj->anim_frame, 1.0F, FTKIRBY_COPYNESS_SPECIALN_STATUS_FLAGS);

    fp->proc_accessory = ftKirbyCopyNessSpecialNProcAccessory;
}

// 0x80155D48
void ftKirbyCopyNessSpecialNSwitchStatusAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTKirbyStatusCopyNessSpecialAirN, fighter_gobj->anim_frame, 1.0F, FTKIRBY_COPYNESS_SPECIALN_STATUS_FLAGS);
    ftPhysicsClampAirVelXMax(fp);

    fp->proc_accessory = ftKirbyCopyNessSpecialNProcAccessory;
}

// 0x80155DA4
void ftKirbyCopyNessSpecialNInitStatusVars(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->motion_vars.flags.flag0 = 0;

    fp->proc_accessory = ftKirbyCopyNessSpecialNProcAccessory;
}

// 0x80155DBC
void ftKirbyCopyNessSpecialNSetStatus(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTKirbyStatusCopyNessSpecialN, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftKirbyCopyNessSpecialNInitStatusVars(fighter_gobj);
}

// 0x80155DFC
void ftKirbyCopyNessSpecialAirNSetStatus(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTKirbyStatusCopyNessSpecialAirN, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftKirbyCopyNessSpecialNInitStatusVars(fighter_gobj);
}
