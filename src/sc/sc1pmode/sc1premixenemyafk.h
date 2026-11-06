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

/**
 * A press of the Sound Test (FGM.asm afk_soundTest._A_away): a real A tap, or the automation's own press after the music played for
 * SC1PREMIX_AFK_SOUNDTEST_WAIT_TIME frames. Counts the frames, so call it once per frame.
 *
 * @return TRUE when A counts as pressed this frame.
 */
extern sb32 sc1PRemixAfkSoundTestTapA(void);

/**
 * Right press of the Sound Test the automation makes one frame before its A press (afk_soundTest._right_away), so the next song plays.
 *
 * @return TRUE when right counts as pressed this frame.
 */
extern sb32 sc1PRemixAfkSoundTestTapRight(void);

/**
 * A or B check of the credits cursor (awayMode._auto_shoot): with the automation on the cursor shoots on every frame. Remembers
 * whether the player tapped, for sc1PRemixAfkCreditsShotIsSilent.
 *
 * @param button_tap Buttons tapped this frame.
 * @return TRUE when the cursor shoots this frame.
 */
extern sb32 sc1PRemixAfkCreditsTapShoot(u16 button_tap);

/**
 * Whether the shot of this frame skips its sound and highlight (awayMode._handle_shoot_fgm, _handle_shoot_gfx): the automation fired
 * it and the player did not tap A or B. Call after sc1PRemixAfkCreditsTapShoot in the same frame.
 *
 * @return TRUE when the sound and the highlight are suppressed.
 */
extern sb32 sc1PRemixAfkCreditsShotIsSilent(void);

#endif

#endif
