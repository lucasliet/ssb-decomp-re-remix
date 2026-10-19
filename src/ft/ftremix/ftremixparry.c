/*
 * ftremixparry.c - Parrying (BETA) setting of Smash Remix (.toolchain/pkgs/smashremix-src/src/Parry.asm, toggle entry_parry).
 * ftMainSearchHitFighter, ftMainSearchHitWeapon and ftMainSearchHitItem call the parry check before their shield check.
 * A parry starts the ClangRecoil action on the defender, makes it intangible and skips the hit. ftMainProcParams adds
 * the extra hitlag of a parried direct hit to the attacker. Every entry point is a no-op while the setting is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftcommon.h>
#include <ef/efmanager.h>
#include <gm/gmcollision.h>
#include <gm/gmdef.h>
#include <gm/gmremixtoggle.h>
#include <it/itprocess.h>
#include <it/itmain.h>
#include <wp/wpmain.h>
#include <wp/wpprocess.h>
#include "ft/ftremix/ftremixhooks.h"

extern void *func_800269C0_275C0(u16 id);

#define FTREMIX_PARRY_SFX 0x5BB                 /* Parry.asm PARRY_SFX */
#define FTREMIX_PARRY_ACTIVATION_FRAMES 5       /* Parry.asm PARRY_ACTIVATION_FRAMES */
#define FTREMIX_PARRY_GFX_ROUTINE 0x7D          /* GFXRoutine.asm PARRY: 0x55 + 40, its position in the add list */
#define FTREMIX_PARRY_SIZE_MUL 1.5F             /* Parry.asm check_parry_collision SIZE_MULTI (0x3FC0 << 16) */
#define FTREMIX_PARRY_SPARK_SIZE 20.0F          /* Parry.asm activate_parry, scaled by the X scale of the top joint */
#define FTREMIX_PARRY_SHOCKWAVE_EFFECT 0x16     /* Parry.asm activate_parry ground bump */
#define FTREMIX_PARRY_DIRECT_HITLAG_BONUS 11    /* Parry.asm direct hit: defender hitlag = attacker hitlag + 11 */
#define FTREMIX_PARRY_ATTACKER_HITLAG_BONUS 14  /* Parry.asm apply_attacker_extra_hitlag */
#define FTREMIX_PARRY_HITLAG_MUL 1.0F           /* Parry.asm gmCommon_DamageCalcHitLag argument (0x3F80 << 16) */

enum
{
    nFTRemixParryActionClangRecoil = 0x053,
    nFTRemixParryActionShieldOn = 0x098,
    nFTRemixParryActionShield = 0x099,
    nFTRemixParryActionShieldOff = 0x09A,
};

/* Parry.asm is_hit_parry: per attacker port, set by a parried direct hit, cleared by apply_attacker_extra_hitlag */
static u8 sFTRemixParryIsHitParry[GMCOMMON_PLAYERS_MAX];

/* Parry.asm _check_action: the shield states a parry can start from */
static sb32 ftRemixParryIsShieldAction(s32 status_id)
{
    return (status_id == nFTRemixParryActionShieldOff) || (status_id == nFTRemixParryActionShieldOn) ||
        (status_id == nFTRemixParryActionShield);
}

/* Parry.asm _check_action to _action_passed: grounded, in a shield state, within the activation frames, shield released */
static sb32 ftRemixParryIsWindowOpen(FTStruct *fp)
{
    if (fp->ga != FALSE)
    {
        return FALSE;
    }
    if (!ftRemixParryIsShieldAction(fp->status_id))
    {
        return FALSE;
    }
    if (fp->status_total_tics > FTREMIX_PARRY_ACTIVATION_FRAMES)
    {
        return FALSE;
    }
    return fp->status_vars.common.guard.is_release != FALSE;
}

/* Parry.asm: the setting is on and the defender is inside the parry window */
static sb32 ftRemixParryCanStart(FTStruct *fp)
{
    if (!gmRemixToggle(parry))
    {
        return FALSE;
    }
    return ftRemixParryIsWindowOpen(fp);
}

/* Parry.asm check_parry_collision: the shield sphere of the defender, with the size multiplied by 1.5F */
static sb32 ftRemixParryCollides(FTStruct *fp, Vec3f *pos_curr, Vec3f *pos_prev, f32 size, s32 attack_state)
{
    DObj *topjoint = fp->joints[nFTPartsJointTopN];
    FTParts *parts;
    Vec3f sphere_offset;
    Vec3f sphere_size;
    f32 angle;
    f32 shield_size = fp->attr->shield_size * FTREMIX_PARRY_SIZE_MUL;

    func_ovl2_800EDE00(topjoint);
    func_ovl2_800EDE5C(topjoint);
    parts = ftGetParts(topjoint);

    sphere_offset.x = 0.0F;
    sphere_offset.y = fp->attr->map_coll.center;
    sphere_offset.z = 0.0F;

    sphere_size.x = shield_size;
    sphere_size.y = shield_size;
    sphere_size.z = shield_size;

    return gmCollisionTestSphere(pos_curr, pos_prev, size, attack_state, parts->unk_dobjtrans_0x9C, &sphere_offset,
        &sphere_size, &parts->vec_scale, 0, &angle, NULL);
}

