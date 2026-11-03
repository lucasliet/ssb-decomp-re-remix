#include <ft/fighter.h>
#include <it/item.h>
#ifdef REGION_REMIX
#include "it/itremix/itremixgoldengun.h"
#endif
extern void *func_800269C0_275C0(u16 id);
#ifdef PORT
extern void port_log(const char *fmt, ...);
#endif

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80146FB0
void ftCommonLGunShootProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, mpCommonSetFighterWaitOrFall);
}

// 0x80146FD4
void ftCommonLGunShootProcAccessory(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    ITStruct *ip;
    f32 size_mul;

    if ((fp->item_gobj != NULL) && (fp->motion_vars.flags.flag0 != 0))
    {
        ip = itGetStruct(fp->item_gobj);

        if (ip->multi != 0)
        {
            // 0x80188660
            Vec3f make_ammo_offset = { 0.0F, 60.0F, 180.0F };
            Vec3f make_effect_offset;

            size_mul = 1.0F / fp->attr->size;

            make_ammo_offset.x *= size_mul;
            make_ammo_offset.y *= size_mul;
            make_ammo_offset.z *= size_mul;

            gmCollisionGetFighterPartsWorldPosition(fp->joints[fp->attr->joint_itemlight_id], &make_ammo_offset);
            itLGunMakeAmmo(fighter_gobj, &make_ammo_offset);

            make_effect_offset.x = 0.0F;
            make_effect_offset.y = 60.0F;
            make_effect_offset.z = 180.0F;

            ftParamMakeEffect(fighter_gobj, nEFKindSparkleWhiteScale, fp->attr->joint_itemlight_id, &make_effect_offset, NULL, fp->lr, TRUE, FALSE);

            make_effect_offset.x = 0.0F;
            make_effect_offset.y = 0.0F;
            make_effect_offset.z = -180.0F;

            ftParamMakeEffect(fighter_gobj, nEFKindDustDashSmall, nFTPartsJointTopN, &make_effect_offset, NULL, fp->lr, FALSE, FALSE);

#ifdef REGION_REMIX
            /* GoldenGun.asm projectile_shoot_fgm_fix (0x80147134): the golden gun plays
             * FGM 0x04C2 where the ray gun plays nSYAudioFGMLGunShoot. The golden bullet
             * is spawned in itlgun.c (projectile_struct_fix and projectile_properties_fix). */
            func_800269C0_275C0((ip->kind == ITREMIX_GOLDENGUN_ID) ? 0x04C2 : nSYAudioFGMLGunShoot);
#else
            func_800269C0_275C0(nSYAudioFGMLGunShoot);
#endif
            ftParamMakeRumble(fp, 9, 0);
        }
        else
        {
            // 0x8018866C
            Vec3f dust_effect_offset = { FTCOMMON_LGUNSHOOT_AMMO_SPAWN_OFF_X, FTCOMMON_LGUNSHOOT_AMMO_SPAWN_OFF_Y, FTCOMMON_LGUNSHOOT_AMMO_SPAWN_OFF_Z };

            ftParamMakeEffect(fighter_gobj, nEFKindDustLight, fp->attr->joint_itemlight_id, &dust_effect_offset, NULL, -fp->lr, TRUE, FALSE);
#ifdef REGION_REMIX
            /* GoldenGun.asm projectile_no_ammo_fgm_fix (0x801471A0): the golden gun plays
             * FGM 0x04C3 where the ray gun plays nSYAudioFGMLGunEmpty. */
            func_800269C0_275C0((ip->kind == ITREMIX_GOLDENGUN_ID) ? 0x04C3 : nSYAudioFGMLGunEmpty);
#else
            func_800269C0_275C0(nSYAudioFGMLGunEmpty);
#endif
        }
        fp->motion_vars.flags.flag0 = 0;
    }
}

// 0x801471C0
void ftCommonLGunShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftCommonLGunShootSwitchStatusAir);
}

// 0x801471E4
void ftCommonLGunShootAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftCommonLGunShootAirSwitchStatusGround);
}

