#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixstopwatch.h"
#include "remix/remix_roster_data.h"
#include "gm/gmremixcolscripts.h"
#ifdef REGION_REMIX
#include "gm/gmremixtoggle.h"
#endif
#include <it/item.h>
#include <it/itcommon/ittomato.h>
#include <if/ifcommon.h>
#include <ft/fighter.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <wp/weapon.h>
#include <wp/wpprocess.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include <ef/efmanager.h>

extern void *func_800269C0_275C0(u16 id);

#define ITREMIX_STOPWATCH_KIND 0x39
#define ITREMIX_STOPWATCH_FILE_OFFSET 0x1120
#define ITREMIX_STOPWATCH_MAX_PORTS 4
#define ITREMIX_STOPWATCH_BACKFIRE_TABLE_SIZE 20
#define ITREMIX_STOPWATCH_FAST_TABLE_SIZE 20
#define ITREMIX_STOPWATCH_SLOW_TABLE_SIZE 16
#define ITREMIX_STOPWATCH_SLOWDOWN_DURATION (9 * 60)
#define ITREMIX_STOPWATCH_SPEEDUP_DURATION (10 * 60)
#define ITREMIX_STOPWATCH_BACKFIRE_ODDS 10
#define ITREMIX_STOPWATCH_SPEEDUP_ODDS 20
#define ITREMIX_STOPWATCH_ROLL_RANGE 100
#define ITREMIX_STOPWATCH_CIRCLE_SIZE 20.0F

#define ITREMIX_STOPWATCH_BEHAVIOUR_DEFAULT 0
#define ITREMIX_STOPWATCH_BEHAVIOUR_SLOW_ALWAYS 1
#define ITREMIX_STOPWATCH_BEHAVIOUR_FAST_ALWAYS 2
#define ITREMIX_STOPWATCH_BEHAVIOUR_BACKFIRE_ALWAYS 3
#define ITREMIX_STOPWATCH_BEHAVIOUR_BACKFIRE_NEVER 4

#define ITREMIX_STOPWATCH_SPEED_SLOW 0
#define ITREMIX_STOPWATCH_SPEED_FAST 1

#define ITREMIX_STOPWATCH_COLANIM_SLOWED 0x86 /* asm GFX_ROUTINE, GFXRoutine.asm add_gfx_routine(STOPWATCH_SLOWED) */
#define ITREMIX_STOPWATCH_COLANIM_FASTED 0x87 /* asm GFX_ROUTINE_FAST, GFXRoutine.asm add_gfx_routine(STOPWATCH_FASTED) */

/* Action ids below 4 are the KO and death states; the asm treats them as "just died". */
#define ITREMIX_STOPWATCH_DEATH_STATUS_LIMIT 4
#define ITREMIX_STOPWATCH_ACTION_REVIVE1 0x007
#define ITREMIX_STOPWATCH_ACTION_REVIVE2 0x008
#define ITREMIX_STOPWATCH_ACTION_REVIVE_WAIT 0x009
#define ITREMIX_STOPWATCH_ACTION_PK_THUNDER_START1 0x0E4
#define ITREMIX_STOPWATCH_ACTION_PK_THUNDER_START2 0x0E5
#define ITREMIX_STOPWATCH_ACTION_PK_THUNDER_START_AIR 0x0E8
#define ITREMIX_STOPWATCH_ACTION_PK_THUNDER_AIR 0x0E9
#define ITREMIX_STOPWATCH_ACTION_CRASH_DIGGIN_BEGIN 0x0E5
#define ITREMIX_STOPWATCH_PROJECTILE_PK_THUNDER_HEAD 0x0E
#define ITREMIX_STOPWATCH_PROJECTILE_PK_THUNDER_TAIL 0x0F

#define ITREMIX_STOPWATCH_FGM_TIMER 0x600
#define ITREMIX_STOPWATCH_FGM_BACKFIRE_TIMER 0x601
#define ITREMIX_STOPWATCH_FGM_TIMER_END 0x605
#define ITREMIX_STOPWATCH_FGM_BACKFIRE_TIMER_END 0x606

