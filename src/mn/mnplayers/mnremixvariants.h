#ifndef _MNREMIXVARIANTS_H_
#define _MNREMIXVARIANTS_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

#define MNREMIX_VARIANT_SLOT_COUNT 4

extern void mnRemixVariantHandleInput(s32 held_player, u16 button_tap);
extern s32 mnRemixRandomResolveFighterKind(s32 fkind);
extern s32 mnRemixRandomPickFighterKind(void);
extern s32 mnRemixVariantLookup(s32 fkind, s32 slot);
extern s32 mnRemixVariantGetFighterKind(s32 held_player, s32 fkind);
extern s32 mnRemixVariantGetOffset(s32 held_player);

#endif

#endif
