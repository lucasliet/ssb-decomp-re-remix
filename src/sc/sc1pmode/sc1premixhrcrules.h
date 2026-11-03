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
#define SC1PREMIX_HRC_COUNTER_X 278.0F
#define SC1PREMIX_HRC_COUNTER_Y 44.0F
#define SC1PREMIX_HRC_COUNTER_SCALE 0.9F
#define SC1PREMIX_HRC_LABEL_X 282.0F
#define SC1PREMIX_HRC_LABEL_Y 47.0F
#define SC1PREMIX_HRC_LABEL_SCALE 0.625F
#define SC1PREMIX_HRC_COUNTER_CENTER_X 160.0F
#define SC1PREMIX_HRC_CLOUD_START (-7.505F)
#define SC1PREMIX_HRC_CLOUD_WARP_THRESHOLD (-342.505F)
#define SC1PREMIX_HRC_CLOUD_WARP_DISTANCE 335.0F
#define SC1PREMIX_HRC_CLOUD_SPEED_FACTOR (-0.001F)
#define SC1PREMIX_HRC_CLOUD_SCALE_X 2.2334F

/** What an attempt does to the record of the fighter. */
#define SC1PREMIX_HRC_RECORD_KEEP 0
#define SC1PREMIX_HRC_RECORD_FIRST 1
#define SC1PREMIX_HRC_RECORD_BEATEN 2

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

/** Where the distance counter (right aligned) and its "FT." label (left aligned) sit on the 320x240 screen and their font scales. */
typedef struct SC1PRemixHrcCounterLayout
{
    float counter_x;
    float counter_y;
    float counter_scale;
    float label_x;
    float label_y;
    float label_scale;

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
 * Layout of the distance counter and its "FT." label. At COMPLETE both slide left by the same step per tic, sink half a unit and grow by
 * 0.01 per tic over the end timer, until the pair is centred (HRC.asm _complete, _animate_counter). The step is fixed on the first tic from
 * the width of the number: the counter right edge travels to 160 + 0.725 * width - 12.5 in the 46 tics the timer takes to run out.
 *
 * @param complete_tics Tics since the attempt ended in COMPLETE, 0 while it is on; capped at the end timer.
 * @param is_completing Whether the attempt ended in COMPLETE; the layout is the resting one otherwise.
 * @param number_width Width of the counter number at its resting scale, in screen units.
 * @param layout Receives the positions and the font scales.
 */
extern void sc1PRemixHrcCounterAt(int complete_tics, int is_completing, float number_width, SC1PRemixHrcCounterLayout *layout);

/**
 * Scroll speed of the clouds: the contest scrolls them against the Sandbag once it passes the start line.
 *
 * @param sandbag_dx X the Sandbag moved this frame.
 * @param is_past_start Whether the Sandbag is at or past the start line.
 * @return Speed to add to the background X each frame.
 */
extern float sc1PRemixHrcCloudSpeed(float sandbag_dx, int is_past_start);

/**
 * Moves the background by one scroll step. The image is the same clouds twice, so once it has scrolled past the threshold it jumps
 * back by the width of one copy (HRC.asm animate_background_image_).
 *
 * @param x Background X.
 * @param speed Scroll speed.
 * @return The new background X.
 */
extern float sc1PRemixHrcCloudStep(float x, float speed);

/**
 * Compares a distance with the record of the fighter (SinglePlayerModes.asm _time_save): the first distance is saved quietly, a longer one
 * is saved and announced as a new record, anything else leaves the record alone.
 *
 * @param previous Saved record in tenths of a foot, 0 for none.
 * @param distance Distance of the attempt in tenths of a foot.
 * @return SC1PREMIX_HRC_RECORD_*.
 */
extern int sc1PRemixHrcRecordVerdict(unsigned int previous, unsigned int distance);

#endif

#endif
