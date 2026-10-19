/**
 * ifremixcombohits.c - Smash Remix combo meter hit state (src/ComboMeter.asm draw_hit_count_).
 *
 * Each frame the victim's meter decides what to show: the combo count while the victim is in a combo, the previous
 * count for the frame buffer after the combo, or nothing. The colour follows the port that attributed the last hit.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixcombometer.h"

extern void *func_800269C0_275C0(u16 id);

#define IFREMIX_COMBO_DEFAULT_FRAME_BUFFER 30
#define IFREMIX_COMBO_MAX_FRAME_BONUS 150
#define IFREMIX_COMBO_INCREDIBLE_HITS 20
#define IFREMIX_COMBO_FGM_INCREDIBLE 466
#define IFREMIX_COMBO_ATTRIBUTION_NONE (-1)

/** Keeps the highest combo of the attacker against the victim, the asm counts the hits since the attribution started. */
static void ifRemixComboHitsRecordHighest(IFRemixComboMeter *meters, s32 attacker, s32 victim, s32 hit_count)
{
    s32 attributed_hits = hit_count - meters[victim].attribution_start_hit + 1;

    if (meters[attacker].highest_combo_vs[victim] < attributed_hits)
    {
        meters[attacker].highest_combo_vs[victim] = attributed_hits;
    }
}

/** Raises the max hits and max damage of the meter when this combo beats them. */
static void ifRemixComboHitsRecordMax(IFRemixComboMeter *meter, s32 hit_count, s32 damage)
{
    if (meter->max_combo_hits < hit_count)
    {
        meter->max_combo_hits = hit_count;
    }
    if (meter->max_combo_damage < damage)
    {
        meter->max_combo_damage = damage;
    }
}

/** Frames the meter stays on screen after a hit, growing with the combo up to the cap. */
static void ifRemixComboHitsRefillFrames(IFRemixComboMeter *meter, s32 hit_count)
{
    s32 bonus_frames = hit_count * 4;

    if (bonus_frames >= IFREMIX_COMBO_MAX_FRAME_BONUS)
    {
        bonus_frames = IFREMIX_COMBO_MAX_FRAME_BONUS;
    }
    meter->frame_buffer = IFREMIX_COMBO_DEFAULT_FRAME_BUFFER + bonus_frames;
}

/**
 * Hit inside a combo: the colour follows the attacker of the hit. A hit with no attacker (or a silver one) keeps the
 * previous colour, and the highest combo is still counted for that previous attacker.
 */
static void ifRemixComboHitsUpdateInCombo(IFRemixComboMeter *meters, s32 victim, s32 hit_count, s32 damage)
{
    IFRemixComboMeter *meter = &meters[victim];
    s32 previous_color = meter->color_current;
    s32 attacker = meter->fp->damage_player;

    ifRemixComboHitsRefillFrames(meter, hit_count);
    /* REMIX-TODO(PokemonAnnouncer): pokemon_combo_ call of draw_hit_count_ is skipped. */

    if ((hit_count == IFREMIX_COMBO_INCREDIBLE_HITS) && (meter->combo_count != hit_count))
    {
        func_800269C0_275C0(IFREMIX_COMBO_FGM_INCREDIBLE);
    }
    ifRemixComboHitsRecordMax(meter, hit_count, damage);
    meter->combo_count = hit_count;

    if ((attacker == IFREMIX_COMBO_ATTRIBUTION_NONE) || (attacker >= IFREMIX_COMBO_COLOR_SILVER))
    {
        meter->color_display = previous_color;
        if (previous_color < IFREMIX_COMBO_COLOR_SILVER)
        {
            ifRemixComboHitsRecordHighest(meters, previous_color, victim, hit_count);
        }
        return;
    }
    meter->color_current = attacker;
    meter->color_display = attacker;
    if (attacker != previous_color)
    {
        meter->attribution_start_hit = hit_count;
    }
    ifRemixComboHitsRecordHighest(meters, attacker, victim, hit_count);
}

/** Outside of a combo: a fresh first hit re-syncs the colour from the attacker, then the frame buffer decides. */
static s32 ifRemixComboHitsUpdateOutOfCombo(IFRemixComboMeter *meter, s32 hit_count)
{
    s32 attacker = meter->fp->damage_player;

    if ((hit_count != 0) && (meter->attribution_start_hit != hit_count))
    {
        meter->color_current = (attacker == IFREMIX_COMBO_ATTRIBUTION_NONE) ? IFREMIX_COMBO_COLOR_SILVER : attacker;
    }
    meter->attribution_start_hit = hit_count;

    if (meter->frame_buffer == 0)
    {
        return IFREMIX_COMBO_ATTRIBUTION_NONE;
    }
    meter->frame_buffer--;
    return meter->combo_count;
}

/**
 * Hit state of the victim's meter for one frame, called only for a meter that has a player struct.
 * hit_count and damage are the victim's combo_count_foe and combo_damage_foe.
 */
s32 ifRemixComboHitsUpdate(IFRemixComboMeter *meters, s32 victim, s32 hit_count, s32 damage)
{
    if (hit_count > 1)
    {
        ifRemixComboHitsUpdateInCombo(meters, victim, hit_count, damage);
        return hit_count;
    }
    return ifRemixComboHitsUpdateOutOfCombo(&meters[victim], hit_count);
}

#endif
