#if defined(PORT) && defined(REGION_REMIX)

#ifndef _FTREMIXDKMODE_H_
#define _FTREMIXDKMODE_H_

#include <ft/fttypes.h>

/**
 * DK Mode and Lanky Mode head and arm scale (DKMode.asm _resize_head). Called from ftDisplayMainProcDisplay right after
 * fp = ftGetStruct(fighter_gobj), where the asm patch sits at RAM 0x800F296C.
 */
void ftRemixDKModeResizeHead(FTStruct *fp);

/**
 * Sets DKMode.dk_mode_enabled (0 off, 1 DK Mode, 2 Lanky Mode). The 12cb portrait easter egg of DKMode.asm _check_portraits calls it.
 */
void ftRemixDKModeSetMode(s32 mode);

#endif

#endif