/* Per-player countdown. Mirrors the asm's 8-byte entry, indexed by fp->player. */
typedef struct ITRemixStopwatchTimer
{
    u8 delay_timer;
    u8 active_frames;
    u8 total_frames;
    u8 unused;
    s16 remaining_time;
    s16 speed_type;
} ITRemixStopwatchTimer;

/* Backfire speed table. Each entry is a game speed value, one byte each. */
static const u8 sITRemixStopwatchBackfireSpeeds[ITREMIX_STOPWATCH_BACKFIRE_TABLE_SIZE] =
{
    1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2,
    3, 3,
    10, 10,
    11, 11, 11, 11, 11
};

/* Fast 'active' and 'total' frame pairs, two bytes per entry. */
static const u8 sITRemixStopwatchFastFrames[ITREMIX_STOPWATCH_FAST_TABLE_SIZE * 2] =
{
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2
};

/* Slow 'active' and 'total' frame pairs, two bytes per entry. */
static const u8 sITRemixStopwatchSlowFrames[ITREMIX_STOPWATCH_SLOW_TABLE_SIZE * 2] =
{
    1, 3, 1, 3,
    1, 2, 1, 2, 1, 2, 1, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2
};

static ITRemixStopwatchTimer sITRemixStopwatchTimers[ITREMIX_STOPWATCH_MAX_PORTS];
static const ITRemixStopwatchTimer sITRemixStopwatchNoTimer = { 0 };
static GObj *sITRemixStopwatchWatchGObj;
static s32 sITRemixStopwatchEngineTimer;
static u8 sITRemixStopwatchWatchSpeed;
static sb32 sITRemixStopwatchIsEndSfxPlaying;
static sb32 sITRemixStopwatchIsPlayerAffected;
static sb32 sITRemixStopwatchIsFastRoll;
static sb32 sITRemixStopwatchFunctionAbbr;
static s16 sITRemixStopwatchProjectileLimit[2];
static u32 sITRemixStopwatchSavedMotionVars[3];
static u8 sITRemixStopwatchSavedHitboxFlags;
static s32 sITRemixStopwatchSavedKinetic;

/* Stopwatch: action ids of a Revive state. Affected players are skipped. */
static sb32 itRemixStopwatchIsReviving(FTStruct *fp)
{
    s32 status = fp->status_id;

    return (status == ITREMIX_STOPWATCH_ACTION_REVIVE1) || (status == ITREMIX_STOPWATCH_ACTION_REVIVE2)
        || (status == ITREMIX_STOPWATCH_ACTION_REVIVE_WAIT);
}

/* Stopwatch: the death and KO states. The per-frame countdown ends a timer on these. */
static sb32 itRemixStopwatchIsJustDied(FTStruct *fp)
{
    return fp->status_id < ITREMIX_STOPWATCH_DEATH_STATUS_LIMIT;
}

/* Stopwatch: clock_out_ exemptions. Revive, spawn invulnerability and Super Star skip the player. */
static sb32 itRemixStopwatchCanBeAffected(FTStruct *fp)
{
    if (itRemixStopwatchIsReviving(fp))
    {
        return FALSE;
    }
    if (fp->invincible_tics != 0)
    {
        return FALSE;
    }
    if (fp->star_invincible_tics != 0)
    {
        return FALSE;
    }
    return TRUE;
}

/* Stopwatch: team mates are players on the owner's team in a team battle. */
static sb32 itRemixStopwatchIsTeamMate(FTStruct *owner_fp, FTStruct *fp)
{
    if (gSCManagerBattleState->is_team_battle == FALSE)
    {
        return FALSE;
    }
    return fp->team == owner_fp->team;
}

/* Stopwatch: behaviour toggle (Toggles.asm entry_stopwatch_behaviour), read by pickup_stopwatch and clock_out_. */
static s32 itRemixStopwatchGetBehaviour(void)
{
#ifdef REGION_REMIX
    return gmRemixToggle(stopwatch_behaviour);
#else
    return ITREMIX_STOPWATCH_BEHAVIOUR_DEFAULT;
#endif
}

