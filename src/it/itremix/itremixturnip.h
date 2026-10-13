#ifndef _ITREMIXTURNIP_H_
#define _ITREMIXTURNIP_H_

#include <ssb_types.h>
#include <sys/objdef.h>

typedef enum ITRemixTurnipFace
{
    nITRemixTurnipFaceNormal,
    nITRemixTurnipFaceWink,
    nITRemixTurnipFaceDot,
    nITRemixTurnipFaceStitch,
    nITRemixTurnipFaceCount
} ITRemixTurnipFace;

/* Peach DSP / Marina pull: spawns a Turnip with a random face (Turnip.asm stage_setting_). */
GObj* itRemixTurnipMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Sets the base damage and the face image of a spawned Turnip (Turnip.asm _set_turnip). */
void itRemixTurnipSetFace(GObj *item_gobj, ITRemixTurnipFace face);

#endif
