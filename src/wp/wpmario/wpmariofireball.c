#include <wp/weapon.h>
#include <ft/fighter.h>
#include <reloc_data.h>
#ifdef REGION_REMIX
#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_roster_data.h"
#endif
#ifdef PORT
extern void *func_800269C0_275C0(u16 id);
#endif

// // // // // // // // // // // //
//                               //
//       INITIALIZED DATA        //
//                               //
// // // // // // // // // // // //

// 0x80188E30
wpMarioFireballAttributes dWPMarioFireballWeaponAttributes[/* */] = 
{
    // Mario
    {
        140,                                      // Lifetime
        55.0F,                                    // Terminal velocity
        30.0F,                                    // Map collision damage
        1.2F,                                     // Gravity
        0.85F,                                    // Map collision rebound velocity?
        F_CLC_DTOR32(20.0F),                      // Rotate speed (0.349065870047F)
        F_CLC_DTOR32(-5.0F),                      // Grounded launch angle (-0.0872664675117F)
        F_CLC_DTOR32(-5.0F),                      // Aerial launch angle (-0.0872664675117F)
        50.0F,                                    // Base velocity
        &gFTMarioFileSpecial1,                    // Pointer to ???
#ifdef PORT
        llMarioSpecial1FireballWeaponAttributes, // Offset of hitbox/attributes?
#else
        &llMarioSpecial1FireballWeaponAttributes, // Offset of hitbox/attributes?
#endif
        0.0F                                      // Animation starting frame?
    },

    // Luigi
    {
#if defined(REGION_US)
        80,                                       // Lifetime
#else
        90,                                       // Lifetime
#endif
        55.0F,                                    // Terminal velocity
        30.0F,                                    // Map collision damage
        0.0F,                                     // Gravity
        0.85F,                                    // Map collision rebound velocity?
        F_CLC_DTOR32(25.0F),                      // Rotate speed (0.436332315207F)
        F_CLC_DTOR32(0.0F),                       // Grounded launch angle (0.0F)
        F_CLC_DTOR32(0.0F),                       // Aerial launch angle (0.0F)
        36.0F,                                    // Base velocity
        &gFTDataLuigiSpecial1,                    // Pointer to ???
#ifdef PORT
        llLuigiSpecial1FireballWeaponAttributes, // Offset of hitbox/attributes?
#else
        &llLuigiSpecial1FireballWeaponAttributes, // Offset of hitbox/attributes?
#endif
        1.0F                                      // Animation starting frame?
    },
#ifdef REGION_REMIX
    // Remix clones, ids 2..6 in fireball.asm struct order. p_weapon is NULL: the projectile file is
    // resolved from file_fkind in wpMarioFireballMakeWeaponAtAngle.
    // Capsule (Dr. Mario); palette is random in 0-2 (Kirby: by hat).
    {
        140, 60.0F, 25.0F, 1.5F, 0.95F, 0.3F, -0.4F, -0.4F, 40.0F,
        NULL, 0, 0.0F,
        REMIX_ID_DRM, 0, 3
    },

    // J Mario
    {
        140, 55.0F, 30.0F, 1.2F, 0.85F, 0.3490659F, -0.08726647F, -0.08726647F, 50.0F,
        NULL, 0, 0.0F,
        REMIX_ID_JMARIO, 0, 0
    },

    // J Luigi
    {
        90, 55.0F, 30.0F, 0.0F, 0.85F, 0.4363323F, 0.0F, 0.0F, 36.0F,
        NULL, 0, 1.0F,
        REMIX_ID_JLUIGI, 0, 0
    },

    // Piano book; angle comes from wpRemixPianoBookMake, file is the Piano special1 file.
    {
        120, 60.0F, 25.0F, 1.4F, 0.55F, 0.139626F, 1.0472F, 1.0472F, 50.0F,
        NULL, 0, 0.0F,
        REMIX_ID_PIANO, 0, 0
    },

    // Dr. Luigi (capsule); palette is random in 3-5.
    {
        100, 55.0F, 30.0F, 0.0F, 0.85F, 0.4363323F, 0.0F, 0.0F, 36.0F,
        NULL, 0, 0.0F,
        REMIX_ID_DRL, 3, 3
    },
#endif
};

// 0x80188E90
WPDesc dWPMarioFireballWeaponDesc =
{
    0x00,                                   // Render flags?
    nWPKindFireball,                        // Weapon Kind
    &gFTMarioFileMain,                      // Pointer to character's loaded files?
#ifdef PORT
    llMarioMainFireballWeaponAttributes,   // Offset of weapon attributes in loaded files
#else
    &llMarioMainFireballWeaponAttributes,   // Offset of weapon attributes in loaded files
#endif

    // DObj transformation struct
    {
        nGCMatrixKindTra,                   // Main matrix transformations
        0x47,                               // Secondary matrix transformations?
        0                                   // ???
    },

    wpMarioFireballProcUpdate,              // Proc Update
    wpMarioFireballProcMap,                 // Proc Map
    wpMarioFireballProcHit,                 // Proc Hit
    wpMarioFireballProcHit,                 // Proc Shield
    wpMarioFireballProcHop,                 // Proc Hop
    wpMarioFireballProcHit,                 // Proc Set-Off
    wpMarioFireballProcReflector,           // Proc Reflector
    wpMarioFireballProcHit                  // Proc Absorb
};

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80168540
sb32 wpMarioFireballProcUpdate(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    DObj *dobj;

    if (wpMainDecLifeCheckExpire(wp) != FALSE)
    {
        efManagerDustExpandSmallMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f, 1.0F);

        return TRUE;
    }
    wpMainApplyGravityClampTVel(wp, dWPMarioFireballWeaponAttributes[wp->weapon_vars.fireball.index].gravity, dWPMarioFireballWeaponAttributes[wp->weapon_vars.fireball.index].vel_terminal);

    dobj = DObjGetStruct(weapon_gobj);

    dobj->rotate.vec.f.x += dWPMarioFireballWeaponAttributes[wp->weapon_vars.fireball.index].rotate_speed;

    return FALSE;
}

