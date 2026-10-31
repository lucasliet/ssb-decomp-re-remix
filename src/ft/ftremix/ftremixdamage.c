/*
 * ftremixdamage.c - Smash Remix damage types 7..0xC (.toolchain/pkgs/smashremix-src/src/Damage.asm add_damage_type).
 * The on-hit GFX (extend_on_hit_gfx_, 0x800E3FC4) and on-hit colour animation (extend_on_hit_routines_, 0x80140BCC)
 * jump tables route these ids here; the vanilla ids 0..6 keep the vanilla switches.
 * The stun, bury and flashbang action changes (stun_damage_action_, stun_electric_action_) are in ftremixdamagestun.c
 * (ftRemixDamageGotoStunAction, ftRemixDamageUsesElectricStatus). prevent_wind_push_while_buried is in ftparam.c
 * (ftParamSetVelPush).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <ef/efmanager.h>
#include <gm/gmdef.h>
#include <lb/lbremixgfx.h>
#include "ft/ftremix/ftremixhooks.h"

/* asm: Damage.asm `constant SHADOW_GFX` is the dark cross, GFX id 0x6C */
#define FTREMIX_DAMAGE_SHADOW_GFX 0x6C
/* asm: GFXRoutine.id.SHADOW_1 (0x59), SHADOW_2..4 follow; the level is added to the base */
#define FTREMIX_DAMAGE_SHADOW_COLANIM_BASE 0x59
/* asm: GFXRoutine.id.LASER (0x6B), registration order in GFXRoutine.asm */
#define FTREMIX_DAMAGE_LASER_COLANIM 0x6B
/* asm: GFXRoutine.id.DEKU_STUN (0x6C) */
#define FTREMIX_DAMAGE_DEKU_STUN_COLANIM 0x6C

/**
 * TRUE for the damage types Remix adds (SHADOW..FLASHBANG). The vanilla ids 0..6 and the ids above 0xC are not new.
 */
sb32 ftRemixDamageIsNewType(s32 element)
{
    return (element >= FTREMIX_DAMAGE_SHADOW) && (element <= FTREMIX_DAMAGE_FLASHBANG);
}

/**
 * Starts the on-hit colour animation of a Remix damage type (asm: on_hit_routine table).
 * STUN and FLASHBANG use the electric animation, BURY the common one.
 */
sb32 ftRemixDamageSetColAnim(GObj *fighter_gobj, s32 element, s32 damage_level)
{
    switch (element)
    {
    case FTREMIX_DAMAGE_SHADOW:
        return ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_DAMAGE_SHADOW_COLANIM_BASE + damage_level, 0);

    case FTREMIX_DAMAGE_STUN:
    case FTREMIX_DAMAGE_FLASHBANG:
        return ftParamCheckSetSkeletonColAnimID(fighter_gobj, damage_level);

    case FTREMIX_DAMAGE_LASER:
        return ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_DAMAGE_LASER_COLANIM, 0);

    case FTREMIX_DAMAGE_DEKU_STUN:
        return ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_DAMAGE_DEKU_STUN_COLANIM, 0);

    default:
        return ftParamCheckSetFighterColAnimID(fighter_gobj, nGMColAnimFighterDamageCommon, 0);
    }
}

/**
 * On-hit GFX of the SHADOW damage type: the fire effect with the dark cross as the next GFX id
 * (asm: create_shadow_gfx_ sets GFX.current_gfx_id = 0x6C, then jumps to the fire effect).
 */
void ftRemixDamageMakeShadowEffect(Vec3f *pos, s32 damage)
{
    lbRemixGfxSetNextEffect(FTREMIX_DAMAGE_SHADOW_GFX);
    efManagerDamageFireMakeEffect(pos, damage);
}

#endif
