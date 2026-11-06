#ifndef _SC1PBONUSSTAGERULES_H_
#define _SC1PBONUSSTAGERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Remix Bonus stage and bonus timer (SinglePlayer.asm extend_high_score_btt/btp_new_record_check_,
 * set_btt/btp_stage_id_1p_, SinglePlayerModes.asm _load_timer and bonus3_timer update_routine), kept apart so the headless unit
 * tests (tests/remix) run them.
 */

/* update_routine: the Home-Run Contest timer counts down from 10 seconds (0x258 tics). */
#define SC1PBONUSSTAGE_HRC_TIMER_TICS 600

/**
 * Whether finishing a Remix-table stage plays the "New Record" voice: the stored run must be complete and the finish time strictly
 * faster. An incomplete stored run has no time to beat.
 *
 * @param is_record_complete Non-zero when the stored run finished every target or platform.
 * @param record_time Best finish time of the stored run, in ticks.
 * @param time_passed Ticks the finished run took.
 * @return Non-zero when the voice is New Record.
 */
extern int sc1PBonusStageRulesIsNewRecord(int is_record_complete, unsigned int record_time, unsigned int time_passed);

/**
 * Stage of a Bonus 1 or Bonus 2 stage inside the 1P game (set_btt_stage_id_1p_, set_btp_stage_id_1p_): the _remix_1p branches are
 * commented out in the asm and Bonus.mode is ignored, so a fighter added by the Remix plays the stage of its default table and
 * the original fighters play their vanilla stage.
 *
 * @param fkind Fighter kind.
 * @param first_added_fkind First fighter kind added by the Remix (Character.id.BOSS).
 * @param default_stage_id Stage of the default table (Character.BTT_TABLE or BTP_TABLE) for the fighter.
 * @param no_stage_id Value the default table holds for a fighter with no stage.
 * @param vanilla_gkind Vanilla stage of the fighter.
 * @return Stage the Bonus stage plays.
 */
extern int sc1PBonusStageRulesGkindFor1PGame(int fkind, int first_added_fkind, int default_stage_id, int no_stage_id, int vanilla_gkind);

/**
 * Whether the timer of a 1P game run is the bonus count-up timer (_load_timer): Bonus 3, the Home-Run Contest and every Practice
 * run take the bonus timer branch; the other runs keep the match timer.
 *
 * @param is_bonus3 Non-zero in a Bonus 3 run.
 * @param is_hrc Non-zero in a Home-Run Contest run.
 * @param is_practice Non-zero in a Practice run.
 */
extern int sc1PBonusStageRulesUsesCountUpTimer(int is_bonus3, int is_hrc, int is_practice);

/**
 * Tics the bonus timer shows in the Home-Run Contest (update_routine): 600 minus the tics passed. The asm returns without updating
 * once the 10 seconds are spent, so the display stays on the last value.
 *
 * @param time_passed Ticks since GO.
 * @return Tics to show, or -1 when the timer stays as it is.
 */
extern int sc1PBonusStageRulesHrcTimerTics(unsigned int time_passed);

#endif

#endif
