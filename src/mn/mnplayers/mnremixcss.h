#ifndef _MNREMIXCSS_H_
#define _MNREMIXCSS_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

extern f32 mnRemixCssCellX(s32 slot);
extern f32 mnRemixCssCellY(s32 slot);
extern s32 mnRemixCssSlotAtPoint(s32 pos_x, s32 pos_y);
extern s32 mnRemixCssGetFighterKind(s32 slot);
extern s32 mnRemixCssGetPortrait(s32 fkind);
extern s32 mnRemixCssGetSelectionStatus(s32 fkind);
extern f32 mnRemixCssGetSpotlightScale(s32 fkind);

#endif

#endif
