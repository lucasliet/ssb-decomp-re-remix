#ifndef _SC1PREMIXENEMYAFK_H_
#define _SC1PREMIXENEMYAFK_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * "afk" player tag automation of Remix (SinglePlayerEnemy.asm _afk_1p, Credits.asm remix_credits_speed_assist_): while the custom
 * player tag number 2 (Remix > Player Tags, or SSB64_REMIX_TAG_NAMES="2=afk") is named "afk", the 1P game hands the playing
 * fighter to a level 10 CPU, the screens that wait for a button press go on by themselves and the credits slow down by themselves
 * at the first Remix name.
 */

/** TRUE while the custom player tag number 2 is named "afk". */
extern sb32 sc1PRemixAfkIsActive(void);

/**
 * Button check of a screen that waits for a press (_handle_stage_clear, _handle_preview, _handle_continue).
 *
 * @param buttons Button mask the screen waits for.
 * @return TRUE while the automation is on, otherwise the result of scSubsysControllerGetPlayerTapButtons.
 */
extern sb32 sc1PRemixAfkTapButtons(u32 buttons);

/**
 * Condition of a screen the automation forces (_handle_bonus time up flag, _handle_rttf gate material).
 *
 * @param condition Condition the screen would evaluate.
 * @return TRUE while the automation is on, otherwise condition.
 */
extern sb32 sc1PRemixAfkForceTrue(sb32 condition);

/**
 * Hands the playing fighter slot of the 1P game to a level 10 CPU (_afk_1p._hmn_check). Does nothing for other slots, in the
 * Home-Run Contest or while the automation is off. Call before the slot data is copied to the fighter description.
 *
 * @param slot Player slot being spawned.
 * @param mapobj_kind Spawn kind of the slot (nMPMapObjKind1PGame*).
 */
extern void sc1PRemixAfkSetupSlot(s32 slot, s32 mapobj_kind);

/**
 * Whether the credits behave as if Start was pressed (remix_credits_speed_assist_), see sc1PRemixAfkSpeedAssistFires.
 *
 * @param name_id Staff roll name count (sSCStaffrollNameID).
 * @param speed Current roll speed.
 * @param normal_speed Roll speed when Start was not pressed.
 */
extern sb32 sc1PRemixAfkIsSpeedAssistActive(s32 name_id, f32 speed, f32 normal_speed);

#endif

#endif
