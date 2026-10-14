#ifndef _FTREMIXENTRY_H_
#define _FTREMIXENTRY_H_

/*
 * Remix entry effect structs (REGION_REMIX only).
 *
 * Remix patches the vanilla entry effect makers (efManager*Entry*MakeEffect) to build their EFDesc from
 * a per-fighter struct that points at one of the fighter's own files (Character.<NAME>_file_N_ptr) and
 * optionally moves the DObj/MObj/AnimJoint/MatAnimJoint offsets inside it. One row of
 * dFTRemixEntryOverrides (ftremixentrydata.c) is one such struct.
 */

#include <ft/fighter.h>

typedef enum FTRemixEntryEffect
{
    nFTRemixEntryEffectMarioPipe,
    nFTRemixEntryEffectFoxArwing,
    nFTRemixEntryEffectDonkeyBarrel,
    nFTRemixEntryEffectSamusPoint,
    nFTRemixEntryEffectLinkWave,
    nFTRemixEntryEffectLinkBeam,
    nFTRemixEntryEffectYoshiEgg,
    nFTRemixEntryEffectKirbyStar,
    nFTRemixEntryEffectCaptainCar,
    nFTRemixEntryEffectsNum

} FTRemixEntryEffect;

#define FTREMIX_ENTRY_SKIP (-1)       /* file_n: the fighter's patch builds no object for this effect */
#define FTREMIX_ENTRY_KEEP (-1)       /* offset: keep the base desc's value */
#define FTREMIX_ENTRY_SAME_BASE (-1)  /* base_effect: flags, transforms and procs come from the effect's own vanilla desc */

typedef struct FTRemixEntryOverride
{
    s32 fkind;
    FTRemixEntryEffect effect;      /* vanilla effect maker whose desc the Remix patch replaces */
    s32 file_n;                     /* Character.<NAME>_file_N_ptr the desc points at (1..9), or FTREMIX_ENTRY_SKIP */
    s32 base_effect;                /* effect whose vanilla desc supplies flags/transforms/procs, or FTREMIX_ENTRY_SAME_BASE */
    intptr_t o_dobjsetup;           /* each offset FTREMIX_ENTRY_KEEP = keep the base desc's value */
    intptr_t o_mobjsub;
    intptr_t o_anim_joint;
    intptr_t o_matanim_joint;

} FTRemixEntryOverride;

/* Captain-car animation: efManagerCaptainEntryCarMakeEffect reads its animation joints from hard-coded offsets
 * of the car file (clown_car_animation1/2 in captainshared.asm change them for Bowser and Conker). */
typedef struct FTRemixEntryCarAnim
{
    s32 fkind;
    s32 file_n;
    intptr_t o_car_anim;
    intptr_t o_wheel_anim_l;
    intptr_t o_wheel_anim_r;

} FTRemixEntryCarAnim;

/* Fox-Arwing animation: efManagerFoxEntryArwingMakeEffect reads the wing joint from the Special3 file and the body
 * joints from the Special2 file at hard-coded offsets (wolfen_entry_2/_3 in Wolf.asm change them for Wolf). */
typedef struct FTRemixEntryArwingAnim
{
    s32 fkind;
    s32 wing_file_n;
    intptr_t o_wing_anim;
    s32 body_file_n;
    intptr_t o_body_anim_r;
    intptr_t o_body_anim_l;

} FTRemixEntryArwingAnim;

#endif
