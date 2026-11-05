#ifndef _GRREMIXBARRELRULES_H_
#define _GRREMIXBARRELRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix barrel stages (Hazards.asm congo_jungle_barrel_), so the headless unit tests (tests/remix) can run them.
 * Ints and floats only.
 */

/** The stage makes no barrel. */
#define GRREMIX_BARREL_STAGE_NONE 0

/** Congo Jungle, its Remix clones and Congo Falls: the normal moving barrel, made only when the hazards are on. */
#define GRREMIX_BARREL_STAGE_HAZARD 1

/** Smashketball: two static barrels pointing down at each side of the main platform. */
#define GRREMIX_BARREL_STAGE_SMASHKETBALL 2

/** Smashketball.asm barrels: x of the right one (0x45772000); the left one is the negative. */
#define GRREMIX_BARREL_SMASHKETBALL_X 3954.0F

/** Smashketball.asm barrels: rotation z of both, pointing down (0x41AFF000). */
#define GRREMIX_BARREL_SMASHKETBALL_ROTATE 21.9921875F

/**
 * Barrel kind of a stage (congo_jungle_barrel_).
 *
 * @param gkind Remix stage id.
 * @return GRREMIX_BARREL_STAGE_NONE, GRREMIX_BARREL_STAGE_HAZARD or GRREMIX_BARREL_STAGE_SMASHKETBALL.
 */
extern int grRemixBarrelRulesGetStageKind(int gkind);

/**
 * Whether the Smashketball barrels are made: always in the Smashketball mode, in a normal match on that stage only with
 * the hazards on (congo_jungle_barrel_ _check_toggle).
 *
 * @param is_smashketball_mode 1 in the Smashketball mode.
 * @param hazards_on 1 when the hazard toggle is on.
 */
extern int grRemixBarrelRulesSmashketballIsMade(int is_smashketball_mode, int hazards_on);

/**
 * Y of the Smashketball barrels: 2944 (0x45380000, near the top) in the Smashketball mode, 1775 (0x44DDE000, midway up)
 * in a normal match on the stage.
 */
extern float grRemixBarrelRulesGetSmashketballY(int is_smashketball_mode);

#endif

#endif
