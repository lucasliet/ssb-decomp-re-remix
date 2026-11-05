/**
 * sc1premixscore.c - Remix 1P and All-Star score tables, see sc1premixscore.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <lb/library.h>
#include <lb/lbbackup.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixscore.h"
#include "sc/sc1pmode/sc1premixscorerules.h"

static LBBackup1PScore* sc1PRemixScoreGetTable(s32 fkind)
{
    LBBackup1PRecord *record;

    if ((fkind < 0) || (fkind >= ARRAY_COUNT(gSCManagerBackupData.spgame_records)))
    {
        return NULL;
    }
    record = &gSCManagerBackupData.spgame_records[fkind];

    return (sc1PRemixScoreTableOf(sc1PRemixModeGet()) == SC1PREMIX_SCORE_TABLE_ALLSTAR) ? &record->allstar : &record->remix_1p;
}

sb32 sc1PRemixScoreUsesOwnTable(void)
{
    return (sc1PRemixScoreTableOf(sc1PRemixModeGet()) != SC1PREMIX_SCORE_TABLE_STANDARD) ? TRUE : FALSE;
}

u32 sc1PRemixScoreGetHiScore(s32 fkind)
{
    LBBackup1PScore *score = sc1PRemixScoreGetTable(fkind);

    return (score != NULL) ? score->hiscore : 0;
}

u32 sc1PRemixScoreGetBonusCount(s32 fkind)
{
    LBBackup1PScore *score = sc1PRemixScoreGetTable(fkind);

    return (score != NULL) ? score->total_bonuses : 0;
}

s32 sc1PRemixScoreGetBestDifficulty(s32 fkind)
{
    LBBackup1PScore *score = sc1PRemixScoreGetTable(fkind);

    return (score != NULL) ? score->best_difficulty : 0;
}

void sc1PRemixScoreTrySave(sb32 is_complete_spgame)
{
    LBBackup1PScore *score = sc1PRemixScoreGetTable(gSCManagerSceneData.fkind);
    sb32 is_write_data = FALSE;

    if (score == NULL)
    {
        return;
    }
    if (sc1PRemixScoreReplaces(score->hiscore, gSCManagerSceneData.spgame_score) != 0)
    {
        score->hiscore = gSCManagerSceneData.spgame_score;
        score->continues = gSCManagerSceneData.continues_used;
        score->total_bonuses = gSCManagerSceneData.bonus_count;
        score->best_difficulty = (is_complete_spgame != FALSE) ? (gSCManagerBackupData.spgame_difficulty + 1) : 0;
        is_write_data = TRUE;
    }
    if ((score->is_complete == FALSE) && (is_complete_spgame != FALSE))
    {
        score->is_complete = TRUE;
        is_write_data = TRUE;
    }
    if (is_write_data != FALSE)
    {
        lbBackupWrite();
    }
}

#endif
