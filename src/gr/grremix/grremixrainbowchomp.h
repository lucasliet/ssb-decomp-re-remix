#ifndef _GRREMIXRAINBOWCHOMP_H_
#define _GRREMIXRAINBOWCHOMP_H_

#include <ssb_types.h>
#include <sys/obj.h>

/**
 * Spawns the Chain Chomp hitbox above the chomp's top joint (Hazards.asm chainchomp_hitbox_stage_setting_).
 * hitbox_file_slot points at the hitbox file pointer the weapon reads its attributes from.
 * Returns the weapon GObj, or NULL when the weapon could not be created.
 */
GObj *grRemixRainbowChompSpawn(GObj *chomp_gobj, void **hitbox_file_slot);

#endif
