#ifndef _SC1PREMIXBONUS3_H_
#define _SC1PREMIXBONUS3_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Bonus 3 mode (SinglePlayerModes.asm): the Race to the Finish stage of the 1P game, entered from the Bonus 2 character select. The
 * player has one stock against three Fighting Polygons at the strongest CPU settings, the match has no time limit and a count-up
 * timer, the title card is skipped, and a finish saves the time in the record of the fighter. Any end of the run goes back to the
 * Bonus 2 character select.
 */

/** TRUE while the 1P game being played is the Bonus 3 mode on its race stage. */
extern sb32 sc1PRemixBonus3IsActive(void);

/**
 * Picks the course of the race, the Normal Race to the Finish or the Remix one (Bonus.mode, the header toggle of the Bonus screens).
 * SSB64_REMIX_BONUS3_COURSE=normal|remix overrides it for the smoke tests.
 *
 * @param course SC1PREMIX_BONUS3_COURSE_NORMAL or SC1PREMIX_BONUS3_COURSE_REMIX.
 */
extern void sc1PRemixBonus3SetCourse(s32 course);

/**
 * Course the next race uses: the choice of the Bonus screen, or the one SSB64_REMIX_BONUS3_COURSE forces.
 *
 * @return SC1PREMIX_BONUS3_COURSE_NORMAL or SC1PREMIX_BONUS3_COURSE_REMIX.
 */
extern s32 sc1PRemixBonus3CourseGet(void);

/**
 * Best finish time of a fighter on the course in use (load_bonus3_time), in ticks.
 *
 * @param fkind Fighter kind.
 * @return The record, 0 when the fighter has none or has no record row.
 */
extern u32 sc1PRemixBonus3RecordGet(s32 fkind);

/** Applies the Bonus 3 rules to the battle being set up: the course, team attack, no items and no time limit; no-op outside the mode. */
extern void sc1PRemixBonus3SetupBattle(void);

/** TRUE when the racers are picked among the Remix Fighting Polygons: the Remix course (polygon_selection_match_rttf). */
extern sb32 sc1PRemixBonus3UsesRemixPolygons(void);

/** Gives the 1P game player the single stock of the mode; no-op outside it. */
extern void sc1PRemixBonus3SetupPlayer(s32 player);

/**
 * CPU level of a polygon of the race: the strongest, whatever the difficulty (_opponent_cpu).
 *
 * @param level Level the stage table gives.
 * @return The level to use.
 */
extern s32 sc1PRemixBonus3EnemyLevel(s32 level);

/** Gives a polygon of the race the knockback ratio of the mode (_opponent_knockback); no-op outside it. */
extern void sc1PRemixBonus3SetupEnemy(s32 player);

/**
 * The player crossed the finish line (_time_save, _new_record): saves the time when it is a record and says which announcer sound the
 * finish plays.
 *
 * @param sfx_id Complete sound the stage would play.
 * @return The sound to play: the New Record one when the record was beaten, the given one otherwise.
 */
extern u16 sc1PRemixBonus3OnComplete(u16 sfx_id);

/** Ends the run: back to the Bonus 2 character select, or a new run when the reset button was used. */
extern void sc1PRemixBonus3LeaveRun(void);

#endif

#endif
