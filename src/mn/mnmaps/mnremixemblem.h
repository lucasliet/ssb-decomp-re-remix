#ifndef _MNREMIXEMBLEM_H_
#define _MNREMIXEMBLEM_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

extern sb32 mnRemixMapsGetEmblem(s32 gkind, u32 *sprite_offset, f32 *pos_x, f32 *pos_y);

#endif

#endif
