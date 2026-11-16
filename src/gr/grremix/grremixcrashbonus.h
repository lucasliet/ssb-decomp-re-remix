#ifndef _GRREMIXCRASHBONUS_H_
#define _GRREMIXCRASHBONUS_H_

/*
 * 1-up easter egg of Crash's Break the Targets and Board the Platforms (Hazards.asm crash_btt_setup, crash_btp_setup,
 * crash_btt_main, crash_btp_main). Stages.asm id_fix_2_ runs the setup before the bonus ground is built.
 */

#include <ssb_types.h>

/**
 * Creates the 1-up watcher of a Crash bonus stage and clears its collected flag. Does nothing on another stage.
 *
 * @param gkind Remix stage id.
 */
void grRemixCrashBonusMake(s32 gkind);

#endif