// 0x80147208
void ftCommonLGunShootAirSwitchStatusGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTCommonStatusLGunShoot, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);

    fp->proc_accessory = ftCommonLGunShootProcAccessory;
}

// 0x80147258
void ftCommonLGunShootSwitchStatusAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTCommonStatusLGunShootAir, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);

    fp->proc_accessory = ftCommonLGunShootProcAccessory;
}

// 0x801472B0
void ftCommonFireFlowerShootProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, mpCommonSetFighterWaitOrFall);
}

// 0x801472D4
void ftCommonFireFlowerShootUpdateAmmoStats(FTStruct *fp, s32 ammo_sub)
{
    ITStruct *ip = itGetStruct(fp->item_gobj);

    // 0x80188678
    Vec3f make_flame_offset = { 60.0F, 100.0F, 0.0F };
    
    f32 size_mul;
    s32 flame_vel_index;

    if (ip->multi >= ammo_sub)
    {
        size_mul = 1.0F / fp->attr->size;

        make_flame_offset.x *= size_mul;
        make_flame_offset.y *= size_mul;
        make_flame_offset.z *= size_mul;

        gmCollisionGetFighterPartsWorldPosition(fp->joints[fp->attr->joint_itemlight_id], &make_flame_offset);

        if (fp->status_vars.common.fireflower.flame_vel_index >= FTCOMMON_FIREFLOWERSHOOT_AMMO_INDEX_LOOP)
        {
            flame_vel_index = FTCOMMON_FIREFLOWERSHOOT_AMMO_INDEX_MAX - fp->status_vars.common.fireflower.flame_vel_index;
        }
        else flame_vel_index = fp->status_vars.common.fireflower.flame_vel_index;

        itFFlowerShootFlame(fp->fighter_gobj, &make_flame_offset, flame_vel_index, ammo_sub);
        ftParamMakeRumble(fp, 6, 0);
    }
    fp->status_vars.common.fireflower.ammo_fire_count++;

    if (fp->status_vars.common.fireflower.ammo_fire_count > (U16_MAX + 1))
    {
        fp->status_vars.common.fireflower.ammo_fire_count = (U16_MAX + 1);
    }
    fp->status_vars.common.fireflower.flame_vel_index++;

    if (fp->status_vars.common.fireflower.flame_vel_index >= FTCOMMON_FIREFLOWERSHOOT_AMMO_INDEX_MAX)
    {
        fp->status_vars.common.fireflower.flame_vel_index = 0;

        ftParamSetMotionID(fp, nFTMotionAttackIDFireFlowerShoot);
        ftParamSetStatUpdate(fp, fp->stat_flags.halfword);
        ftParamUpdate1PGameAttackStats(fp, 0);
    }
}

