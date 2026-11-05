/**
 * sc1pbonusremix.c - Remix bonus stage tables, choice and records, see sc1pbonusremix.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <lb/library.h>
#include <lb/lbbackup.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <gr/grdef.h>
#include <sc/sctypes.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include "sc/sc1pmode/sc1pbonusremix.h"
#include "sc/sc1pmode/sc1pbonusremixrules.h"
#include "sc/sc1pmode/sc1premixbonus3.h"
#include "sc/sc1pmode/sc1premixbonus3rules.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmodename.h"

static s32 sSC1PBonusRemixStageIndex;

s32 sc1PBonusRemixGetStageIndex(void)
{
    return sSC1PBonusRemixStageIndex;
}

void sc1PBonusRemixSetStageIndex(s32 index)
{
    sSC1PBonusRemixStageIndex = (index < 0) ? 0 : ((index >= SC1PBONUS_REMIX_STAGE_COUNT) ? (SC1PBONUS_REMIX_STAGE_COUNT - 1) : index);
}

sb32 sc1PBonusRemixIsRemixChoice(void)
{
    return (sc1PRemixBonus3CourseGet() == SC1PREMIX_BONUS3_COURSE_REMIX) ? TRUE : FALSE;
}

static u32 sc1PBonusRemixGetStageTable(sb32 is_platforms)
{
    return (is_platforms != FALSE) ? REMIX_SYM_Bonus__btp_stage_table : REMIX_SYM_Bonus__btt_stage_table;
}

s32 sc1PBonusRemixGetStageId(sb32 is_platforms, s32 index)
{
    if ((index < 0) || (index >= SC1PBONUS_REMIX_STAGE_COUNT))
    {
        return SC1PBONUS_REMIX_NO_STAGE;
    }
    return (s32)remix_ram_image_read_byte(sc1PBonusRemixGetStageTable(is_platforms) + index);
}

s32 sc1PBonusRemixFindStageIndex(sb32 is_platforms, s32 stage_id)
{
    unsigned char stages[SC1PBONUS_REMIX_STAGE_COUNT];
    s32 index;

    for (index = 0; index < SC1PBONUS_REMIX_STAGE_COUNT; index++)
    {
        stages[index] = (unsigned char)sc1PBonusRemixGetStageId(is_platforms, index);
    }
    return sc1PBonusRemixStageIndexOf(stages, SC1PBONUS_REMIX_STAGE_COUNT, stage_id);
}

s32 sc1PBonusRemixGetDefaultStageId(sb32 is_platforms, s32 fkind)
{
    u32 table = (is_platforms != FALSE) ? REMIX_SYM_Character__BTP_TABLE : REMIX_SYM_Character__BTT_TABLE;

    return (s32)remix_ram_image_read_byte(table + fkind);
}

void sc1PBonusRemixGetStageIcons(s32 index, s32 *first, s32 *second)
{
    u32 icons = REMIX_SYM_Bonus__stock_icon_table + (index * 2);

    *first = (s32)remix_ram_image_read_byte(icons);
    *second = (s32)remix_ram_image_read_byte(icons + 1);
}

u32 sc1PBonusRemixGetIconOffset(s32 fkind)
{
    return remix_ram_image_read_word(REMIX_SYM_SinglePlayerModes__icon_offset_table + (fkind * sizeof(u32)));
}

s32 sc1PBonusRemixGetBattleGkind(sb32 is_platforms, s32 fkind)
{
    s32 source = sc1PBonusRemixStageSource((sc1PRemixModeIs(SC1PREMIX_MODE_REMIX_1P) != FALSE) ? 1 : 0, (sc1PBonusRemixIsRemixChoice() != FALSE) ? 1 : 0, fkind, REMIX_ID_BOSS);
    u32 remix_1p_table = (is_platforms != FALSE) ? REMIX_SYM_Character__REMIX_BTP_TABLE : REMIX_SYM_Character__REMIX_BTT_TABLE;
    s32 stage_id;

    switch (source)
    {
    case SC1PBONUS_REMIX_SOURCE_REMIX_1P_TABLE:
        stage_id = (s32)remix_ram_image_read_byte(remix_1p_table + fkind);
        break;

    case SC1PBONUS_REMIX_SOURCE_SELECTED:
        stage_id = sc1PBonusRemixGetStageId(is_platforms, sSC1PBonusRemixStageIndex);
        break;

    case SC1PBONUS_REMIX_SOURCE_DEFAULT_TABLE:
        stage_id = sc1PBonusRemixGetDefaultStageId(is_platforms, fkind);
        break;

    default:
        return -1;
    }
    return (stage_id == SC1PBONUS_REMIX_NO_STAGE) ? -1 : stage_id;
}

sb32 sc1PBonusRemixIsPlatformStage(s32 gkind)
{
    sb32 is_vanilla_stage = ((gkind >= nGRKindBonus1Start) && (gkind <= nGRKindBonus2End)) ? TRUE : FALSE;
    sb32 is_vanilla_platforms = (gkind >= nGRKindBonus2Start) ? TRUE : FALSE;
    s32 stage_class = (s32)remix_ram_image_read_byte(REMIX_SYM_Stages__class_table + gkind);

    return (sc1PBonusRemixRulesIsPlatformStage(is_vanilla_platforms, is_vanilla_stage, stage_class) != 0) ? TRUE : FALSE;
}

sb32 sc1PBonusRemixGetStageWords(s32 gkind, u32 *start, u32 *dobjdesc, u32 *anim_joint)
{
    u32 words = remix_ram_image_read_word(REMIX_SYM_Stages__bonus_pointer_table + (gkind * sizeof(u32)));

    if (words == 0)
    {
        return FALSE;
    }
    *start = remix_ram_image_read_word(words);
    *dobjdesc = remix_ram_image_read_word(words + 4);
    *anim_joint = remix_ram_image_read_word(words + 8);

    return TRUE;
}

static LBBackupBonusRecord* sc1PBonusRemixGetRecord(sb32 is_platforms, s32 stage_index, s32 fkind)
{
    if ((stage_index < 0) || (stage_index >= SC1PBONUS_REMIX_STAGE_COUNT) || (fkind < 0) || (fkind >= GMCOMMON_FIGHTERS_SAVE_NUM))
    {
        return NULL;
    }
    return &gSCManagerBackupData.remix_bonus[stage_index][fkind][(is_platforms != FALSE) ? 1 : 0];
}

sb32 sc1PBonusRemixUsesRecordTable(sb32 is_platforms, s32 fkind)
{
    if ((sc1PRemixModeGet() != SC1PREMIX_MODE_STANDARD) || (sc1PBonusRemixIsRemixChoice() == FALSE))
    {
        return FALSE;
    }
    return (sc1PBonusRemixIsDefaultStage(sc1PBonusRemixGetStageId(is_platforms, sSC1PBonusRemixStageIndex), sc1PBonusRemixGetDefaultStageId(is_platforms, fkind)) == 0) ? TRUE : FALSE;
}

s32 sc1PBonusRemixGetRecordCount(sb32 is_platforms, s32 fkind)
{
    LBBackupBonusRecord *record = sc1PBonusRemixGetRecord(is_platforms, sSC1PBonusRemixStageIndex, fkind);

    return (record != NULL) ? record->count : 0;
}

u32 sc1PBonusRemixGetRecordTime(sb32 is_platforms, s32 fkind)
{
    LBBackupBonusRecord *record = sc1PBonusRemixGetRecord(is_platforms, sSC1PBonusRemixStageIndex, fkind);

    return (record != NULL) ? sc1PBonusRemixRecordTime(record->count, record->time) : SC1PBONUS_REMIX_TIME_UNSET;
}

sb32 sc1PBonusRemixIsRecordComplete(sb32 is_platforms, s32 fkind)
{
    return (sc1PBonusRemixGetRecordCount(is_platforms, fkind) >= SC1PBONUS_REMIX_TASK_MAX) ? TRUE : FALSE;
}

sb32 sc1PBonusRemixTrySave(sb32 is_tasks_fail, s32 fkind, s32 tasks_complete, u32 time_passed)
{
    s32 gkind = gSCManagerBattleState->gkind;
    sb32 is_platforms = sc1PBonusRemixIsPlatformStage(gkind);
    LBBackupBonusRecord *record;
    s32 count;

    if (sc1PBonusRemixIsDefaultStage(gkind, sc1PBonusRemixGetDefaultStageId(is_platforms, fkind)) != 0)
    {
        return FALSE;
    }
    record = sc1PBonusRemixGetRecord(is_platforms, sc1PBonusRemixFindStageIndex(is_platforms, gkind), fkind);

    if (record == NULL)
    {
        return TRUE;
    }
    if (is_tasks_fail != FALSE)
    {
        count = sc1PBonusRemixCountAfterRun(record->count, tasks_complete);

        if (count != record->count)
        {
            record->count = (u8)count;
            lbBackupWrite();
        }
        return TRUE;
    }
    if (sc1PBonusRemixIsTimeBeaten(record->count, record->time, time_passed) != 0)
    {
        record->count = SC1PBONUS_REMIX_TASK_MAX;
        record->time = time_passed;
        lbBackupWrite();
    }
    return TRUE;
}

#endif
