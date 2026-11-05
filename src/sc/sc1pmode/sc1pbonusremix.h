#ifndef _SC1PBONUSREMIX_H_
#define _SC1PBONUSREMIX_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Remix Break the Targets and Board the Platforms (Bonus.asm, SinglePlayer.asm, Stages.asm bonus_fix_*): the 33 stages of the Bonus 1
 * and 2 screens with the Normal / Remix choice (Bonus.mode, Bonus.stage), the stage a fighter plays, the targets and platforms of the
 * Remix stages and the records of a fighter on a stage that is not his own. The tables are read from the Remix image.
 */

/** Stage index picked on the STAGE row (Bonus.stage). */
extern s32 sc1PBonusRemixGetStageIndex(void);
extern void sc1PBonusRemixSetStageIndex(s32 index);

/** TRUE when the Bonus screen choice is Remix (Bonus.mode). */
extern sb32 sc1PBonusRemixIsRemixChoice(void);

/** Stage id at an index of the BTT or BTP table (Bonus.btt_stage_table, btp_stage_table). */
extern s32 sc1PBonusRemixGetStageId(sb32 is_platforms, s32 index);

/** Index of a stage id in the BTT or BTP table, -1 when it is not there. */
extern s32 sc1PBonusRemixFindStageIndex(sb32 is_platforms, s32 stage_id);

/** Stage a fighter normally plays (Character.BTT_TABLE or BTP_TABLE), 0xFF when none. */
extern s32 sc1PBonusRemixGetDefaultStageId(sb32 is_platforms, s32 fkind);

/** The one or two fighters whose stock icons stand for a stage (Bonus.stock_icon_table); second is 0 when there is only one. */
extern void sc1PBonusRemixGetStageIcons(s32 index, s32 *first, s32 *second);

/** Offset of the stock icon of a fighter in the icon file (SinglePlayerModes.icon_offset_table). */
extern u32 sc1PBonusRemixGetIconOffset(s32 fkind);

/**
 * Stage a Break the Targets or Board the Platforms run plays for a fighter (set_btt_stage_id_, set_btp_stage_id_).
 *
 * @param is_platforms TRUE for Board the Platforms.
 * @param fkind Fighter kind.
 * @return The stage id, or -1 to keep the standard stage of the fighter.
 */
extern s32 sc1PBonusRemixGetBattleGkind(sb32 is_platforms, s32 fkind);

/** TRUE when a stage is a Board the Platforms one, by the class table for Remix stages (bonus_fix_3_). */
extern sb32 sc1PBonusRemixIsPlatformStage(s32 gkind);

/**
 * The three words of a Remix bonus stage (bonus_pointer_table): for targets the start, the DObjDesc and the AObjEvent offsets, for
 * platforms the first two of them.
 *
 * @return TRUE when the stage has them.
 */
extern sb32 sc1PBonusRemixGetStageWords(s32 gkind, u32 *start, u32 *dobjdesc, u32 *anim_joint);

/**
 * Whether the record shown on the Bonus screen for a fighter is in the Remix table: the choice is Remix and the stage picked is not
 * the one the fighter normally plays.
 */
extern sb32 sc1PBonusRemixUsesRecordTable(sb32 is_platforms, s32 fkind);

/** Targets or platforms of the record of the picked stage, 0 outside the Remix table. */
extern s32 sc1PBonusRemixGetRecordCount(sb32 is_platforms, s32 fkind);

/** Best finish time of the record of the picked stage, in ticks. */
extern u32 sc1PBonusRemixGetRecordTime(sb32 is_platforms, s32 fkind);

/** TRUE when the fighter finished the picked stage. */
extern sb32 sc1PBonusRemixIsRecordComplete(sb32 is_platforms, s32 fkind);

/**
 * Saves a run of the bonus stage being played into the Remix table when the stage is not the one the fighter normally plays
 * (extend_high_score_btt/btp_count_write_ and _time_write_ _remix).
 *
 * @param is_tasks_fail TRUE when the targets or platforms were not all done.
 * @param fkind Fighter kind.
 * @param tasks_complete Targets or platforms done.
 * @param time_passed Ticks of the run.
 * @return TRUE when the run belongs to the Remix table (saved or skipped), FALSE for the standard record.
 */
extern sb32 sc1PBonusRemixTrySave(sb32 is_tasks_fail, s32 fkind, s32 tasks_complete, u32 time_passed);

#endif

#endif
