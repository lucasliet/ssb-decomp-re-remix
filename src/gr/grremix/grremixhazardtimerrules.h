#ifndef _GRREMIXHAZARDTIMERRULES_H_
#define _GRREMIXHAZARDTIMERRULES_H_

#include <stdint.h>

/*
 * Pure timer rules of the Rainbow Road, Metallic Madness and Spawned Fear hazards (Hazards.asm sfx_main_,
 * chainchomp_hitbox_main_, shrink_timer_ and cacodemon_factory_). Ints only, so the headless tests can run them.
 */

#define GRREMIX_RAINBOW_SFX_FIRST_TIMER 0x0960
#define GRREMIX_RAINBOW_SFX_INTERVAL 0x003C
/* Hazards.asm 5953-5958: t6 starts at 4 and the bnez delay slot decrements it on every pass, so five loop cues play. */
#define GRREMIX_RAINBOW_SFX_LOOP_COUNT 5
#define GRREMIX_RAINBOW_SFX_LAST_TIMER (GRREMIX_RAINBOW_SFX_FIRST_TIMER + GRREMIX_RAINBOW_SFX_INTERVAL * (GRREMIX_RAINBOW_SFX_LOOP_COUNT + 1))
#define GRREMIX_RAINBOW_FGM_SFX_NONE 0
#define GRREMIX_RAINBOW_FGM_SFX_FIRST 0x0402
#define GRREMIX_RAINBOW_FGM_SFX_LOOP 0x0403
#define GRREMIX_RAINBOW_FGM_SFX_LAST 0x0404

#define GRREMIX_RAINBOW_CHOMP_SOUND_FRAMES 0x0F

#define GRREMIX_CACODEMON_START_FRAME 1500  /* asm 0x05DC: no spawn roll before this timer value */
#define GRREMIX_CACODEMON_ROLL_RANGE 1500   /* asm: random integer in [0, 1500) per frame */
#define GRREMIX_CACODEMON_ROLL_HIT 80       /* asm 0x50: the roll value that spawns a Cacodemon */
#define GRREMIX_CACODEMON_FORCE_FRAME 3000  /* asm 0x0BB8: the spawn always triggers on this timer value */

typedef enum GRRemixCacodemonFactoryAction
{
    nGRRemixCacodemonFactoryActionNone = 0,     /* keep counting */
    nGRRemixCacodemonFactoryActionRestart = 1,  /* a Cacodemon is alive: only the timer goes back to 0 */
    nGRRemixCacodemonFactoryActionSpawn = 2     /* spawn a Cacodemon (the timer goes back to 0 first) */

} GRRemixCacodemonFactoryAction;

/**
 * FGM of the Rainbow Road start sequence for one sfx timer value (sfx_main_): 0x402 at 0x960, 0x403 every 60 frames five
 * times, then 0x404 at 0xAC8, which also restarts the timer.
 *
 * @param timer The sfx timer before this frame's increment.
 * @return The FGM id to play, or GRREMIX_RAINBOW_FGM_SFX_NONE.
 */
int32_t grRemixRainbowRulesSfxCue(int32_t timer);

/**
 * Growl roll window of the chomp hitbox (chainchomp_hitbox_main_). The asm reads byte 0x0E of the weapon GObj, which is
 * frame_draw_last, the low 8 bits of the frame count of the last draw, so the window reopens every 256 frames.
 *
 * @param frame_draw_last GObj frame_draw_last of the chomp hitbox weapon.
 * @return 1 when the growl roll runs this frame, else 0.
 */
int32_t grRemixRainbowRulesGrowlWindowIsOpen(uint8_t frame_draw_last);

/**
 * Laser shrink timer kept for one port (shrink_timer_ first loop): an active Super or Poison Mushroom clears it, so the
 * mushroom alone owns the size.
 *
 * @param timer The port's shrink timer.
 * @param is_mushroom_active Nonzero while a mushroom effect runs on the port (asm player_shrooms[port] != 0).
 * @return The timer to keep.
 */
uint32_t grRemixMetallicRulesShrinkTimerKept(uint32_t timer, int32_t is_mushroom_active);

/**
 * Decision of the Cacodemon factory for one frame past the 1500-frame wait (cacodemon_factory_ _spawn). A roll hit or the
 * forced frame restarts the timer even when a Cacodemon is alive: the restart sits in the delay slot of the prevent check
 * (Hazards.asm 8641-8643).
 *
 * @param frame The factory timer before this frame's increment, at least GRREMIX_CACODEMON_START_FRAME.
 * @param roll The roll of this frame, in [0, GRREMIX_CACODEMON_ROLL_RANGE).
 * @param is_spawn_prevented Nonzero while a Cacodemon is alive.
 * @return What the factory does this frame.
 */
GRRemixCacodemonFactoryAction grRemixCacodemonRulesFactoryAction(uint32_t frame, int32_t roll, int32_t is_spawn_prevented);

#endif
