#ifndef _FTREMIXSTAMINAHIT_H_
#define _FTREMIXSTAMINAHIT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/**
 * Stamina.asm action_hit_crash_prevent (RAM 0x800E825C, ftParamStopVoiceRunProcDamage): a fighter already defeated in a VS
 * Stamina match that is hit again has the status variable word the next status reads (dead.wait) cleared, so a status that
 * reads it does not run on the value left by the KO.
 *
 * @param fp Fighter struct of the fighter that was hit.
 */
void ftRemixStaminaClearDeadWaitOnHit(FTStruct *fp);

#endif

#endif
