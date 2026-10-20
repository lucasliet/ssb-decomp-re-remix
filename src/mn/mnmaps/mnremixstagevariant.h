#ifndef _MNREMIXSTAGEVARIANT_H_
#define _MNREMIXSTAGEVARIANT_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

extern s32 mnRemixMapsGetVariantType(void);
extern s32 mnRemixMapsGetVariantGroundKind(s32 gkind);
extern sb32 mnRemixMapsCycleVariant(s32 gkind);

#endif

#endif
