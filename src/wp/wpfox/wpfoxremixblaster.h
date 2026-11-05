#ifndef _WPFOXREMIXBLASTER_H_
#define _WPFOXREMIXBLASTER_H_

/*
 * J Fox laser of Smash Remix (.toolchain/pkgs/smashremix-src/src/JFox/JFox.asm get_laser_special_struct_). Declared here,
 * defined in wpfoxremixblaster.c.
 */

#include <ft/fighter.h>
#include <wp/weapon.h>

/**
 * Points the blaster descriptor at the J Fox weapon file (file 6) when the shooter is J Fox, or a Kirby or J Kirby that
 * copied J Fox. Other shooters, and a J Fox whose file is not loaded, keep the vanilla Fox file.
 * @param fp Fighter that fires the laser.
 * @param desc Copy of dWPFoxBlasterWeaponDesc, edited in place.
 */
void wpRemixFoxBlasterSetFile(FTStruct *fp, WPDesc *desc);

#endif
