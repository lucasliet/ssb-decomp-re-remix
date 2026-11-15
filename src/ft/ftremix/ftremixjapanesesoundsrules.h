#ifndef _FTREMIXJAPANESESOUNDSRULES_H_
#define _FTREMIXJAPANESESOUNDSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Japanese Sounds toggle (Japan.asm japanese_sfx_*, Character.asm apply_sound_type_): which fighters
 * use the J sounds and the J hit sound table of fighter hitboxes. Only int, so the headless unit tests (tests/remix)
 * can run them.
 */

/** Values of the japanese_sounds toggle. */
#define FTREMIXJSFX_TOGGLE_DEFAULT 0
#define FTREMIXJSFX_TOGGLE_ALWAYS 1
#define FTREMIXJSFX_TOGGLE_NEVER 2

/**
 * Whether a fighter uses the J sounds: ALWAYS for every fighter, NEVER for none, any other value only for the fighters
 * of sound type J (Character.asm sound_type).
 *
 * @param toggle_value Value of the japanese_sounds toggle.
 * @param is_j_fighter 1 when the fighter has sound type J.
 * @return 1 when the J sounds are used.
 */
extern int ftRemixJapaneseSoundsUseJ(int toggle_value, int is_j_fighter);

/**
 * Hit sound id of a fighter hitbox (Character.asm apply_sound_type_, 0x800E2CA0 in ftMainPlayHitSFX). The J table
 * sound_type_J is the vanilla table (0x80128D00) with the Punch and Kick rows replaced by the J ids; every other row,
 * the Bat row's PunchM/PunchL included, keeps the vanilla id.
 *
 * @param use_j 1 when the attacker uses the J sounds.
 * @param fgm_kind Row of the table (GMAttackSound).
 * @param fgm_level Column of the table (GMAttackLevel).
 * @param fgm_id Vanilla id dFTMainHitCollisionFGMs[fgm_kind][fgm_level].
 * @return The id to play.
 */
extern int ftRemixJapaneseHitCollisionFgmRule(int use_j, int fgm_kind, int fgm_level, int fgm_id);

#endif

#endif
