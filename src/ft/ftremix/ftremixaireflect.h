#ifndef _FTREMIXAIREFLECT_H_
#define _FTREMIXAIREFLECT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <wp/weapon.h>

/* CPU reflect and absorb hooks of the Remix AI (Reflect.asm scope AI), used by ftcomputer.c. */

/** Prediction window, in frames, of the projectile hazard check of fp (set_projectile_detection_radius_). */
f32 ftRemixAiReflectDetectionFrames(FTStruct *fp);

/** TRUE when fp raises its reflect/absorb flag for the projectile wp (extend_projectile_reflect_initial_). */
sb32 ftRemixAiReflectProjectileRaisesFlag(FTStruct *fp, WPStruct *wp);

/** TRUE when fp raises its reflect/absorb flag for an item hazard (extend_item_reflect_initial_, Character.fighter_reflect). */
sb32 ftRemixAiReflectItemRaisesFlag(FTStruct *fp);

/**
 * maintain_reflect_input_: while a Remix reflector or absorber is inside its special scope, releases B (the vanilla Fox and Ness
 * behaviour). Returns TRUE when the command was set and the caller must return FALSE.
 */
sb32 ftRemixAiReflectMaintainInput(FTStruct *fp);

/** apply_reflect_input_: a Remix reflector or absorber outside its special scope presses its down special. */
void ftRemixAiReflectApplyInput(FTStruct *fp);

/**
 * extend_reflect_absorb_character_check_: TRUE when the picker drops the NSP candidate of fp against target_fp, a reflector
 * or absorber (Character.fighter_reflect) while fp's Character.ai_aerial_chase entry is CHASE. target_fp may be a stale
 * pointer to a non-fighter target; its kind is range checked.
 */
sb32 ftRemixAiReflectSkipsNspCandidate(FTStruct *fp, FTStruct *target_fp);

#endif

#endif