/* Stopwatch: pickup_stopwatch backfire roll. The roll is always drawn, as in the asm. */
static sb32 itRemixStopwatchRollBackfire(void)
{
    s32 behaviour = itRemixStopwatchGetBehaviour();
    s32 roll = syUtilsRandIntRange(ITREMIX_STOPWATCH_ROLL_RANGE);

    if (behaviour == ITREMIX_STOPWATCH_BEHAVIOUR_BACKFIRE_ALWAYS)
    {
        return TRUE;
    }
    if (behaviour != ITREMIX_STOPWATCH_BEHAVIOUR_DEFAULT)
    {
        return FALSE;
    }
    return roll < ITREMIX_STOPWATCH_BACKFIRE_ODDS;
}

/* Stopwatch: clock_out_ roll. TRUE means fast (speed-up), FALSE means slow. */
static sb32 itRemixStopwatchRollFastOrSlow(void)
{
    s32 behaviour = itRemixStopwatchGetBehaviour();
    s32 roll;

    if (behaviour == ITREMIX_STOPWATCH_BEHAVIOUR_SLOW_ALWAYS)
    {
        return FALSE;
    }
    if (behaviour == ITREMIX_STOPWATCH_BEHAVIOUR_FAST_ALWAYS)
    {
        return TRUE;
    }
    roll = syUtilsRandIntRange(ITREMIX_STOPWATCH_ROLL_RANGE);

    return roll < ITREMIX_STOPWATCH_SPEEDUP_ODDS;
}

/* Stopwatch: backfire duration for a speed. Bands are below 4, below 7, below 10, below 11, then the rest. */
static s32 itRemixStopwatchBackfireDuration(u8 speed)
{
    if (speed < 4)
    {
        return 11 * 60;
    }
    if (speed < 7)
    {
        return 17 * 60;
    }
    if (speed < 10)
    {
        return 210;
    }
    if (speed < 11)
    {
        return 5 * 60;
    }
    return 6 * 60;
}

/* Stopwatch: stores a frame pair, the duration and the speed type for one player. */
static void itRemixStopwatchSetTimer(FTStruct *fp, const u8 *frames, s32 duration, s32 speed_type)
{
    ITRemixStopwatchTimer *timer = &sITRemixStopwatchTimers[fp->player];

    timer->delay_timer = frames[0];
    timer->active_frames = frames[0];
    timer->total_frames = frames[1];
    timer->remaining_time = duration;
    timer->speed_type = speed_type;
}

/* Stopwatch: _speedup_random_speed. A fast frame pair picked at random, for a 10 second timer. */
static void itRemixStopwatchApplySpeedUp(FTStruct *fp)
{
    s32 index = syUtilsRandIntRange(ITREMIX_STOPWATCH_FAST_TABLE_SIZE);

    itRemixStopwatchSetTimer(fp, &sITRemixStopwatchFastFrames[index * 2], ITREMIX_STOPWATCH_SPEEDUP_DURATION, ITREMIX_STOPWATCH_SPEED_FAST);
    sITRemixStopwatchIsPlayerAffected = TRUE;
    gmRemixColScriptsSetOverride(fp->player, ITREMIX_STOPWATCH_COLANIM_FASTED);
    ftParamCheckSetFighterColAnimID(fp->fighter_gobj, ITREMIX_STOPWATCH_COLANIM_FASTED, 0);
}

/* Stopwatch: the slow branch of clock_out_. A slow frame pair picked at random, for a 9 second timer. */
static void itRemixStopwatchApplySlowdown(FTStruct *fp)
{
    s32 index = syUtilsRandIntRange(ITREMIX_STOPWATCH_SLOW_TABLE_SIZE);

    itRemixStopwatchSetTimer(fp, &sITRemixStopwatchSlowFrames[index * 2], ITREMIX_STOPWATCH_SLOWDOWN_DURATION, ITREMIX_STOPWATCH_SPEED_SLOW);
    sITRemixStopwatchIsPlayerAffected = TRUE;
    gmRemixColScriptsSetOverride(fp->player, ITREMIX_STOPWATCH_COLANIM_SLOWED);
    ftParamCheckSetFighterColAnimID(fp->fighter_gobj, ITREMIX_STOPWATCH_COLANIM_SLOWED, 0);
}

