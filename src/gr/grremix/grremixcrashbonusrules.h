#ifndef _GRREMIXCRASHBONUSRULES_H_
#define _GRREMIXCRASHBONUSRULES_H_

#include <stdint.h>

/*
 * Pure rules of the 1-up easter egg of Crash's Break the Targets and Board the Platforms (Hazards.asm crash_btt_setup,
 * crash_btp_setup, crash_btt_main and crash_btp_main). Ints only, so the headless tests can run them.
 */

/** Hazards.asm crash_bt*_main: FGM.play(0x5C3) when the 1-up is collected. */
#define GRREMIX_CRASH_BONUS_FGM_ONE_UP 0x05C3
/** MIDI.id.CRASHBASH_LOADING (docs/remix/audio.md): the song that replaces the stage song. */
#define GRREMIX_CRASH_BONUS_BGM_ONE_UP 0x016F
/** Hazards.asm crash_bt*_main: lui 0x38D1, the scale of the shrunk 1-up object. */
#define GRREMIX_CRASH_BONUS_ONE_UP_SCALE_WORD 0x38D10000u

/** Where the 1-up of a Crash bonus stage sits: the floor line under it and its yakumono DObj. */
typedef struct GRRemixCrashBonusOneUp
{
    int32_t line_id;
    int32_t dobj_index;

} GRRemixCrashBonusOneUp;

/**
 * 1-up of a stage: Crash Break the Targets watches floor line 9 and shrinks yakumono DObj 5 (stage objects + 0x14),
 * Crash Board the Platforms line 0x16 and DObj 15 (+ 0x3C).
 *
 * @param gkind Remix stage id.
 * @return The 1-up of the stage, NULL when the stage has no crash_bt*_setup.
 */
const GRRemixCrashBonusOneUp* grRemixCrashBonusRulesFind(int32_t gkind);

/**
 * TRUE when a fighter on this floor line collects the 1-up (Hazards.asm crash_bt*_main: FTStruct + 0xEC against the line).
 *
 * @param one_up 1-up of the stage.
 * @param floor_line_id Floor line of the fighter.
 */
int32_t grRemixCrashBonusRulesIsCollected(const GRRemixCrashBonusOneUp *one_up, int32_t floor_line_id);

#endif
