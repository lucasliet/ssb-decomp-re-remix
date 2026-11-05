/**
 * sc1premixhrcrecord.c - Home-Run Contest record in the Remix save, see sc1premixhrcrecord.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <lb/library.h>
#include <lb/lbbackup.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include "sc/sc1pmode/sc1premixhighscores.h"
#include "sc/sc1pmode/sc1premixhrcrecord.h"
#include "sc/sc1pmode/sc1premixhrcrules.h"

static sb32 sc1PRemixHrcRecordIsValidFighter(s32 fkind)
{
    return ((fkind >= 0) && (fkind < ARRAY_COUNT(gSCManagerBackupData.spgame_records))) ? TRUE : FALSE;
}

s32 sc1PRemixHrcRecordGet(s32 fkind)
{
    if (sc1PRemixHrcRecordIsValidFighter(fkind) == FALSE)
    {
        return 0;
    }
    return (s32)gSCManagerBackupData.spgame_records[fkind].hrc_distance;
}

sb32 sc1PRemixHrcRecordSave(s32 fkind, s32 distance)
{
    s32 verdict;

    if ((sc1PRemixHrcRecordIsValidFighter(fkind) == FALSE) || (distance < 0))
    {
        return FALSE;
    }
    if (sc1PRemixHighScoresCanSaveRecord() == FALSE)
    {
        return FALSE;
    }
    verdict = sc1PRemixHrcRecordVerdict(gSCManagerBackupData.spgame_records[fkind].hrc_distance, (unsigned int)distance);

    if (verdict == SC1PREMIX_HRC_RECORD_KEEP)
    {
        return FALSE;
    }
    gSCManagerBackupData.spgame_records[fkind].hrc_distance = (u32)distance;
    lbBackupWrite();
    port_log("hrc: record saved, fkind=%d distance=%d beaten=%d\n", (int)fkind, (int)distance, (int)(verdict == SC1PREMIX_HRC_RECORD_BEATEN));

    return (verdict == SC1PREMIX_HRC_RECORD_BEATEN) ? TRUE : FALSE;
}

#endif