// 0x80147434
void ftCommonFireFlowerShootProcAccessory(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    ITStruct *ip;
    s32 ammo_sub;

    if (!(fp->input.pl.button_hold & fp->input.button_mask_a))
    {
        fp->status_vars.common.fireflower.is_release = TRUE;
    }
    if (fp->status_vars.common.fireflower.release_lag < FTCOMMON_FIREFLOWERSHOOT_RELEASE_LAG)
    {
        fp->status_vars.common.fireflower.release_lag++;
    }
    if ((fp->status_vars.common.fireflower.release_lag < FTCOMMON_FIREFLOWERSHOOT_RELEASE_LAG) && (fp->input.pl.button_tap & fp->input.button_mask_a))
    {
        fp->status_vars.common.fireflower.release_lag = 0;
    }
    if (fp->item_gobj != NULL)
    {
        if (fp->motion_vars.flags.flag0 != 0)
        {
            ip = itGetStruct(fp->item_gobj);

            ammo_sub = (fp->status_vars.common.fireflower.ammo_fire_count == 0) ? 2 : 1;

            fp->status_vars.common.fireflower.effect_make_int--;

            if (fp->status_vars.common.fireflower.effect_make_int == 0)
            {
                fp->status_vars.common.fireflower.effect_make_int = FTCOMMON_FIREFLOWERSHOOT_EFFECT_SPAWN_INT;

                if (ip->multi < ammo_sub)
                {
                    // 0x80188684
                    Vec3f effect_noammo_offset = { 60.0F, 100.0F, 0.0F };

                    ftParamMakeEffect(fighter_gobj, nEFKindDustLight, fp->attr->joint_itemlight_id, &effect_noammo_offset, NULL, -fp->lr, TRUE, FALSE);
                    func_800269C0_275C0(nSYAudioFGMFireFlowerBurn);
                }
                else
                {
                    // 0x80188690
                    Vec3f effect_haveammo_offset = { 0.0F, 0.0F, -180.0F };

                    ftParamMakeEffect(fighter_gobj, nEFKindDustLight, nFTPartsJointTopN, &effect_haveammo_offset, NULL, fp->lr, FALSE, FALSE);
                    func_800269C0_275C0(nSYAudioFGMBurnE);
                }
            }
            fp->status_vars.common.fireflower.ammo_sub--;

            if (fp->status_vars.common.fireflower.ammo_sub == 0)
            {
                fp->status_vars.common.fireflower.ammo_sub = FTCOMMON_FIREFLOWERSHOOT_AMMO_INDEX_MAX;

                ftCommonFireFlowerShootUpdateAmmoStats(fp, ammo_sub);
            }
            if (fp->motion_vars.flags.flag0 == 1)
            {
                if (ip->multi >= ammo_sub)
                {
                    // 0x8018869C
                    Vec3f effect_spark_offset = { 0.0F, 80.0F, 0.0F };

                    // 0x801886A8
                    Vec3f effect_spark_scatter = { 90.0F, 90.0F, 90.0F };

                    // 0x801886B4
                    Vec3f effect_dust_offset = { 0.0F, 0.0F, -180.0F };

                    ftParamMakeEffect(fighter_gobj, nEFKindSparkleWhiteScale, fp->attr->joint_itemlight_id, &effect_spark_offset, &effect_spark_scatter, fp->lr, TRUE, FALSE);
                    ftParamMakeEffect(fighter_gobj, nEFKindDustDashSmall, nFTPartsJointTopN, &effect_dust_offset, NULL, fp->lr, FALSE, FALSE);
                }
                fp->motion_vars.flags.flag0 = 2;

                gcSetAnimSpeed(fighter_gobj, 0.0F);
            }
        }
        if ((fp->status_vars.common.fireflower.ammo_fire_count >= 5) && (fp->status_vars.common.fireflower.is_release != FALSE) && (fp->status_vars.common.fireflower.release_lag >= 20))
        {
            fp->motion_vars.flags.flag0 = 0;

            gcSetAnimSpeed(fighter_gobj, 1.0F);
        }
    }
}

// 0x8014772C
void ftCommonFireFlowerShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftCommonFireFlowerShootSwitchStatusAir);
}

// 0x80147750
void ftCommonFireFlowerShootAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftCommonFireFlowerShootAirSwitchStatusGround);
}

// 0x80147774
void ftCommonFireFlowerShootAirSwitchStatusGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTCommonStatusFireFlowerShoot, fighter_gobj->anim_frame, DObjGetStruct(fighter_gobj)->anim_speed, FTSTATUS_PRESERVE_NONE);

    fp->proc_accessory = ftCommonFireFlowerShootProcAccessory;
}

// 0x801477C8
void ftCommonFireFlowerShootSwitchStatusAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTCommonStatusFireFlowerShootAir, fighter_gobj->anim_frame, DObjGetStruct(fighter_gobj)->anim_speed, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);

    fp->proc_accessory = ftCommonFireFlowerShootProcAccessory;
}

// 0x80147824
void ftCommonFireFlowerShootInitStatusVars(FTStruct *fp)
{
    fp->status_vars.common.fireflower.flame_vel_index = 0;
    fp->status_vars.common.fireflower.ammo_sub = 1;
    fp->status_vars.common.fireflower.effect_make_int = 1;
    fp->status_vars.common.fireflower.ammo_fire_count = 0;
    fp->status_vars.common.fireflower.is_release = FALSE;
    fp->status_vars.common.fireflower.release_lag = 0;
}

