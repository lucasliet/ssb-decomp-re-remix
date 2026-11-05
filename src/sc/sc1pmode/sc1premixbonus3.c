/**
 * sc1premixbonus3.c - Bonus 3 mode rules on the 1P game, see sc1premixbonus3.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmsound.h>
#include <lb/library.h>
#include <lb/lbbackup.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include "gr/grremix/grremixids.h"
#include "remix/remix_debug.h"
#include "sc/sc1pmode/sc1premixbonus3.h"
#include "sc/sc1pmode/sc1premixbonus3rules.h"
#include "sc/sc1pmode/sc1premixbonusexit.h"
#include "sc/sc1pmode/sc1premixhighscores.h"
#include "sc/sc1pmode/sc1premixmode.h"

extern int strcmp(const char *left, const char *right);

static s32 sSC1PRemixBonus3Course = SC1PREMIX_BONUS3_COURSE_NORMAL;
static sb32 sSC1PRemixBonus3IsFinished;
static u16 sSC1PRemixBonus3FinishSfx;

sb32 sc1PRemixBonus3IsActive(void)
{
    if (sc1PRemixModeIs(SC1PREMIX_MODE_BONUS3) == FALSE)
    {
        return FALSE;
    }
    return (gSCManagerSceneData.spgame_stage == nSC1PGameStageBonus3) ? TRUE : FALSE;
}

void sc1PRemixBonus3SetCourse(s32 course)
{
    sSC1PRemixBonus3Course = course;
}

static s32 sc1PRemixBonus3GetCourse(void)
{
    const char *value = remix_debug_env_text("SSB64_REMIX_BONUS3_COURSE");

    if (value == NULL)
    {
        return sSC1PRemixBonus3Course;
    }
    if (strcmp(value, "remix") == 0)
    {
        return SC1PREMIX_BONUS3_COURSE_REMIX;
    }
    if (strcmp(value, "normal") == 0)
    {
        return SC1PREMIX_BONUS3_COURSE_NORMAL;
    }
    port_log("SSB64_REMIX_BONUS3_COURSE=%s is not a course (normal, remix)\n", value);

    return sSC1PRemixBonus3Course;
}

void sc1PRemixBonus3SetupBattle(void)
{
    if (sc1PRemixBonus3IsActive() == FALSE)
    {
        return;
    }
    sSC1PRemixBonus3IsFinished = FALSE;

    if (sc1PRemixBonus3GetCourse() == SC1PREMIX_BONUS3_COURSE_REMIX)
    {
        gSCManagerBattleState->gkind = nGRRemixStage_REMIX_RTTF;
    }
    gSCManagerBattleState->is_team_attack = TRUE;
    gSCManagerBattleState->item_appearance_rate = 0;
    gSCManagerBattleState->time_limit = SCBATTLE_TIMELIMIT_INFINITE;
}

sb32 sc1PRemixBonus3UsesRemixPolygons(void)
{
    if (sc1PRemixBonus3IsActive() == FALSE)
    {
        return FALSE;
    }
    return (sc1PRemixBonus3GetCourse() == SC1PREMIX_BONUS3_COURSE_REMIX) ? TRUE : FALSE;
}

void sc1PRemixBonus3SetupPlayer(s32 player)
{
    if (sc1PRemixBonus3IsActive() == FALSE)
    {
        return;
    }
    gSCManager1PGameBattleState.players[player].stock_count = SC1PREMIX_BONUS3_PLAYER_STOCKS;
}

s32 sc1PRemixBonus3EnemyLevel(s32 level)
{
    return (sc1PRemixBonus3IsActive() != FALSE) ? SC1PREMIX_BONUS3_ENEMY_LEVEL : level;
}

void sc1PRemixBonus3SetupEnemy(s32 player)
{
    if (sc1PRemixBonus3IsActive() == FALSE)
    {
        return;
    }
    gSCManagerBattleState->players[player].handicap = SC1PREMIX_BONUS3_ENEMY_HANDICAP;
}

static u32* sc1PRemixBonus3RecordSlot(s32 fkind)
{
    LBBackup1PRecord *record;

    if ((fkind < 0) || (fkind >= ARRAY_COUNT(gSCManagerBackupData.spgame_records)))
    {
        return NULL;
    }
    record = &gSCManagerBackupData.spgame_records[fkind];

    return (sc1PRemixBonus3GetCourse() == SC1PREMIX_BONUS3_COURSE_REMIX) ? &record->bonus3_remix_time : &record->bonus3_time;
}

u32 sc1PRemixBonus3RecordGet(s32 fkind)
{
    u32 *slot = sc1PRemixBonus3RecordSlot(fkind);

    return (slot != NULL) ? *slot : 0;
}

s32 sc1PRemixBonus3CourseGet(void)
{
    return sc1PRemixBonus3GetCourse();
}

/** _time_save: keeps the time when the fighter has no record or it is faster; returns the verdict for the announcer. */
static s32 sc1PRemixBonus3SaveRecord(s32 fkind, u32 finish_time)
{
    u32 *slot = sc1PRemixBonus3RecordSlot(fkind);
    s32 verdict;

    if ((slot == NULL) || (sc1PRemixHighScoresCanSaveRecord() == FALSE))
    {
        return SC1PREMIX_BONUS3_RECORD_KEEP;
    }
    verdict = sc1PRemixBonus3RecordVerdict(*slot, finish_time);

    if (verdict == SC1PREMIX_BONUS3_RECORD_KEEP)
    {
        return verdict;
    }
    *slot = finish_time;
    lbBackupWrite();

    port_log("bonus3: record saved, fkind=%d time=%u beaten=%d\n", (int)fkind, (unsigned int)finish_time, (int)(verdict == SC1PREMIX_BONUS3_RECORD_BEATEN));

    return verdict;
}

u16 sc1PRemixBonus3OnComplete(u16 sfx_id)
{
    s32 verdict;

    if (sc1PRemixBonus3IsActive() == FALSE)
    {
        return sfx_id;
    }
    if (sSC1PRemixBonus3IsFinished != FALSE)
    {
        return sSC1PRemixBonus3FinishSfx;
    }
    verdict = sc1PRemixBonus3SaveRecord(gSCManagerBattleState->players[gSCManagerSceneData.player].fkind, gSCManagerBattleState->time_passed);

    sSC1PRemixBonus3IsFinished = TRUE;
    sSC1PRemixBonus3FinishSfx = (u16)sc1PRemixBonus3CompleteFgm(verdict, sfx_id, nSYAudioVoiceAnnounceNewRecord);

    return sSC1PRemixBonus3FinishSfx;
}

void sc1PRemixBonus3LeaveRun(void)
{
    sb32 is_restart = sc1PRemixModeLeaveRun();

    port_log("bonus3: run ended, restart=%d\n", (int)is_restart);
}

#endif
