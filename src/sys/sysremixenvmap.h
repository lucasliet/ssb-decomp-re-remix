#ifndef _SYSREMIXENVMAP_H_
#define _SYSREMIXENVMAP_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/gbi.h>

/**
 * SinglePlayer.asm env_mapping_fix_ (RAM 0x80016958, camera display list setup): on the screens of sysRemixEnvMapIsLookAtScreen
 * appends the two G_MV_LIGHT look-at commands, which a camera with its own look-at matrix replaces later in the list.
 *
 * @param dl Display list cursor, advanced past the appended commands.
 */
extern void sysRemixEnvMapAppendLookAt(Gfx **dl);

#endif

#endif