// 0x801685F0
sb32 wpMarioFireballProcMap(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    Vec3f pos;

    wpMapTestAll(weapon_gobj);

    if (wpMapCheckAllRebound(weapon_gobj, MAP_FLAG_MAIN_MASK, dWPMarioFireballWeaponAttributes[wp->weapon_vars.fireball.index].collide_rebound, &pos) != FALSE)
    {
#ifdef REGION_REMIX
        if (lbCommonMag2D(&wp->physics.vel_air) < wpRemixMarioFireballMinVel(wp, dWPMarioFireballWeaponAttributes[wp->weapon_vars.fireball.index].vel_min))
#else
        if (lbCommonMag2D(&wp->physics.vel_air) < dWPMarioFireballWeaponAttributes[wp->weapon_vars.fireball.index].vel_min)
#endif
        {
            efManagerDustExpandSmallMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f, 1.0F);
            return TRUE;
        }
        wpMainVelSetModelPitch(weapon_gobj);

#ifdef REGION_REMIX
        if (wpRemixMarioFireballBounceEffect(weapon_gobj) == FALSE)
        {
            efManagerFireGrindMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f);
        }
#else
        efManagerFireGrindMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f);
#endif
    }
    return FALSE;
}

// 0x801686C0
sb32 wpMarioFireballProcHit(GObj *weapon_gobj)
{
#ifdef REGION_REMIX
    if (wpRemixMarioFireballHitEffect(weapon_gobj) != FALSE)
    {
        return TRUE;
    }
#endif
    func_800269C0_275C0(nSYAudioFGMExplodeS);
    efManagerSparkleWhiteMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f);

    return TRUE;
}

// 0x801686F8
sb32 wpMarioFireballProcHop(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);

    syVectorRotateAbout3D(&wp->physics.vel_air, &wp->shield_collide_dir, wp->shield_collide_angle * 2);
    wpMainVelSetModelPitch(weapon_gobj);

    return FALSE;
}

// 0x80168748
sb32 wpMarioFireballProcReflector(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    FTStruct *fp = ftGetStruct(wp->owner_gobj);

    wp->lifetime = dWPMarioFireballWeaponAttributes[wp->weapon_vars.fireball.index].lifetime;

    wpMainReflectorSetLR(wp, fp);
    wpMainVelSetModelPitch(weapon_gobj);

    return FALSE;
}

// 0x801687A0
GObj* wpMarioFireballMakeWeapon(GObj *fighter_gobj, Vec3f *pos, s32 index)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 angle = (fp->ga == nMPKineticsAir) ? dWPMarioFireballWeaponAttributes[index].angle_air : dWPMarioFireballWeaponAttributes[index].angle_ground;

    return wpMarioFireballMakeWeaponAtAngle(fighter_gobj, pos, index, angle);
}

// Spawns the fireball at an explicit launch angle (Piano's book uses a random spread).
GObj* wpMarioFireballMakeWeaponAtAngle(GObj *fighter_gobj, Vec3f *pos, s32 index, f32 angle)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *weapon_gobj;
    WPStruct *wp;
    WPDesc weapon_desc = dWPMarioFireballWeaponDesc;

    weapon_desc.p_weapon = dWPMarioFireballWeaponAttributes[index].p_weapon;
#ifdef REGION_REMIX
    if (dWPMarioFireballWeaponAttributes[index].file_fkind != 0)
    {
        weapon_desc.p_weapon = wpRemixMarioFireballFile(dWPMarioFireballWeaponAttributes[index].file_fkind);
    }
#endif
    weapon_desc.o_attributes = dWPMarioFireballWeaponAttributes[index].offset;

    weapon_gobj = wpManagerMakeWeapon(fighter_gobj, &weapon_desc, pos, (WEAPON_FLAG_COLLPROJECT | WEAPON_FLAG_PARENT_FIGHTER));

    if (weapon_gobj == NULL)
    {
        return NULL;
    }
    wp = wpGetStruct(weapon_gobj);

    wp->weapon_vars.fireball.index = index;

    wp->lifetime = dWPMarioFireballWeaponAttributes[index].lifetime;

    wp->physics.vel_air.z = 0.0F;

    wp->physics.vel_air.x = dWPMarioFireballWeaponAttributes[index].vel_base * __cosf(angle) * fp->lr;
    wp->physics.vel_air.y = dWPMarioFireballWeaponAttributes[index].vel_base * __sinf(angle);

    DObjGetStruct(weapon_gobj)->mobj->palette_id = dWPMarioFireballWeaponAttributes[index].anim_frame;
#ifdef REGION_REMIX
    if (dWPMarioFireballWeaponAttributes[index].palette_range != 0)
    {
        DObjGetStruct(weapon_gobj)->mobj->palette_id = wpRemixMarioFireballPaletteId(fp, dWPMarioFireballWeaponAttributes[index].palette_base, dWPMarioFireballWeaponAttributes[index].palette_range);
    }
#endif

    wpMainVelSetModelPitch(weapon_gobj);

    return weapon_gobj;
}