/* Stopwatch: clock_out_ per-player rules. Owner speed-up, team mates, then everyone else. */
static void itRemixStopwatchClockOutPlayer(FTStruct *owner_fp, FTStruct *fp)
{
    if (fp == owner_fp)
    {
        if (sITRemixStopwatchIsFastRoll)
        {
            itRemixStopwatchApplySpeedUp(fp);
        }
        return;
    }
    if (itRemixStopwatchIsTeamMate(owner_fp, fp))
    {
        if (sITRemixStopwatchIsFastRoll)
        {
            itRemixStopwatchApplySpeedUp(fp);
            return;
        }
        if (gSCManagerBattleState->is_team_attack == FALSE)
        {
            return;
        }
    }
    if (itRemixStopwatchCanBeAffected(fp) == FALSE)
    {
        return;
    }
    if (sITRemixStopwatchIsFastRoll)
    {
        return;
    }
    itRemixStopwatchApplySlowdown(fp);
}

/* Stopwatch: clock_out_. Rolls fast or slow once, then applies it to every fighter. */
static void itRemixStopwatchClockOut(FTStruct *owner_fp)
{
    GObj *player_gobj;

    sITRemixStopwatchIsFastRoll = itRemixStopwatchRollFastOrSlow();
    for (player_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; player_gobj != NULL; player_gobj = player_gobj->link_next)
    {
        itRemixStopwatchClockOutPlayer(owner_fp, ftGetStruct(player_gobj));
    }
}

/* Stopwatch: backfire branch of pickup_stopwatch. Sets the game speed and the global engine timer. */
static void itRemixStopwatchStartBackfire(void)
{
    s32 index = syUtilsRandIntRange(ITREMIX_STOPWATCH_BACKFIRE_TABLE_SIZE);
    u8 speed = sITRemixStopwatchBackfireSpeeds[index];

    sITRemixStopwatchIsPlayerAffected = TRUE;
    sITRemixStopwatchWatchSpeed = speed;
    sITRemixStopwatchEngineTimer = itRemixStopwatchBackfireDuration(speed);
    /* REMIX-TODO(Render): backfire BG rectangle is not drawn (Render.draw_rectangle, colour 0xA1333399 under speed 7, else 0x0066AA77) */
}

/* Stopwatch: pickup_stopwatch _end. Plays the backfire or normal pickup cue when a player was affected. */
static void itRemixStopwatchPlayPickupSfx(sb32 is_backfire)
{
    if (sITRemixStopwatchIsPlayerAffected == FALSE)
    {
        return;
    }
    sITRemixStopwatchIsPlayerAffected = FALSE;
    if (is_backfire)
    {
        func_800269C0_275C0(ITREMIX_STOPWATCH_FGM_BACKFIRE_TIMER);
        return;
    }
    func_800269C0_275C0(ITREMIX_STOPWATCH_FGM_TIMER);
}

/* Stopwatch: clears every per-player timer. */
static void itRemixStopwatchClearTimers(void)
{
    s32 port;

    for (port = 0; port < ITREMIX_STOPWATCH_MAX_PORTS; port++)
    {
        sITRemixStopwatchTimers[port] = sITRemixStopwatchNoTimer;
    }
}

/* Stopwatch: _destroy_watch. Ends the per-frame handler and resets every timer. */
static void itRemixStopwatchDestroy(GObj *watch_gobj)
{
    sITRemixStopwatchWatchGObj = NULL;
    itRemixStopwatchClearTimers();
    gcEjectGObj(watch_gobj);
    /* REMIX-TODO(Render): the backfire BG rectangle is not freed (Render.DESTROY_OBJECT_) */
}

/* Stopwatch: _play_end_sfx and _generate_white_circle_gfx. End cue once, then a circle at the player's ECB top. */
static void itRemixStopwatchSpawnEndCircle(FTStruct *fp)
{
    Vec3f pos = *fp->coll_data.p_translate;
    DObj *topjoint = fp->joints[0];
    s32 size;

    pos.y += fp->attr->map_coll.center;
    size = (s32)(topjoint->scale.vec.f.x * ITREMIX_STOPWATCH_CIRCLE_SIZE);
    efManagerSetOffMakeEffect(&pos, size);
}

