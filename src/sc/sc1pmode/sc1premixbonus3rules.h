#ifndef _SC1PREMIXBONUS3RULES_H_
#define _SC1PREMIXBONUS3RULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Remix Bonus 3 mode (SinglePlayerModes.asm, Bonus 3 branches), kept apart so the headless unit tests (tests/remix) run
 * them. Bonus 3 is the Race to the Finish stage of the 1P game played alone: three Fighting Polygons at the strongest CPU settings,
 * one stock, no time limit, and a timer that counts up until the finish line.
 */

/* _opponent_cpu (_level9) and _opponent_knockback (_bonus3): the Very Hard values of the stage. */
#define SC1PREMIX_BONUS3_ENEMY_LEVEL 9
#define SC1PREMIX_BONUS3_ENEMY_HANDICAP 9

/* load_player_stocks: the stock count 0 is one stock. */
#define SC1PREMIX_BONUS3_PLAYER_STOCKS 0

/* _time_save: a record slot holding this filler is a console memory fix and counts as empty. */
#define SC1PREMIX_BONUS3_RECORD_FILLER 0xAAAAAAAAu

/* Normal course (Stages.id.RACE_TO_THE_FINISH) and Remix course (Stages.id.REMIX_RTTF) of the Bonus header toggle. */
#define SC1PREMIX_BONUS3_COURSE_NORMAL 0
#define SC1PREMIX_BONUS3_COURSE_REMIX 1

/* Outcome of a finished race against the record of the fighter. */
#define SC1PREMIX_BONUS3_RECORD_KEEP 0
#define SC1PREMIX_BONUS3_RECORD_FIRST 1
#define SC1PREMIX_BONUS3_RECORD_BEATEN 2

/**
 * Compares a finish time with the record of the fighter (_time_save, _save_check): an empty slot takes the time without a fanfare,
 * a strictly faster time beats the record, anything else keeps it. The time is in ticks and lower is better.
 *
 * @param previous Record slot of the fighter, 0 or SC1PREMIX_BONUS3_RECORD_FILLER when empty.
 * @param finish_time Ticks the race took.
 * @return SC1PREMIX_BONUS3_RECORD_*.
 */
extern int sc1PRemixBonus3RecordVerdict(unsigned int previous, unsigned int finish_time);

/**
 * Announcer sound of the finish (_new_record): the Complete sound turns into the New Record one when the record was beaten.
 *
 * @param verdict SC1PREMIX_BONUS3_RECORD_*.
 * @param complete_fgm Sound of a plain finish.
 * @param record_fgm Sound of a new record.
 */
extern int sc1PRemixBonus3CompleteFgm(int verdict, int complete_fgm, int record_fgm);

/**
 * Whether a finish may be saved: the Single Player Enemy option must leave the enemy to the CPU (_time_save) and the high score
 * switch of SinglePlayer.asm (always on in the port) must be on.
 *
 * @param enemy_port 0 when the CPU plays the enemy, otherwise the port index plus one.
 */
extern int sc1PRemixBonus3CanSaveRecord(int enemy_port);

/**
 * Vertical position of the count-up timer (bonus3_timer): digits at 30 and the "sec" and "csec" marks at 20, or 210 and 200 with the
 * Dragon King HUD setting "always" (1).
 *
 * @param dragon_king_hud Toggles.entry_dragon_king_hud.
 * @param digit_y Receives the Y of the digits.
 * @param symbol_y Receives the Y of the marks.
 */
extern void sc1PRemixBonus3TimerY(int dragon_king_hud, float *digit_y, float *symbol_y);

#endif

#endif
