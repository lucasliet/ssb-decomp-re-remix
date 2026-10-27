#ifndef _SC1PREMIXHRCRULES_H_
#define _SC1PREMIXHRCRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Home-Run Contest that need no game state (HRC.asm main_): the camera that follows the Sandbag, the distance
 * and when an attempt ends. Kept apart so the headless unit tests (tests/remix) run them.
 */

#define SC1PREMIX_HRC_START_X (-19392.0F)
#define SC1PREMIX_HRC_DEFAULT_CAMERA_X (-21312.0F)
#define SC1PREMIX_HRC_UNITS_PER_FOOT_TENTH 9.12F
#define SC1PREMIX_HRC_CAMERA_WARP_THRESHOLD 12528.0F
#define SC1PREMIX_HRC_CAMERA_WARP_DISTANCE (-9120.0F)
#define SC1PREMIX_HRC_COMPLETE_TIMER_START 45
#define SC1PREMIX_HRC_TIME_LIMIT_TICS 600
#define SC1PREMIX_HRC_COUNTER_X 246.0F
#define SC1PREMIX_HRC_COUNTER_Y 44.0F
#define SC1PREMIX_HRC_COUNTER_HEIGHT 16.0F
#define SC1PREMIX_HRC_COUNTER_CENTER_X 160.0F

/** Verdicts of an attempt check. */
#define SC1PREMIX_HRC_VERDICT_KEEP_GOING 0
#define SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS 1
#define SC1PREMIX_HRC_VERDICT_END 2

/** State the contest keeps between frames: the warp that keeps the camera in range, the distance in tenths of a foot and the end timer. */
typedef struct SC1PRemixHrcState
{
    float warp_distance;
    int distance;
    int complete_timer;

} SC1PRemixHrcState;

/** Where the camera and the Sandbag go this frame. */
typedef struct SC1PRemixHrcMove
{
    float camera_x;
    float sandbag_shift;
    int is_past_start;

} SC1PRemixHrcMove;

/** Where the distance counter sits on the 320x240 screen and how tall its text is. */
typedef struct SC1PRemixHrcCounterLayout
{
    float x;
    float y;
    float height;

} SC1PRemixHrcCounterLayout;

/** What the attempt check reads from the game (HRC.asm main_, _check_failure_complete). */
typedef struct SC1PRemixHrcAttempt
{
    int time_passed;
    int is_past_start;
    int is_sandbag_aerial;
    int is_over_platform;
    int is_sandbag_moving;
    int is_human_stuck_in_hold;
    int is_human_resting;

} SC1PRemixHrcAttempt;

/** Clears the state for a new attempt. */
extern void sc1PRemixHrcStateReset(SC1PRemixHrcState *state);

/**
 * Follows the Sandbag for one frame: the camera waits at the default X until the Sandbag passes the start line, then pans a third of the way
 * to it each frame; past the warp threshold the camera and the Sandbag move back together and the warp grows so the distance stays right.
 *
 * @param state State of the contest, updated (distance, warp).
 * @param sandbag_x X of the Sandbag.
 * @param camera_x X of the camera.
 * @param move Receives the new camera X and the shift to apply to the Sandbag.
 */
extern void sc1PRemixHrcTrack(SC1PRemixHrcState *state, float sandbag_x, float camera_x, SC1PRemixHrcMove *move);

/**
 * Whether the attempt goes on (HRC.asm _check_failure_complete): inputs of the player are cut once the time is up, and the attempt ends
 * when the Sandbag has stopped, unless the player is in an action that must finish first.
 *
 * @param attempt What the game reports.
 * @return SC1PREMIX_HRC_VERDICT_*.
 */
extern int sc1PRemixHrcAttemptVerdict(const SC1PRemixHrcAttempt *attempt);

/**
 * Layout of the distance counter. At COMPLETE it moves to the centre of the screen while growing to twice its size and sinking half a unit
 * per tic over the end timer (HRC.asm _complete, _animate_counter).
 *
 * @param complete_tics Tics since the attempt ended in COMPLETE, 0 while it is on; capped at the end timer.
 * @param layout Receives the position and the height of the text.
 */
extern void sc1PRemixHrcCounterAt(int complete_tics, SC1PRemixHrcCounterLayout *layout);

#endif

#endif