/* Stopwatch: _remove_gfx_routine. Restores the default GFX routine when the player's override is a Stopwatch routine. */
static void itRemixStopwatchRemoveGfx(FTStruct *fp)
{
    s32 colanim_id = gmRemixColScriptsGetOverride(fp->player);

    if (colanim_id != ITREMIX_STOPWATCH_COLANIM_SLOWED && colanim_id != ITREMIX_STOPWATCH_COLANIM_FASTED)
    {
        return;
    }
    gmRemixColScriptsSetOverride(fp->player, 0);
    ftParamResetStatUpdateColAnim(fp->fighter_gobj);
}

/* Stopwatch: _timer_exhausted. Clears the player's timer, then plays the end cue and circle for living players. */
static void itRemixStopwatchExpire(FTStruct *fp, ITRemixStopwatchTimer *timer)
{
    timer->delay_timer = 0;
    timer->active_frames = 0;
    timer->total_frames = 0;
    timer->speed_type = ITREMIX_STOPWATCH_SPEED_SLOW;
    itRemixStopwatchRemoveGfx(fp);
    if (itRemixStopwatchIsJustDied(fp))
    {
        return;
    }
    if (sITRemixStopwatchIsEndSfxPlaying == FALSE)
    {
        sITRemixStopwatchIsEndSfxPlaying = TRUE;
        func_800269C0_275C0(ITREMIX_STOPWATCH_FGM_TIMER_END);
    }
    itRemixStopwatchSpawnEndCircle(fp);
}

/* Stopwatch: the delay counter. Restarts at the 'total' frame count after it reaches zero. */
static void itRemixStopwatchTickDelay(ITRemixStopwatchTimer *timer)
{
    if (timer->delay_timer == 0)
    {
        timer->delay_timer = timer->total_frames;
        return;
    }
    timer->delay_timer -= 1;
}

/* Stopwatch: per-player timer in handle_active_stopwatch_. Returns TRUE while the timer keeps counting. */
static sb32 itRemixStopwatchTickPlayer(FTStruct *fp)
{
    ITRemixStopwatchTimer *timer = &sITRemixStopwatchTimers[fp->player];

    if (timer->remaining_time == 0)
    {
        return FALSE;
    }
    timer->remaining_time = itRemixStopwatchIsJustDied(fp) ? 0 : timer->remaining_time - 1;
    if (timer->remaining_time == 0)
    {
        itRemixStopwatchExpire(fp, timer);
        return FALSE;
    }
    itRemixStopwatchTickDelay(timer);

    return TRUE;
}

/* Stopwatch: global engine timer (_subtract_timer_2). Returns TRUE when it was running this frame. */
static sb32 itRemixStopwatchTickEngine(void)
{
    if (sITRemixStopwatchEngineTimer == 0)
    {
        return FALSE;
    }
    /* REMIX-TODO(Render): the BG rectangle fade near the end (alpha steps every 4 frames under 240) is not drawn */
    sITRemixStopwatchEngineTimer -= 1;
    if (sITRemixStopwatchEngineTimer != 0)
    {
        return TRUE;
    }
    sITRemixStopwatchWatchSpeed = 0;
    /* REMIX-TODO(Render): the BG rectangle is not made transparent */
    if (sITRemixStopwatchIsEndSfxPlaying == FALSE)
    {
        func_800269C0_275C0(ITREMIX_STOPWATCH_FGM_BACKFIRE_TIMER_END);
    }
    return TRUE;
}

/* Stopwatch: handle_active_stopwatch_. Per-frame countdown for each fighter and the engine timer. */
static void itRemixStopwatchHandleActive(GObj *watch_gobj)
{
    GObj *player_gobj;
    sb32 is_timer_active = FALSE;

    sITRemixStopwatchIsEndSfxPlaying = FALSE;
    for (player_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; player_gobj != NULL; player_gobj = player_gobj->link_next)
    {
        if (itRemixStopwatchTickPlayer(ftGetStruct(player_gobj)))
        {
            is_timer_active = TRUE;
        }
    }
    if (itRemixStopwatchTickEngine())
    {
        is_timer_active = TRUE;
    }
    if (is_timer_active)
    {
        return;
    }
    itRemixStopwatchDestroy(watch_gobj);
}

