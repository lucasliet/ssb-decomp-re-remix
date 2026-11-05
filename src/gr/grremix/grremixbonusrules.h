#ifndef _GRREMIXBONUSRULES_H_
#define _GRREMIXBONUSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix bonus stage hazards (Hazards.asm marina_btt_setup, btp_bomb_or_barrel_setup, peach_btt_pipe_override),
 * so the headless unit tests (tests/remix) can run them. Ints and floats only.
 */

/** Frames before the first rolling bomb of a stage that does not set its own (the vanilla Race to the Finish actor, 0xB4). */
#define GRREMIX_BONUS_BOMB_WAIT_DEFAULT 180

/** Frames before the first rolling bomb of Marina's Break the Targets (marina_btt_setup overwrites the timer with 1). */
#define GRREMIX_BONUS_BOMB_WAIT_MARINA 1

/** Frames between two rolling bombs (grBonus3TaruBombProcUpdate). */
#define GRREMIX_BONUS_BOMB_WAIT_REPEAT 180

/** Chance threshold of the vanilla dokan exit: a random float at or below it takes the side pipe. */
#define GRREMIX_BONUS_DOKAN_SIDE_THRESHOLD_VANILLA 0.25F

/** Peach's Break the Targets threshold (0x3F900000): above 1, so the random float never misses it. */
#define GRREMIX_BONUS_DOKAN_SIDE_THRESHOLD_PEACH 1.125F

/**
 * Rolling bomb spawner of a bonus stage (marina_btt_setup, btp_bomb_or_barrel_setup bomb_spawn_coordinates).
 *
 * @param gkind Remix stage id.
 * @param x Receives the spawn x (4050 Marina, 4800 Mewtwo, 3150 Crash).
 * @param y Receives the spawn y (6000 Marina, 4200 Mewtwo, 8100 Crash).
 * @param first_wait Receives the frames before the first bomb.
 * @return 1 when the stage has the spawner, 0 otherwise (the outputs are untouched).
 */
extern int grRemixBonusRulesGetRollingBomb(int gkind, float *x, float *y, int *first_wait);

/**
 * Threshold of the random float that picks the side pipe when a fighter leaves the dokan wait (peach_btt_pipe_override).
 *
 * @param gkind Remix stage id.
 */
extern float grRemixBonusRulesGetDokanSideThreshold(int gkind);

#endif

#endif