/* Parry.asm activate_parry, part 1: ClangRecoil action, intangible hurtbox, parry GFX routine, cleared hit values */
static void ftRemixParryStart(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixParryActionClangRecoil, 1.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    fp->hitstatus = nGMHitStatusIntangible;
    ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_PARRY_GFX_ROUTINE, 0);

    fp->status_vars.common.rebound.rebound_timer = 0.0F;
    fp->hitlag_tics = 0;
    fp->shield_damage = 0;
    fp->shield_damage_total = 0;
}

/* Parry.asm activate_parry, white spark at the centre of the defender, scaled by the top joint */
static void ftRemixParryMakeSpark(FTStruct *fp)
{
    Vec3f pos;
    DObj *topjoint = fp->joints[nFTPartsJointTopN];

    pos.x = fp->coll_data.p_translate->x;
    pos.y = fp->coll_data.p_translate->y + fp->attr->map_coll.center;
    pos.z = fp->coll_data.p_translate->z;

    efManagerSetOffMakeEffect(&pos, (s32) (topjoint->scale.vec.f.x * FTREMIX_PARRY_SPARK_SIZE));
}

/* Parry.asm activate_parry, parry sound, spark and ground bump */
static void ftRemixParryActivate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixParryStart(fighter_gobj);
    func_800269C0_275C0(FTREMIX_PARRY_SFX);
    ftRemixParryMakeSpark(fp);
    ftParamMakeEffect(fighter_gobj, FTREMIX_PARRY_SHOCKWAVE_EFFECT, 0, NULL, NULL, fp->lr, FALSE, 0);
}

/* Parry.asm _calculate_hitlag of the weapon and item hits: hitlag of the damage, minus one tic when it is not zero */
static s32 ftRemixParryDamageHitlag(FTStruct *fp, s32 damage)
{
    s32 hitlag = ftParamGetHitLag(damage, fp->status_id, FTREMIX_PARRY_HITLAG_MUL);

    if (hitlag != 0)
    {
        hitlag -= 1;
    }
    return hitlag;
}

/* Parry.asm direct hit: the first enabled hitbox of the attacker that reaches the parry sphere */
static FTAttackColl* ftRemixParryFindFighterHit(FTStruct *fp, FTStruct *attacker_fp)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(attacker_fp->attack_colls); i++)
    {
        FTAttackColl *attack_coll = &attacker_fp->attack_colls[i];

        if (attack_coll->attack_state == nGMAttackStateOff)
        {
            continue;
        }
        if (ftRemixParryCollides(fp, &attack_coll->pos_curr, &attack_coll->pos_prev, attack_coll->size, attack_coll->attack_state))
        {
            return attack_coll;
        }
    }
    return NULL;
}

/* Parry.asm direct hit, registration: the attacker gets the shield hit and its damage, the defender gets its hitlag + 11 */
static void ftRemixParryApplyFighterHit(FTStruct *fp, FTStruct *attacker_fp, FTAttackColl *attack_coll)
{
    s32 damage = attack_coll->damage + attack_coll->shield_damage;

    ftMainSetHitInteractStats(attacker_fp, attack_coll->group_id, fp->fighter_gobj, nGMHitTypeShield, 0, 0);
    sFTRemixParryIsHitParry[attacker_fp->player]++;

    attacker_fp->attack_damage = damage;
    attacker_fp->hitlag_tics = ftParamGetHitLag(damage, fp->status_id, FTREMIX_PARRY_HITLAG_MUL);
    fp->hitlag_tics = attacker_fp->hitlag_tics + FTREMIX_PARRY_DIRECT_HITLAG_BONUS;
}

/* Parry.asm direct_hit_parry_bubble_check (0x800E4D38): TRUE when the hit was parried, the caller then skips its shield check */
sb32 ftRemixParryDirectHit(FTStruct *victim_fp, FTStruct *attacker_fp)
{
    FTAttackColl *attack_coll;

    if (!ftRemixParryCanStart(victim_fp))
    {
        return FALSE;
    }
    attack_coll = ftRemixParryFindFighterHit(victim_fp, attacker_fp);
    if (attack_coll == NULL)
    {
        return FALSE;
    }
    ftRemixParryActivate(victim_fp->fighter_gobj);
    ftRemixParryApplyFighterHit(victim_fp, attacker_fp, attack_coll);
    return TRUE;
}

