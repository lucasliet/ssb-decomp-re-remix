#ifndef _FTREMIXAIREFLECTRULES_H_
#define _FTREMIXAIREFLECTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix CPU reflect and absorb hooks (.toolchain/pkgs/smashremix-src/src/Reflect.asm, scope AI), so the
 * headless unit tests (tests/remix) can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_REFLECT_FRAMES_DEFAULT 15.0F /* vanilla projectile prediction window, in frames */
#define FTREMIX_AI_REFLECT_FRAMES_MARINA 25.0F  /* set_projectile_detection_radius_: her absorb starts slowly */

/* Projectile.asm ids of the projectiles a CPU absorber ignores (extend_projectile_reflect_initial_, _ness_absorb). */
#define FTREMIX_AI_REFLECT_PROJECTILE_SHEIK_NEEDLE 0x23
#define FTREMIX_AI_REFLECT_PROJECTILE_CONKER_NUT 0x24

/* Marina.asm Action.DSPG_Begin to DSPA_Pull_Fail: her absorb (Clanpot) actions, the upper bound is exclusive. */
#define FTREMIX_AI_REFLECT_MARINA_ABSORB_FIRST 0x0FE
#define FTREMIX_AI_REFLECT_MARINA_ABSORB_END 0x10A

/* Piano absorb actions, the upper bound is exclusive (maintain_reflect_input_, _piano_absorb). */
#define FTREMIX_AI_REFLECT_PIANO_ABSORB_FIRST 0x0E5
#define FTREMIX_AI_REFLECT_PIANO_ABSORB_END 0x0EE

/* Marina's Clanpot is not used while she holds an item or her charge is at the maximum. */
#define FTREMIX_AI_REFLECT_MARINA_CHARGE_MAX 4

/* Which special-move scope a CPU fighter holds while it reflects or absorbs. */
#define FTREMIX_AI_REFLECT_KIND_NONE 0
#define FTREMIX_AI_REFLECT_KIND_FOX 1   /* Fox down special scope (reflector) */
#define FTREMIX_AI_REFLECT_KIND_NESS 2  /* Ness down special scope (PSI Magnet) */
#define FTREMIX_AI_REFLECT_KIND_PIANO 3 /* Piano absorb actions */
#define FTREMIX_AI_REFLECT_KIND_MARINA 4 /* Marina Clanpot actions */

/** Frames of the projectile prediction window of the CPU fighter fkind (set_projectile_detection_radius_). */
float ftRemixAiReflectRulesDetectionFrames(int fkind);

/**
 * TRUE when the CPU fighter fkind raises its reflect/absorb flag (is_opponent_ra) for a projectile of kind projectile_kind
 * (extend_projectile_reflect_initial_). Vanilla Fox, N Fox and Ness always do; the Remix reflectors follow the asm chain.
 * marina_cannot_absorb is TRUE when Marina holds an item or has her charge at the maximum.
 */
int ftRemixAiReflectRulesProjectileRaisesFlag(int fkind, int projectile_kind, int marina_cannot_absorb);

/**
 * TRUE when the CPU fighter fkind raises its flag for an item hazard (extend_item_reflect_initial_). is_table_reflector is
 * the Character.fighter_reflect entry of fkind; vanilla Fox, N Fox and Ness bypass the table in the asm.
 */
int ftRemixAiReflectRulesItemRaisesFlag(int fkind, int is_table_reflector, int marina_cannot_absorb);

/** Scope held by a Remix fighter while it keeps its reflect/absorb input (maintain_reflect_input_), KIND_NONE otherwise. */
int ftRemixAiReflectRulesMaintainKind(int fkind);

/** Scope used to start the reflect/absorb input (apply_reflect_input_); NLucas is missing there (the asm repeats LUCAS). */
int ftRemixAiReflectRulesApplyKind(int fkind);

/** TRUE when a Piano or Marina action id is inside her absorb actions; other kinds are decided by the caller. */
int ftRemixAiReflectRulesIsAbsorbAction(int kind, int action);

/**
 * extend_reflect_absorb_character_check_ (see docs/remix/ai.md): TRUE when the picker skips the NSP candidate of a CPU whose
 * ai_aerial_chase entry is CHASE because the target is a reflector or absorber.
 */
int ftRemixAiReflectRulesSkipsNspCandidate(int is_target_reflector, int is_own_chase);

#endif

#endif
