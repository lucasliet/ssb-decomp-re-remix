/**
 * ftremixspeed.c - Smash Remix Speed.asm: the Game Speed toggle (and Stopwatch backfire speed) decides how many
 * gcRunAll frames run per game frame.
 *
 * handle_rate_ replaces the gcRunAll call at the end of ifCommonBattleGoUpdateInterface (RAM 0x8011427C).
 * handle_rate_training is called by Training.asm and returns the new Training.skip_advance value.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include "it/itremix/itremixstopwatch.h"
#include "sys/objman.h"

#define FTREMIX_SPEED_DEFAULT 0
#define FTREMIX_SPEED_2X 5
#define FTREMIX_SPEED_3X 6
#define FTREMIX_SPEED_FIRST_SLOW 7
#define FTREMIX_SPEED_LAST 12

/* Speed.asm waiting_time_table: {active frames, total frames} for speeds 1 to 12 (2x and 3x are not read from it). */
static const u8 sFTRemixSpeedWaitingTable[FTREMIX_SPEED_LAST][2] =
{
    { 5, 6 },
    { 3, 4 },
    { 2, 3 },
    { 4, 7 },
    { 0, 2 },
    { 0, 3 },
    { 1, 8 },
    { 1, 4 },
    { 1, 3 },
    { 1, 2 },
    { 2, 3 },
    { 3, 4 },
};

/* Speed.asm waiting_timer, shared by handle_rate_ and handle_rate_training. */
static s32 sFTRemixSpeedWaitingTimer = 1;

/* Speed.asm additional_advance: TRUE while an extra advance of the current game frame runs (read by PoisonDmg). */
static sb32 sFTRemixSpeedAdditionalAdvance;

/**
 * Returns the speed in effect: the Stopwatch backfire speed when one is active, otherwise the Game Speed toggle.
 */
static s32 ftRemixSpeedCurrent(void)
{
    u8 watch_speed = itRemixStopwatchGetWatchSpeed();

    if (watch_speed != 0)
    {
        return watch_speed;
    }
    return gmRemixToggle(game_speed);
}

/**
 * Advances the waiting timer of a slow or fast table speed and returns its frame count for this game frame:
 * 1 for a normal frame, 0 for a skipped frame (slow speeds), 2 for an extra frame (fast speeds).
 */
static s32 ftRemixSpeedTableFrameCount(s32 speed)
{
    s32 remaining;
    s32 is_active;
    u8 active_frames = sFTRemixSpeedWaitingTable[speed - 1][0];
    u8 total_frames = sFTRemixSpeedWaitingTable[speed - 1][1];

    remaining = sFTRemixSpeedWaitingTimer - 1;
    if (remaining < 0)
    {
        remaining = 0;
    }
    if (remaining == 0)
    {
        sFTRemixSpeedWaitingTimer = total_frames;
    }
    else
    {
        sFTRemixSpeedWaitingTimer = remaining;
    }

    is_active = (remaining < active_frames);
    if (speed < FTREMIX_SPEED_FIRST_SLOW)
    {
        return is_active ? 1 : 2;
    }
    return is_active ? 1 : 0;
}

/**
 * Returns the number of gcRunAll calls for this game frame (0 skips the frame). Unknown speed values run a
 * normal frame.
 */
static s32 ftRemixSpeedFrameCount(void)
{
    s32 speed = ftRemixSpeedCurrent();

    switch (speed)
    {
        case FTREMIX_SPEED_DEFAULT:
            return 1;

        case FTREMIX_SPEED_2X:
            return 2;

        case FTREMIX_SPEED_3X:
            return 3;

        default:
            break;
    }
    if ((speed < 1) || (speed > FTREMIX_SPEED_LAST))
    {
        return 1;
    }
    return ftRemixSpeedTableFrameCount(speed);
}

/**
 * Runs the extra frames of a game frame, each with additional_advance set, and returns how many ran.
 */
static s32 ftRemixSpeedRunExtraFrames(s32 frame_count)
{
    s32 i;

    for (i = 0; i < frame_count - 1; i++)
    {
        sFTRemixSpeedAdditionalAdvance = TRUE;
        gcRunAll();
    }
    return i;
}

/**
 * Returns TRUE while an extra advance of the current game frame runs (Speed.additional_advance).
 */
sb32 ftRemixSpeedIsAdditionalAdvance(void)
{
    return sFTRemixSpeedAdditionalAdvance;
}

/**
 * Runs the gcRunAll frames of the battle loop for the Game Speed toggle (Speed.asm handle_rate_). Replaces the
 * vanilla gcRunAll call of ifCommonBattleGoUpdateInterface. The last frame clears additional_advance.
 */
void ftRemixSpeedRunFrames(void)
{
    s32 frame_count = ftRemixSpeedFrameCount();

    if (frame_count == 0)
    {
        return;
    }
    ftRemixSpeedRunExtraFrames(frame_count);
    sFTRemixSpeedAdditionalAdvance = FALSE;
    gcRunAll();
}

/**
 * Runs the extra frames for the Game Speed toggle in Training (Speed.asm handle_rate_training) and returns the new
 * Training.skip_advance value. Training then runs its own frame unless the returned value is TRUE.
 * training_speed_is_default is FALSE when the Training speed multiplier overrides Game Speed; then the flag is kept.
 */
sb32 ftRemixSpeedTrainingRunFrames(sb32 training_speed_is_default, sb32 skip_advance)
{
    s32 frame_count;

    if (!training_speed_is_default)
    {
        return skip_advance;
    }
    frame_count = ftRemixSpeedFrameCount();
    if (frame_count == 0)
    {
        return TRUE;
    }
    ftRemixSpeedRunExtraFrames(frame_count);
    return FALSE;
}

#endif