/* Stopwatch: _check_for_active_players. Registers the per-frame handler, once, while no watch exists. */
static void itRemixStopwatchActivate(void)
{
    if (sITRemixStopwatchWatchGObj != NULL)
    {
        return;
    }
    sITRemixStopwatchWatchGObj = gcMakeGObjSPAfter(nGCCommonKindEffect, itRemixStopwatchHandleActive, nGCCommonLinkIDEffect, GOBJ_PRIORITY_DEFAULT);
}

/* Stopwatch: pickup_stopwatch. PICKUP_ITEM_MAIN, then the shared tomato tail at 0x80145C4C. */
static void itRemixStopwatchPickupProc(FTStruct *fp, GObj *item_gobj)
{
    sb32 is_backfire = itRemixStopwatchRollBackfire();

    if (is_backfire)
    {
        itRemixStopwatchStartBackfire();
    }
    else
    {
        itRemixStopwatchClockOut(fp);
    }
    itRemixStopwatchActivate();
    itRemixStopwatchPlayPickupSfx(is_backfire);
    itRemixConsumeTomatoTail(fp, item_gobj);
}

/* Stopwatch: byte 0x18C of the fighter state, rebuilt from its bitfields in N64 bit order (bit 0 is the MSB). */
static u8 itRemixStopwatchHitboxFlagsByte(FTStruct *fp)
{
    u8 flags = 0;

    flags |= (u8)fp->is_attack_active << 7;
    flags |= (u8)fp->is_hitstatus_nodamage << 6;
    flags |= (u8)fp->is_damage_coll_modify << 5;
    flags |= (u8)fp->is_modelpart_modify << 4;
    flags |= (u8)fp->is_texturepart_modify << 3;
    flags |= (u8)fp->is_reflect << 2;
    flags |= (u8)(fp->reflect_lr & 0x3);

    return flags;
}

/* Stopwatch: state snapshot taken before a fast player's first update. */
static void itRemixStopwatchSaveState(FTStruct *fp)
{
    sITRemixStopwatchSavedMotionVars[0] = fp->motion_vars.flags.flag0;
    sITRemixStopwatchSavedMotionVars[1] = fp->motion_vars.flags.flag1;
    sITRemixStopwatchSavedMotionVars[2] = fp->motion_vars.flags.flag2;
    sITRemixStopwatchSavedHitboxFlags = itRemixStopwatchHitboxFlagsByte(fp);
    sITRemixStopwatchSavedKinetic = fp->ga;
}

/* Stopwatch: any change to the snapshot means the second update must be skipped. */
static sb32 itRemixStopwatchHasStateChanged(FTStruct *fp)
{
    if (itRemixStopwatchHitboxFlagsByte(fp) != sITRemixStopwatchSavedHitboxFlags)
    {
        return TRUE;
    }
    if (fp->ga != sITRemixStopwatchSavedKinetic)
    {
        return TRUE;
    }
    if (fp->motion_vars.flags.flag0 != sITRemixStopwatchSavedMotionVars[0])
    {
        return TRUE;
    }
    if (fp->motion_vars.flags.flag1 != sITRemixStopwatchSavedMotionVars[1])
    {
        return TRUE;
    }
    return fp->motion_vars.flags.flag2 != sITRemixStopwatchSavedMotionVars[2];
}

/* Stopwatch: _run_fast for a fighter. Runs the update here; the hook runs it a second time unless the state changed. */
static s32 itRemixStopwatchRunTwice(GObj *player_gobj, void (*update_proc)(GObj *gobj))
{
    FTStruct *fp = ftGetStruct(player_gobj);

    itRemixStopwatchSaveState(fp);
    update_proc(player_gobj);
    if (itRemixStopwatchHasStateChanged(fp))
    {
        return ITREMIX_STOPWATCH_CHECK_SKIP;
    }
    return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
}

