#ifndef _MNREMIXCOSTUMES_H_
#define _MNREMIXCOSTUMES_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

#define MNREMIX_COSTUME_BUTTON_UP 0
#define MNREMIX_COSTUME_BUTTON_RIGHT 1
#define MNREMIX_COSTUME_BUTTON_DOWN 2
#define MNREMIX_COSTUME_BUTTON_LEFT 3

#define MNREMIX_SHADE_MAX 2

extern sb32 mnRemixCostumeGetDefaultRoyal(s32 fkind, s32 button, s32 *costume);
extern sb32 mnRemixCostumeGetDefaultTeam(s32 fkind, s32 team, s32 *costume);
extern s32 mnRemixCostumeGetMaxID(s32 fkind);
extern void mnRemixCostumeApplyButton(s32 fkind, s32 button, s32 *costume, s32 *shade);

#endif

#endif
