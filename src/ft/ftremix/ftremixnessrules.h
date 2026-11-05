#ifndef _FTREMIXNESSRULES_H_
#define _FTREMIXNESSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Ness family patches (src/nessshared.asm, src/Lucas/Lucas.asm) so the headless unit tests
 * (tests/remix) can run them without game state: J Ness and Lucas PK Fire and PK Thunder values, the Lucas PSI Magnet and
 * PK Thunder multi-hit rules. The engine side is ftremixnessshared.c and the lucas directory.
 *
 * "Power kind" is the fighter kind whose moves are used: the fighter itself, or the copied power of a Kirby.
 */

/* pkfire_ground_speed_1_/_2_, pkfire_air_speed_1_/_2_: 0x42B4 and 0x42F0 */
#define FTREMIX_NESS_PKFIRE_SPEED_GROUND 90.0F
#define FTREMIX_NESS_PKFIRE_SPEED_AIR 120.0F

/* up_special_landing_fsm_: LANDING_FSM_JNESS 0x3E75C28F, LANDING_FSM_LUCAS 0x3E99999A, AIR_SPEED_MULTIPLIER_LUCAS
 * 0x3F52 << 16 */
#define FTREMIX_NESS_PKTHUNDER_LANDING_LAG_JNESS 0.24F
#define FTREMIX_NESS_PKTHUNDER_LANDING_LAG_LUCAS 0.3F
#define FTREMIX_NESS_PKTHUNDER_DRIFT_LUCAS 0.8203125F

/* up_special_ending_delay_: ENDING_DELAY_JNESS */
#define FTREMIX_NESS_PKTHUNDER_END_DELAY_JNESS 20

/* pkfire_pillar_gravity_: J_ITPKFIRE_GRAVITY 0x3ECCCCCD, J_ITPKFIRE_TVEL 0x4248 */
#define FTREMIX_NESS_PKFIRE_PILLAR_GRAVITY_JNESS 0.4F
#define FTREMIX_NESS_PKFIRE_PILLAR_TVEL_JNESS 50.0F

/* psi_magnet_multiplier_: the original line doubles the absorbed damage, Lucas multiplies by 0x4020 << 16 */
#define FTREMIX_NESS_PSIMAGNET_HEAL_MULTIPLIER 2.0F
#define FTREMIX_NESS_PSIMAGNET_HEAL_MULTIPLIER_LUCAS 2.5F

/* usp_multi_hit_: HIT_RATE, frames between hitbox refreshes */
#define FTREMIX_LUCAS_PKTHUNDER_HIT_RATE 8

/* True for J Ness and Lucas, the fighters whose PK Fire and PK Thunder weapons read their own loaded files. */
int ftRemixNessRulesUsesOwnFiles(int power_kind);

/* pkfire_horizontal: Lucas takes the grounded launch even in the air. */
int ftRemixNessRulesPKFireIsAir(int power_kind, int is_air);

/* pkfire_*_speed_*_: J Ness and Lucas launch faster than Ness. */
float ftRemixNessRulesPKFireSpeed(int power_kind, int is_air, float vanilla_speed);

/* pkfire_explosion: a Lucas PK Fire hit shows an explosion instead of leaving a flame pillar. */
int ftRemixNessRulesPKFireExplodes(int power_kind);

/* get_pkfire_collision_pointer_: J Ness's pillar reads the J Ness file, Ness keeps the vanilla one. */
int ftRemixNessRulesPKFirePillarUsesOwnFile(int power_kind);

/* pkfire_pillar_gravity_: only J Ness has the Japanese pillar gravity and terminal velocity. */
float ftRemixNessRulesPKFirePillarGravity(int power_kind, float vanilla_gravity);
float ftRemixNessRulesPKFirePillarTVel(int power_kind, float vanilla_tvel);

/* up_special_landing_fsm_ (the aerial end of PK Thunder only). */
float ftRemixNessRulesPKThunderLandingLag(int fkind, float vanilla_lag);
float ftRemixNessRulesPKThunderDrift(int fkind, float vanilla_drift);

/* up_special_ending_delay_ */
int ftRemixNessRulesPKThunderEndDelay(int fkind, int vanilla_delay);

/* psi_magnet_multiplier_ (Lucas.asm) */
float ftRemixNessRulesPsiMagnetHealMultiplier(int fkind);

/* dsp_absorption_turnaround_prevent (Lucas.asm): Lucas keeps his facing when he absorbs. */
int ftRemixNessRulesPsiMagnetKeepsFacing(int fkind);

/* usp_multi_hit_ (Lucas.asm): the hitboxes refresh when the lifetime is a non-zero multiple of the hit rate. */
int ftRemixNessRulesPKThunderHitRefreshDue(int lifetime);

#endif

#endif
