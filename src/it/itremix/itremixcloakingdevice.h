#ifndef _ITREMIXCLOAKINGDEVICE_H_
#define _ITREMIXCLOAKINGDEVICE_H_

#include "it/itremix/itremix.h"

/* Spawns Cloaking Device (Remix item 0x2D). Fighter call sites use this signature. */
GObj* itRemixCloakingDeviceMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Forgets every port's cloak. Meant for the scene-end hook (see notes, Vanilla patches). */
void itRemixCloakingDeviceClearStates(void);

/* TRUE while `player` (port index) is cloaked; gates the damage-immunity patch. */
sb32 itRemixCloakingDeviceIsCloaked(s32 player);

extern const ITRemixItemDesc dITRemixCloakingDeviceItemDesc;

#endif
