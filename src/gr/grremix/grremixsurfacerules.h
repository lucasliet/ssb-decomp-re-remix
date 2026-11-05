#ifndef _GRREMIXSURFACERULES_H_
#define _GRREMIXSURFACERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure surface knockback rules of Surface.asm (knockback_table, get_struct_, fgm_override_) and of the Casino Night Zone
 * side bumpers (Hazards.asm casino_night_side_bumper_knockback), free of game globals so the headless unit tests
 * (tests/remix) can run them. See grremixsurface.h for the game side.
 */

#define GRREMIX_SURFACE_RULES_ROW_COUNT 20
#define GRREMIX_SURFACE_RULES_BOUNCE_MATERIAL 0x24

/* Hazards.asm 4458: wall surface ids above 0x16 are the Casino side bumpers. */
#define GRREMIX_SURFACE_RULES_BUMPER_LAST_NON_BUMPER 0x16
#define GRREMIX_SURFACE_RULES_BUMPER_SIDE_SHIFT 3

/* Action.asm DamageHigh1 and DamageFlyRoll: the bumper knockback never hits a fighter in these damage statuses. */
#define GRREMIX_SURFACE_RULES_STATUS_DAMAGE_FIRST 0x25
#define GRREMIX_SURFACE_RULES_STATUS_DAMAGE_LAST 0x37

/*
 * One knockback struct of Surface.asm (type, damage, angle, growth, fixed, base, effect, fgm). The first seven members have
 * the order of GRAttackColl. kind keeps the raw type word: a non-zero high halfword is the "cruel" flag that the
 * Hazard Mode toggle does not turn off. fgm_id is -1 for the original sound.
 */
typedef struct GRRemixSurfaceKnockback
{
    int material;
    int kind;
    int damage;
    int angle;
    int growth;
    int fixed;
    int base;
    int effect;
    int fgm_id;

} GRRemixSurfaceKnockback;

/** Row of the knockback table of a material, or -1 when the material has no knockback struct (get_struct_ NULL). */
int grRemixSurfaceRulesFindRow(unsigned int material);

/** Knockback struct of a row returned by grRemixSurfaceRulesFindRow. */
const GRRemixSurfaceKnockback* grRemixSurfaceRulesGetRow(int row);

/** TRUE when the struct is cruel (type high halfword set), which ignores the Hazard Mode toggle (get_struct_ _struct). */
int grRemixSurfaceRulesIsCruel(const GRRemixSurfaceKnockback *knockback);

/** TRUE when Hazard Mode off removes the knockback of this struct (get_struct_ _check_hazard_mode). */
int grRemixSurfaceRulesIsBlockedByHazardMode(const GRRemixSurfaceKnockback *knockback, int hazards_are_on);

/** Casino bumper wall surface adjusted by the facing of the fighter, or -1 when the wall is not a bumper. */
int grRemixSurfaceRulesBumperMaterial(unsigned int wall_surface, int is_left_wall, int lr);

/** TRUE when a fighter in this status can be knocked back by a Casino side bumper (not in DamageHigh1..DamageFlyRoll). */
int grRemixSurfaceRulesBumperStatusApplies(int status_id);

#endif

#endif
