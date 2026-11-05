#ifndef _MNREMIXCOSTUMES_H_
#define _MNREMIXCOSTUMES_H_

#include <ssb_types.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

#define MNREMIX_COSTUME_BUTTON_UP 0
#define MNREMIX_COSTUME_BUTTON_RIGHT 1
#define MNREMIX_COSTUME_BUTTON_DOWN 2
#define MNREMIX_COSTUME_BUTTON_LEFT 3

#define MNREMIX_SHADE_MAX 2

extern sb32 mnRemixCostumeGetDefaultRoyal(s32 fkind, s32 button, s32 *costume);
extern sb32 mnRemixCostumeGetDefaultTeam(s32 fkind, s32 team, s32 *costume);
extern s32 mnRemixCostumeGetMaxID(s32 fkind);
extern s32 mnRemixCostumeGetRandomLegal(s32 fkind);
extern void mnRemixCostumeApplyButton(s32 fkind, s32 button, s32 *costume, s32 *shade);

/**
 * Applies the extra costume of the fighter model of a CSS panel (ftRemixPartImageApplyFighter): the CSS model is not drawn by
 * ftDisplayMainProcDisplay, so each CSS calls this every frame for its panel fighters, after the model or its costume changed.
 * Idempotent, and does nothing for a vanilla costume.
 *
 * @param fighter_gobj Fighter GObj of the panel, NULL when the panel has none.
 */
extern void mnRemixCostumeApplyPartImage(GObj *fighter_gobj);

#endif

#endif
