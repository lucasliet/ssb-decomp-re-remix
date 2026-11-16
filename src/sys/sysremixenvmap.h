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

/**
 * Marks the Continue / Game Over screen as running. The asm tests scene id 0x01 with the Continue image (file 0x4F) loaded
 * first; the port keeps scene_curr at the 1P battle while sc1PManager runs that screen, so the screen announces itself.
 *
 * @param is_active Non-zero while mnPlayers1PGameContinueStartScene runs.
 */
extern void sysRemixEnvMapSetContinueScreen(int is_active);

#endif

#endif
