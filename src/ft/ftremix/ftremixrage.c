/**
 * ftremixrage.c - Smash Remix Rage.asm: the Rage setting. The knockback of a hit grows with the damage of its attacker,
 * with a multiplier that depends on the mode, and a fighter above 100% damage gives off smoke.
 *
 * ftMainProcessHitCollisionStatsMain, ftCommonThrownReleaseThrownUpdateStats, ftParamGetCommonKnockback and
 * ftMainProcPhysicsMap call into this file (see docs/remix/toggles.md). Every entry point is a no-op while the
 * setting is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <wp/weapon.h>

/* Global.frame_counter (0x8003CB30) is the audio tic counter dSYAudioCurrentTic of sys/audio.c. */
extern s32 dSYAudioCurrentTic;

/* Rage.asm toggle values, the titles of string_table_rage in Toggles.asm. */
#define FTREMIX_RAGE_OFF 0
#define FTREMIX_RAGE_ULT 1
#define FTREMIX_RAGE_FOUR 2
#define FTREMIX_RAGE_MANIC 3
#define FTREMIX_RAGE_DEPRESS 4

/* Rage.asm knockback_multiply: the attacker damage is clamped to [35, 150] and the scale is (damage - 35) / 115. */
#define FTREMIX_RAGE_DAMAGE_MIN 35
#define FTREMIX_RAGE_DAMAGE_MAX 150
#define FTREMIX_RAGE_DAMAGE_SPAN 115.0F

/* Rage.asm gen_gfx: smoke starts at 100% damage, and its spawn frame mask gets stricter at 125% and 150%. */
#define FTREMIX_RAGE_SMOKE_MIN_DAMAGE 100
#define FTREMIX_RAGE_SMOKE_MID_DAMAGE 125
#define FTREMIX_RAGE_SMOKE_HIGH_DAMAGE 150
#define FTREMIX_RAGE_SMOKE_EFFECT 7

/* Object id of a fighter gobj, the same check as the Remix player owner test in Rage.asm (0x3E8). */
#define FTREMIX_RAGE_FIGHTER_GOBJ_ID 0x3E8

/* Rage.asm attacker_damage_percent: the attacker damage of the hit being resolved, read by the knockback formula. */
static s32 sFTRemixRageAttackerPercent;

/* Rage.asm knockback_multiply: multiplier of the scaled damage, as float literals of the asm (0x3DCD, 0x3E1A, 0x3F00,
 * 0xBF00 in the high half). */
static f32 ftRemixRageMultiplier(s32 rage_mode)
{
    if (rage_mode == FTREMIX_RAGE_ULT)
    {
        return 0.10009765625F;
    }
    if (rage_mode == FTREMIX_RAGE_FOUR)
    {
        return 0.150390625F;
    }
    if (rage_mode == FTREMIX_RAGE_MANIC)
    {
        return 0.5F;
    }
    return -0.5F;
}

/* Rage.asm knockback_multiply: the attacker damage clamped to [35, 150]. */
static s32 ftRemixRageClampDamage(s32 damage)
{
    if (damage < FTREMIX_RAGE_DAMAGE_MIN)
    {
        return FTREMIX_RAGE_DAMAGE_MIN;
    }
    if (damage > FTREMIX_RAGE_DAMAGE_MAX)
    {
        return FTREMIX_RAGE_DAMAGE_MAX;
    }
    return damage;
}

/* Rage.asm gen_gfx: the frame mask of the smoke spawn (0x3F, 0x1F or 0x0F), the smoke comes faster as damage grows. */
static s32 ftRemixRageSmokeFrameMask(s32 damage)
{
    if (damage < FTREMIX_RAGE_SMOKE_MID_DAMAGE)
    {
        return 0x3F;
    }
    if (damage < FTREMIX_RAGE_SMOKE_HIGH_DAMAGE)
    {
        return 0x1F;
    }
    return 0x0F;
}

/* Rage.asm direct_attack (0x800E3F70) and throw (0x8014B060): the attacker damage percent that the knockback of this
 * hit reads. */
void ftRemixRageSaveAttackerPercent(FTStruct *attacker_fp)
{
    if (gmRemixToggle(rage) == FTREMIX_RAGE_OFF)
    {
        return;
    }
    sFTRemixRageAttackerPercent = attacker_fp->percent_damage;
}

/* Rage.asm projectile_attack (0x800E40FC): a weapon hit takes the damage percent of its owner when the owner is a
 * fighter. */
void ftRemixRageSaveProjectileOwnerPercent(WPStruct *wp)
{
    GObj *owner_gobj = wp->owner_gobj;

    if (owner_gobj == NULL)
    {
        return;
    }
    if (owner_gobj->id != FTREMIX_RAGE_FIGHTER_GOBJ_ID)
    {
        return;
    }
    ftRemixRageSaveAttackerPercent(ftGetStruct(owner_gobj));
}

/* Rage.asm knockback_multiply (0x800E9F60): scales the knockback before the 2500 limit. Ultimate skips the knockback
 * of a move with knockback_weight set (the asm branches to its end when a3 is not 0), and the attacker damage is reset
 * once it has been used. */
f32 ftRemixRageKnockbackScale(f32 knockback, s32 knockback_weight)
{
    s32 rage_mode = gmRemixToggle(rage);
    s32 damage;
    f32 multiplier;

    if (rage_mode == FTREMIX_RAGE_OFF)
    {
        return knockback;
    }
    if ((rage_mode == FTREMIX_RAGE_ULT) && (knockback_weight != 0))
    {
        return knockback;
    }

    damage = ftRemixRageClampDamage(sFTRemixRageAttackerPercent);
    multiplier = ftRemixRageMultiplier(rage_mode);
    sFTRemixRageAttackerPercent = 0;

    return knockback * (1.0F + ((f32) (damage - FTREMIX_RAGE_DAMAGE_MIN) / FTREMIX_RAGE_DAMAGE_SPAN) * multiplier);
}

/* Rage.asm gen_gfx (0x800E23D0): the smoke of a fighter at 100% damage or more, spawned on the frames of its mask. */
void ftRemixRageSmokeEffect(FTStruct *fp)
{
    s32 damage = fp->percent_damage;

    if (gmRemixToggle(rage) == FTREMIX_RAGE_OFF)
    {
        return;
    }
    if (damage < FTREMIX_RAGE_SMOKE_MIN_DAMAGE)
    {
        return;
    }
    if ((dSYAudioCurrentTic & ftRemixRageSmokeFrameMask(damage)) != 0)
    {
        return;
    }
    ftParamMakeEffect(fp->fighter_gobj, FTREMIX_RAGE_SMOKE_EFFECT, 0, NULL, NULL, fp->lr, FALSE, FALSE);
}

#endif
