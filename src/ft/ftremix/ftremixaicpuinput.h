#ifndef _FTREMIXAICPUINPUT_H_
#define _FTREMIXAICPUINPUT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* CPU-only inputs that the Remix AI adds inside shared fighter statuses. Pure rules live in ftremixaicpuinputrules.c. */

/**
 * auto_jab_grab_ (AI.asm): TRUE when a CPU grabs during its jab without the Z tap (level 10, from the third frame of the
 * status, Single Button Mode off, R or R+C). Called from ftCommonCatchCheckInterruptAttack11.
 */
sb32 ftRemixAiAutoJabGrab(FTStruct *fp);

/**
 * prevent_dash (GBowser/gbowser.asm): the status a dash start enters. A CPU Giga Bowser enters the fast walk instead of
 * Dash; everyone else keeps status_id. Called from ftCommonDashSetStatus.
 */
s32 ftRemixAiDashStatus(FTStruct *fp, s32 status_id);

#endif

#endif
