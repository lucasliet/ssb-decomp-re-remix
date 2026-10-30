#ifndef _FTREMIXAITECHING_H_
#define _FTREMIXAITECHING_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Teching and Z-cancel hooks of the Remix AI (AI.asm random_teching_, z_cancel_). */

/**
 * random_teching_: TRUE when the fighter teched (roll, in place or the vanilla tech), FALSE when the bounce must run.
 * Replaces the tech checks of ftCommonDamageAirCommonProcMap and ftCommonDamageFallProcMap.
 */
sb32 ftRemixAiTechIsHandled(GObj *fighter_gobj);

/** z_cancel_: TRUE when a CPU Z-cancel roll succeeds for this landing. Consumed by ftRemixZCancelResult. */
sb32 ftRemixAiShouldZCancel(FTStruct *fp);

#endif

#endif
