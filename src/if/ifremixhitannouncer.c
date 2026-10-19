/**
 * ifremixhitannouncer.c - Smash Remix hit and throw announcements (src/PokemonAnnouncer.asm hit_announcement_ and
 * throw_hit_announcement_). The knockback and damage buckets choose the STANDARD call of the announcer.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixhitannouncer.h"
#include "if/ifremixannouncer.h"
#include <gm/gmdef.h>
#include <sc/scene.h>
#include <sc/scmanager.h>

#define IFREMIX_HIT_KNOCKBACK_MID 115.0F
#define IFREMIX_HIT_KNOCKBACK_HIGH 155.0F
#define IFREMIX_THROW_KNOCKBACK_MID 120.0F
#define IFREMIX_THROW_KNOCKBACK_HIGH 175.0F
#define IFREMIX_DAMAGE_LITTLE_MAX 4
#define IFREMIX_DAMAGE_MIN 18
#define IFREMIX_HIT_DAMAGE_EARLY_TIME 0x360

/** Plays the high or the mid knockback call. The caller checked that the knockback reached the mid threshold. */
static void ifRemixHitAnnouncerPlayKnockback(f32 knockback, f32 high_threshold)
{
    if (knockback >= high_threshold)
    {
        ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallKnockbackHigh);
        return;
    }
    ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallKnockbackMid);
}

/**
 * Plays the damage bucket: nothing for no damage or below the minimum, the light call below the light maximum, otherwise
 * the early or the regular damage call.
 */
static void ifRemixHitAnnouncerPlayDamage(s32 damage, sb32 is_early)
{
    if (damage == 0)
    {
        return;
    }
    if (damage < IFREMIX_DAMAGE_LITTLE_MAX)
    {
        ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallLittleDamage);
        return;
    }
    if (damage < IFREMIX_DAMAGE_MIN)
    {
        return;
    }
    if (is_early)
    {
        ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallDamageEarly);
        return;
    }
    ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallDamage);
}

/** Hit announcement, see ifRemixHitAnnouncerPlayHit. */
void ifRemixHitAnnouncerPlayHit(FTStruct *victim_fp)
{
    s32 damage = victim_fp->damage_queue;
    sb32 is_early = gSCManagerBattleState->time_passed < IFREMIX_HIT_DAMAGE_EARLY_TIME;

    if (!ifRemixAnnouncerIsActive())
    {
        return;
    }
    if (victim_fp->damage_knockback >= IFREMIX_HIT_KNOCKBACK_MID)
    {
        ifRemixHitAnnouncerPlayKnockback(victim_fp->damage_knockback, IFREMIX_HIT_KNOCKBACK_HIGH);
        return;
    }
    if ((damage != 0) && (victim_fp->damage_element == nGMHitElementFire))
    {
        ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallBurn);
        return;
    }
    ifRemixHitAnnouncerPlayDamage(damage, is_early);
}

/** Throw announcement, see ifRemixHitAnnouncerPlayThrow. */
void ifRemixHitAnnouncerPlayThrow(s32 damage, f32 knockback)
{
    if (!ifRemixAnnouncerIsActive())
    {
        return;
    }
    if (knockback >= IFREMIX_THROW_KNOCKBACK_MID)
    {
        ifRemixHitAnnouncerPlayKnockback(knockback, IFREMIX_THROW_KNOCKBACK_HIGH);
        return;
    }
    ifRemixHitAnnouncerPlayDamage(damage, FALSE);
}

#endif
