/**
 * ftremixtripping.c - Smash Remix Tripping.asm: random trips on turn, dash and turn run, the turn hitstun
 * reset that keeps a trip, and the recovery from a trip damage status.
 *
 * ftCommonTurnSetStatus, ftCommonTurnProcInterrupt, ftCommonDashSetStatus, ftCommonTurnRunSetStatus and
 * ftCommonDamageCommonProcUpdate call into this file (see docs/remix/toggles.md).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include <ft/fighter.h>
#include <sys/audio.h>
#include <sys/utils.h>

#define FTREMIX_TRIPPING_PLAYERS 4

/* Tripping.asm toggle values: entry_tripping 0 (OFF), 1 (LOW), 2 (HIGH), 3 (BRAWL). */
#define FTREMIX_TRIPPING_OFF 0
#define FTREMIX_TRIPPING_HIGH 2
#define FTREMIX_TRIPPING_BRAWL 3

/* Roll ranges (0, N-1) per entered status, before the toggle divides them. */
#define FTREMIX_TRIPPING_RANGE_TURN_RUN 80
#define FTREMIX_TRIPPING_RANGE_DASH 96
#define FTREMIX_TRIPPING_RANGE_DASH_DANCE 64
#define FTREMIX_TRIPPING_ROLL_HIT 2

#define FTREMIX_TRIPPING_HITSTUN_TICS 8
#define FTREMIX_TRIPPING_SFX_TRIPSTART 0x0456
#define FTREMIX_TRIPPING_RECOVERY_MIN_TICS 6

/* ZCancel.asm CRUEL_Z_CANCEL_MODE: when set, a failed Z-cancel trips regardless of the Tripping toggle. */
#define FTREMIX_TRIPPING_ZCANCEL_TRIP 10
#define FTREMIX_TRIPPING_ZCANCEL_RANDOM 11

/* Action.asm Tumble, not in the vanilla status enum. */
#define FTREMIX_TRIPPING_STATUS_TUMBLE 0x039

/* Tripping.asm player_trip_flag: one flag per port, set by a trip and cleared when the trip ends. */
static u8 sFTRemixTrippingFlag[FTREMIX_TRIPPING_PLAYERS];

/* Returns the roll range (0, N-1) for entering status_id, or 0 when no roll applies to it.
 * Turn only rolls when it follows a dash (dash dance). */
static s32 ftRemixTrippingRollRange(FTStruct *fp, s32 status_id)
{
    if (status_id == nFTCommonStatusTurnRun)
    {
        return FTREMIX_TRIPPING_RANGE_TURN_RUN;
    }
    if (status_id == nFTCommonStatusDash)
    {
        return FTREMIX_TRIPPING_RANGE_DASH;
    }
    if (fp->status_id != nFTCommonStatusDash)
    {
        return 0;
    }
    return FTREMIX_TRIPPING_RANGE_DASH_DANCE;
}

/* Brawl divides the range by 16 and High by 4 (Tripping.asm trip_roll). */
static s32 ftRemixTrippingScaleRange(s32 range, s32 toggle_value)
{
    if (toggle_value == FTREMIX_TRIPPING_BRAWL)
    {
        return range >> 4;
    }
    if (toggle_value == FTREMIX_TRIPPING_HIGH)
    {
        return range >> 2;
    }
    return range;
}

/* Puts the fighter in hitstun and plays the trip sound. The caller switches the status to DamageLw3. */
static void ftRemixTrippingApply(FTStruct *fp)
{
    fp->status_vars.common.damage.hitstun_tics = FTREMIX_TRIPPING_HITSTUN_TICS;
    syAudioPlayFGM(FTREMIX_TRIPPING_SFX_TRIPSTART);
    sFTRemixTrippingFlag[fp->player] = TRUE;
}

/* Returns the status to enter: DamageLw3 when the roll trips the fighter, status_id otherwise.
 * Called before the vanilla ftMainSetStatus of Turn, Dash and TurnRun (asm: tripping, jal in the status set). */
s32 ftRemixTrippingEnterStatus(FTStruct *fp, s32 status_id)
{
    s32 range;
    s32 toggle_value;

    toggle_value = gmRemixToggle(tripping);
    if (toggle_value == FTREMIX_TRIPPING_OFF)
    {
        return status_id;
    }

    sFTRemixTrippingFlag[fp->player] = FALSE;

    range = ftRemixTrippingRollRange(fp, status_id);
    if (range == 0)
    {
        return status_id;
    }

    range = ftRemixTrippingScaleRange(range, toggle_value);
    if (syUtilsRandIntRange(range) != FTREMIX_TRIPPING_ROLL_HIT)
    {
        return status_id;
    }

    ftRemixTrippingApply(fp);
    return nFTCommonStatusDamageLw3;
}

/* ZCancel.asm _trip ends by setting player_trip_flag to 1 after the status change (its first store, meant to clear the flag,
 * writes the port index but is overwritten). The flag keeps the hitstun of the trip alive, see ftRemixTrippingAllowsTurnStateReset. */
void ftRemixTrippingStoreZCancelFlag(s32 player)
{
    sFTRemixTrippingFlag[player] = TRUE;
}

/* FALSE only while a ground trip is running; the turn state reset must then keep the hitstun
 * (Tripping.asm trip_turn_hitstun_retention_1/2 skip the store at 0x0B18). */
sb32 ftRemixTrippingAllowsTurnStateReset(FTStruct *fp)
{
    if (gmRemixToggle(tripping) == FTREMIX_TRIPPING_OFF)
    {
        return TRUE;
    }
    return (sFTRemixTrippingFlag[fp->player] == FALSE);
}

/* Z-cancel punishment that always trips (TRIP or RANDOM) ignores the Tripping toggle
 * (Tripping.asm damaged_check_trip). */
static sb32 ftRemixTrippingPunishIsTrip(void)
{
    s32 punish_mode;

    punish_mode = gmRemixToggle(punish_on_failed_z_cancel);
    return (punish_mode == FTREMIX_TRIPPING_ZCANCEL_TRIP) || (punish_mode == FTREMIX_TRIPPING_ZCANCEL_RANDOM);
}

/* Status after a finished trip: DownBounceD on the ground, Tumble in the air. */
static s32 ftRemixTrippingRecoveryStatus(FTStruct *fp)
{
    if (fp->ga == nMPKineticsGround)
    {
        return nFTCommonStatusDownBounceD;
    }
    return FTREMIX_TRIPPING_STATUS_TUMBLE;
}

/* Recovers a trip once its damage animation has played (Tripping.asm damaged_check_trip).
 * Returns TRUE when the status was changed, so the caller returns before its own damage update. */
sb32 ftRemixTrippingFinishDamage(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((ftRemixTrippingPunishIsTrip() == FALSE) && (gmRemixToggle(tripping) == FTREMIX_TRIPPING_OFF))
    {
        return FALSE;
    }
    if (sFTRemixTrippingFlag[fp->player] == FALSE)
    {
        return FALSE;
    }
    if (fp->status_id != nFTCommonStatusDamageLw3)
    {
        sFTRemixTrippingFlag[fp->player] = FALSE;
        return FALSE;
    }
    if (fp->status_total_tics < FTREMIX_TRIPPING_RECOVERY_MIN_TICS)
    {
        return FALSE;
    }

    sFTRemixTrippingFlag[fp->player] = FALSE;
    ftMainSetStatus(fighter_gobj, ftRemixTrippingRecoveryStatus(fp), 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    return TRUE;
}

#endif
