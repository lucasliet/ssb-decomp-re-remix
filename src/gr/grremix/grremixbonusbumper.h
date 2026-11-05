#ifndef _GRREMIXBONUSBUMPER_H_
#define _GRREMIXBONUSBUMPER_H_

/*
 * Bumpers of the Remix Board the Platforms stages (Hazards.asm sonic_btp_setup).
 */

#include <ssb_types.h>
#include <sys/objdef.h>

/**
 * sonic_btp_setup: the bumpers of Sonic's Board the Platforms play the casino bumper sound (FGM 0x3D5) when hit and draw
 * the casino bumper palette and texture of the file at stage header +0xB0 (on a private copy of the display list).
 *
 * @param item_gobj Bumper item just made by sc1PBonusStageMakeBumpers.
 */
void grRemixBonusBumperSetup(GObj *item_gobj);

#endif