/* Stopwatch: _timer_check (slowdown). Skips every frame outside the active range. Physics is frozen with function_abbr. */
static s32 itRemixStopwatchCheckSlow(void (*update_proc)(GObj *gobj), ITRemixStopwatchTimer *timer)
{
    if (sITRemixStopwatchProjectileLimit[0] != 0)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    if (timer->delay_timer < timer->active_frames)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    if (update_proc != ftMainProcPhysicsMapDefault)
    {
        return ITREMIX_STOPWATCH_CHECK_SKIP;
    }
    sITRemixStopwatchFunctionAbbr = TRUE;

    return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
}

/* Stopwatch: _timer_check_fast (speed-up). Runs the update twice outside the active range. Projectiles run once. */
static s32 itRemixStopwatchCheckFast(GObj *gobj, void (*update_proc)(GObj *gobj), ITRemixStopwatchTimer *timer)
{
    if (sITRemixStopwatchProjectileLimit[1] != 0)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    if (timer->delay_timer < timer->active_frames)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    if (update_proc == wpProcessProcWeaponMain)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_TWICE;
    }
    return itRemixStopwatchRunTwice(gobj, update_proc);
}

/* Stopwatch: _check_spd_type. Picks the slow or fast rule from the timer's speed type. */
static s32 itRemixStopwatchCheckSpeedType(GObj *gobj, void (*update_proc)(GObj *gobj), ITRemixStopwatchTimer *timer)
{
    if (timer->speed_type == ITREMIX_STOPWATCH_SPEED_FAST)
    {
        return itRemixStopwatchCheckFast(gobj, update_proc, timer);
    }
    return itRemixStopwatchCheckSlow(update_proc, timer);
}

/* Stopwatch: PK Thunder actions of Ness, Jeff and Lucas, and the Crash DSP dig start, run at normal speed. */
static sb32 itRemixStopwatchIsExemptAction(FTStruct *fp)
{
    s32 kind = fp->fkind;
    s32 status = fp->status_id;

    if (kind == REMIX_ID_CRASH)
    {
        return status == ITREMIX_STOPWATCH_ACTION_CRASH_DIGGIN_BEGIN;
    }
    if ((kind != REMIX_ID_NESS) && (kind != REMIX_ID_JNESS) && (kind != REMIX_ID_LUCAS))
    {
        return FALSE;
    }
    return (status == ITREMIX_STOPWATCH_ACTION_PK_THUNDER_START1) || (status == ITREMIX_STOPWATCH_ACTION_PK_THUNDER_START2)
        || (status == ITREMIX_STOPWATCH_ACTION_PK_THUNDER_START_AIR) || (status == ITREMIX_STOPWATCH_ACTION_PK_THUNDER_AIR);
}

/* Stopwatch: _player_check. Fighter update or physics proc, with the exemptions and the timer rule. */
static s32 itRemixStopwatchCheckPlayer(GObj *player_gobj, void (*update_proc)(GObj *gobj))
{
    FTStruct *fp = ftGetStruct(player_gobj);
    ITRemixStopwatchTimer *timer = &sITRemixStopwatchTimers[fp->player];

    if (timer->remaining_time == 0)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    if (itRemixStopwatchIsExemptAction(fp))
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    if (fp->capture_gobj != NULL)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    return itRemixStopwatchCheckSpeedType(player_gobj, update_proc, timer);
}

/* Stopwatch: _player_check_projectile. Projectiles never run fast. The owner's timer decides the rest. */
static s32 itRemixStopwatchCheckProjectile(GObj *projectile_gobj, void (*update_proc)(GObj *gobj))
{
    WPStruct *wp = wpGetStruct(projectile_gobj);
    FTStruct *owner_fp;
    ITRemixStopwatchTimer *timer;

    if ((wp->kind == ITREMIX_STOPWATCH_PROJECTILE_PK_THUNDER_HEAD) || (wp->kind == ITREMIX_STOPWATCH_PROJECTILE_PK_THUNDER_TAIL))
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    sITRemixStopwatchProjectileLimit[1] = TRUE;
    if (wp->owner_gobj == NULL)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    owner_fp = ftGetStruct(wp->owner_gobj);
    timer = &sITRemixStopwatchTimers[owner_fp->player];
    if (timer->remaining_time == 0)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    return itRemixStopwatchCheckSpeedType(projectile_gobj, update_proc, timer);
}

