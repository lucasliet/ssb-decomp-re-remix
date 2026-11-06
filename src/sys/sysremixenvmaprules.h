#ifndef _SYSREMIXENVMAPRULES_H_
#define _SYSREMIXENVMAPRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Whether a screen sets the Remix look-at vectors when its camera is set up (SinglePlayer.asm env_mapping_fix_), so Metal
 * Mario and the polygons get environment mapping on screens whose camera has no look-at matrix: the 1P VS preview (0x0E),
 * the CSS screens (0x10 to 0x14), the VS results (0x18), the ending room (0x30) and the Continue screen (0x31, where the asm
 * tests the title screen with the Continue image as first loaded file).
 *
 * @param screen_id Screen id (gSCManagerSceneData.scene_curr, the same numbers as Global.current_screen).
 * @return Non-zero when the look-at vectors are set.
 */
extern int sysRemixEnvMapIsLookAtScreen(int screen_id);

#endif

#endif
