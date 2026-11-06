#ifndef _FTREMIXSTAMINAACTION_H_
#define _FTREMIXSTAMINAACTION_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/**
 * Animation speed ftMainSetStatus uses for the status it sets (Stamina.asm stamina_ko_action, _corrections): the forced KO
 * action plays at normal speed. Call before ftRemixStaminaKOStatus replaces the requested status.
 *
 * @param fp Fighter struct.
 * @param status_id Status requested.
 * @param anim_speed Animation speed requested.
 * @return 1.0 when the KO action replaces the request, otherwise anim_speed.
 */
f32 ftRemixStaminaKOAnimSpeed(FTStruct *fp, s32 status_id, f32 anim_speed);

#endif

#endif
