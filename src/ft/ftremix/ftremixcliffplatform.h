#ifndef _FTREMIXCLIFFPLATFORM_H_
#define _FTREMIXCLIFFPLATFORM_H_

/*
 * Cliff statuses on a platform that disappears (Hazards.asm platform_despawn_crash_fix and platform_despawn_crash_fix_2).
 * ftcommoncliffcatchwait.c and ftcommoncliffclimb.c call this under #ifdef REGION_REMIX.
 */

#include <ssb_types.h>

/**
 * TRUE when the line a fighter hangs on, or climbs from, belongs to a yakumono (moving platform) that is turned off or
 * hidden. The caller then sets the fall status instead of reading the vanished line.
 *
 * @param line_id Collision line id of the cliff (coll_data.cliff_id or cliffmotion.cliff_id).
 */
sb32 ftRemixCliffPlatformHasVanished(s32 line_id);

#endif
