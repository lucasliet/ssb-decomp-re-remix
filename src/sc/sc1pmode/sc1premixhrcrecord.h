#ifndef _SC1PREMIXHRCRECORD_H_
#define _SC1PREMIXHRCRECORD_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Home-Run Contest record of each fighter (SinglePlayerModes.asm _time_save), kept in the Remix save next to the 1P records. The rule that
 * compares distances is in sc1premixhrcrules.h.
 */

/**
 * Saves the distance of an attempt when it makes or beats the record of the fighter.
 *
 * @param fkind Fighter that played.
 * @param distance Distance in tenths of a foot.
 * @return TRUE when it beat an earlier record, so the end of the attempt is announced as a new record.
 */
extern sb32 sc1PRemixHrcRecordSave(s32 fkind, s32 distance);

/**
 * Record of a fighter.
 *
 * @param fkind Fighter.
 * @return Distance in tenths of a foot, 0 for none.
 */
extern s32 sc1PRemixHrcRecordGet(s32 fkind);

#endif

#endif