// 0x80147844
void ftCommonItemShootSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    ITStruct *ip = itGetStruct(fp->item_gobj);
    s32 status_id;
    void (*proc_accessory)(GObj*);

    switch (ip->kind)
    {
    case nITKindLGun:
        status_id = nFTCommonStatusLGunShoot;
        proc_accessory = ftCommonLGunShootProcAccessory;
        break;

    case nITKindFFlower:
        status_id = nFTCommonStatusFireFlowerShoot;
        proc_accessory = ftCommonFireFlowerShootProcAccessory;
        break;
#ifdef REGION_REMIX
    /* GoldenGun.asm extend_grounded_shoot_item_id_check (0x80147888): the golden gun
     * takes the ray gun shoot action, at half frame speed (see ftMainSetStatus below). */
    case ITREMIX_GOLDENGUN_ID:
        status_id = nFTCommonStatusLGunShoot;
        proc_accessory = ftCommonLGunShootProcAccessory;
        break;
#endif
#ifdef PORT
    default:
        /* Guard against ip->type miscoding as nITTypeShoot for non-shootable items
         * (observed: pokeball, pill bottle crash here because the caller in
         * ftCommonAttack1CheckInterruptCommon routes any nITTypeShoot to this
         * function). Falling through without initializing status_id / proc_accessory
         * corrupts ftMainSetStatus with garbage and triggers SIGBUS in
         * ftMainProcPhysicsMap. Bail safely instead. */
        port_log("SSB64: !!! ftCommonItemShootSetStatus: unexpected ip->kind=%d type=%d "
                 "item_gobj=%p — aborting to avoid UB\n",
            ip->kind, ip->type, fp->item_gobj);
        return;
#endif
    }
    fp->motion_vars.flags.flag0 = 0;

#ifdef REGION_REMIX
    /* GoldenGun.asm extend_grounded_shoot_item_id_check (0x80147888): the golden gun
     * enters the common tail with a3 = 0x3F00 (0.5F) instead of the vanilla 1.0F. */
    if (ip->kind == ITREMIX_GOLDENGUN_ID)
    {
        ftMainSetStatus(fighter_gobj, status_id, 0.0F, 0.5F, FTSTATUS_PRESERVE_NONE);
    }
    else ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
#else
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
#endif
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp->proc_accessory = proc_accessory;

    ftCommonFireFlowerShootInitStatusVars(fp);
}

// 0x801478EC
void ftCommonItemShootAirSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    ITStruct *ip = itGetStruct(fp->item_gobj);
    s32 status_id;
    void (*proc_accessory)(GObj*);

    switch (ip->kind)
    {
    case nITKindLGun:
        status_id = nFTCommonStatusLGunShootAir;
        proc_accessory = ftCommonLGunShootProcAccessory;
        break;

    case nITKindFFlower:
        status_id = nFTCommonStatusFireFlowerShootAir;
        proc_accessory = ftCommonFireFlowerShootProcAccessory;
        break;
#ifdef REGION_REMIX
    /* GoldenGun.asm extend_aerial_shoot_item_id_check (0x80147930): the golden gun
     * takes the ray gun aerial shoot action, at half frame speed. */
    case ITREMIX_GOLDENGUN_ID:
        status_id = nFTCommonStatusLGunShootAir;
        proc_accessory = ftCommonLGunShootProcAccessory;
        break;
#endif
    }
    fp->motion_vars.flags.flag0 = 0;

#ifdef REGION_REMIX
    /* GoldenGun.asm extend_aerial_shoot_item_id_check (0x80147930): the golden gun
     * enters the common tail with a3 = 0x3F00 (0.5F) instead of the vanilla 1.0F. */
    if (ip->kind == ITREMIX_GOLDENGUN_ID)
    {
        ftMainSetStatus(fighter_gobj, status_id, 0.0F, 0.5F, FTSTATUS_PRESERVE_NONE);
    }
    else ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
#else
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
#endif
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp->proc_accessory = proc_accessory;

    ftPhysicsClampAirVelXMax(fp);
    ftCommonFireFlowerShootInitStatusVars(fp);
}
