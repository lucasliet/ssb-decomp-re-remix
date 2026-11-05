#ifndef _MNREMIXCHARACTERS_H_
#define _MNREMIXCHARACTERS_H_

#include <ssb_types.h>
#include <mn/mndef.h>
#include "mn/mndata/mnremixchardata.h"

#if defined(PORT) && defined(REGION_REMIX)

typedef struct MNRemixCharactersEmblem
{
    intptr_t dobjdesc;
    intptr_t mobjsub;
    intptr_t matanim;
} MNRemixCharactersEmblem;

extern sb32 mnRemixCharactersHasTallNameTag(s32 fkind);
extern sb32 mnRemixCharactersGetEmblem(s32 fkind, MNRemixCharactersEmblem *emblem);
extern void mnRemixCharactersMakeNullMotion(MNCharactersMotion *motion);
extern sb32 mnRemixCharactersGetSpecialMotion(MNCharactersMotion *motion, s32 fkind, s32 motion_kind, s32 track, s32 *vanilla_fkind);
extern sb32 mnRemixCharactersGetJabMotion(MNCharactersMotion *motion, s32 fkind, s32 track, s32 *vanilla_fkind);
extern sb32 mnRemixCharactersGetSpecialNameRef(s32 fkind, s32 motion_kind, MNRemixCharDataSpriteRef *ref);
extern sb32 mnRemixCharactersIsMotionPlayable(s32 fkind, const MNCharactersMotion *motion);
extern sb32 mnRemixCharactersNextForcedMotionKind(s32 *motion_kind);

#endif

#endif