/* Parry.asm projectile_hit_parry_bubble_check (0x800E52C0): a hitbox outside the fighter range (is_damage_detect) counts as a hit */
static sb32 ftRemixParryWeaponHasHit(FTStruct *fp, WPStruct *wp, sb32 *is_damage_detect)
{
    WPAttackColl *wp_attack_coll = &wp->attack_coll;
    s32 i;

    for (i = 0; i < wp_attack_coll->attack_count; i++)
    {
        if (is_damage_detect[i] == FALSE)
        {
            return TRUE;
        }
        if (ftRemixParryCollides(fp, &wp_attack_coll->attack_pos[i].pos_curr, &wp_attack_coll->attack_pos[i].pos_prev, wp_attack_coll->size, wp_attack_coll->attack_state))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Parry.asm projectile hit registration: the weapon is stale-damaged and the parrier gets the hitlag of that damage */
static void ftRemixParryApplyWeaponHit(FTStruct *fp, WPStruct *wp)
{
    s32 damage;

    wpProcessUpdateHitInteractStats(wp, &wp->attack_coll, fp->fighter_gobj, nGMHitTypeShieldRehit, 0);
    damage = wpMainGetStaledDamage(wp);
    wp->hit_normal_damage = damage;
    fp->hitlag_tics = ftRemixParryDamageHitlag(fp, damage);
}

/* Parry.asm projectile_hit_parry_bubble_check (0x800E52C0): TRUE when the weapon hit was parried, the caller skips the fighter */
sb32 ftRemixParryWeaponHit(FTStruct *fp, WPStruct *wp, sb32 *is_damage_detect)
{
    if (!ftRemixParryCanStart(fp))
    {
        return FALSE;
    }
    if (!ftRemixParryWeaponHasHit(fp, wp, is_damage_detect))
    {
        return FALSE;
    }
    ftRemixParryActivate(fp->fighter_gobj);
    ftRemixParryApplyWeaponHit(fp, wp);
    return TRUE;
}

/* Parry.asm item loop (lines 514-521): the previous position of an interpolating hitbox (state 3), else the current one */
static Vec3f* ftRemixParryItemPrevPos(ITAttackColl *it_attack_coll, s32 i)
{
    if (it_attack_coll->attack_state > 2)
    {
        return &it_attack_coll->attack_pos[i].pos_prev;
    }
    return &it_attack_coll->attack_pos[i].pos_curr;
}

/* Parry.asm item_hit_parry_bubble_check (0x800E59A0): a hitbox outside the fighter range (is_damage_detect) counts as a hit */
static sb32 ftRemixParryItemHasHit(FTStruct *fp, ITStruct *ip, sb32 *is_damage_detect)
{
    ITAttackColl *it_attack_coll = &ip->attack_coll;
    s32 i;

    for (i = 0; i < it_attack_coll->attack_count; i++)
    {
        if (is_damage_detect[i] == FALSE)
        {
            return TRUE;
        }
        if (ftRemixParryCollides(fp, &it_attack_coll->attack_pos[i].pos_curr, ftRemixParryItemPrevPos(it_attack_coll, i), it_attack_coll->size, it_attack_coll->attack_state))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Parry.asm item hit registration: the item is damaged and the parrier gets the hitlag of that damage */
static void ftRemixParryApplyItemHit(FTStruct *fp, ITStruct *ip)
{
    s32 damage;

    itProcessSetHitInteractStats(&ip->attack_coll, fp->fighter_gobj, nGMHitTypeShieldRehit, 0);
    damage = itMainGetDamageOutput(ip);
    ip->hit_normal_damage = damage;
    fp->hitlag_tics = ftRemixParryDamageHitlag(fp, damage);
}

/* Parry.asm item_hit_parry_bubble_check (0x800E59A0): TRUE when the item hit was parried, the caller skips the fighter */
sb32 ftRemixParryItemHit(FTStruct *fp, ITStruct *ip, sb32 *is_damage_detect)
{
    if (!ftRemixParryCanStart(fp))
    {
        return FALSE;
    }
    if (!ftRemixParryItemHasHit(fp, ip, is_damage_detect))
    {
        return FALSE;
    }
    ftRemixParryActivate(fp->fighter_gobj);
    ftRemixParryApplyItemHit(fp, ip);
    return TRUE;
}

/* Parry.asm apply_attacker_extra_hitlag (0x800E6690): a parried hit adds 14 tics to the attacker, once */
void ftRemixParryAddAttackerHitlag(FTStruct *fp)
{
    if (!gmRemixToggle(parry))
    {
        return;
    }
    if (sFTRemixParryIsHitParry[fp->player] == 0)
    {
        return;
    }
    fp->hitlag_tics += FTREMIX_PARRY_ATTACKER_HITLAG_BONUS;
    sFTRemixParryIsHitParry[fp->player] = 0;
}

#endif