/* Stopwatch: check_function_. Speed.asm calls this before each fighter or projectile update. */
s32 itRemixStopwatchCheckFunction(GObj *gobj, void (*update_proc)(GObj *gobj))
{
    if (sITRemixStopwatchWatchGObj == NULL)
    {
        return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
    }
    sITRemixStopwatchFunctionAbbr = FALSE;
    sITRemixStopwatchProjectileLimit[0] = 0;
    sITRemixStopwatchProjectileLimit[1] = 0;
    if ((update_proc == ftMainProcUpdateInterrupt) || (update_proc == ftMainProcPhysicsMapDefault))
    {
        return itRemixStopwatchCheckPlayer(gobj, update_proc);
    }
    if (update_proc == wpProcessProcWeaponMain)
    {
        return itRemixStopwatchCheckProjectile(gobj, update_proc);
    }
    return ITREMIX_STOPWATCH_CHECK_RUN_NORMAL;
}

/* Read by the vanilla hitlag patches, see the notes file. */
sb32 itRemixStopwatchIsFunctionAbbr(void)
{
    return sITRemixStopwatchFunctionAbbr;
}

/* Read by Speed.asm, see the notes file. */
u8 itRemixStopwatchGetWatchSpeed(void)
{
    return sITRemixStopwatchWatchSpeed;
}

/* Item.asm clear_active_stopwatch_. Scene-end reset of every Stopwatch flag, timer and the handler. */
void itRemixStopwatchClearActive(void)
{
    sITRemixStopwatchWatchGObj = NULL;
    sITRemixStopwatchEngineTimer = 0;
    itRemixStopwatchClearTimers();
    sITRemixStopwatchWatchSpeed = 0;
    sITRemixStopwatchIsFastRoll = FALSE;
    sITRemixStopwatchProjectileLimit[0] = 0;
    sITRemixStopwatchProjectileLimit[1] = 0;
    sITRemixStopwatchFunctionAbbr = FALSE;
    sITRemixStopwatchIsPlayerAffected = FALSE;
    sITRemixStopwatchIsEndSfxPlaying = FALSE;
    /* REMIX-TODO(Render): the backfire BG rectangle reference is not cleared */
}

static ITDesc sITRemixStopwatchItemInfo =
{
    ITREMIX_STOPWATCH_KIND,                 /* Item.Stopwatch.id */
    &gITManagerCommonData,                  /* item file 0xFB, see notes */
    ITREMIX_STOPWATCH_FILE_OFFSET,          /* item footer offset */

    {
        nGCMatrixKindTraRotRpyRSca,         /* item_info_array 0x0C = 0x1C000000 */
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itTomatoFallProcUpdate,                 /* 0x801744C0 */
    itTomatoFallProcMap,                    /* 0x80174524 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* Stopwatch: stage_setting_. Remix SPAWN_ITEM. Same as itTomatoMakeItem with the Stopwatch item info. */
GObj* itRemixStopwatchMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixStopwatchItemInfo, pos, vel, flags);
    DObj *joint;
    ITStruct *ip;

    if (item_gobj == NULL)
    {
        return NULL;
    }
    ip = itGetStruct(item_gobj);
    joint = DObjGetStruct(item_gobj);
    gcAddXObjForDObjFixed(joint, 0x2E, 0);
    joint->translate.vec.f = *pos;
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

const ITRemixItemDesc dITRemixStopwatchItemDesc =
{
    ITREMIX_STOPWATCH_KIND,
    itRemixStopwatchMakeItem,
    TRUE,
    NULL,                                   /* PICKUP_ITEM_INIT(0) */
    itRemixStopwatchPickupProc,             /* PICKUP_ITEM_MAIN */
    itTomatoDroppedSetStatus,               /* DROP_ITEM 0x801745FC */
    NULL,                                   /* THROW_ITEM(0) */
    NULL                                    /* PLAYER_COLLISION(0) */
};

#endif
