#ifndef _GMRUMBLE_H_
#define _GMRUMBLE_H_

#include <ssb_types.h>
#include <sys/objdef.h>
#include <gm/gmdef.h>

extern sb32 gmRumbleUpdateEventExecute(ub8 *is_active, GMRumbleScript *p_script, s32 player);
extern sb32 gmRumbleUpdateEventQueue(ub8 *is_active, GMRumbleScript *p_script);
extern void gmRumbleAddLinkAfter(GMRumblePlayer *rplayer, GMRumbleLink *rlink);
extern sb32 gmRumbleUpdateEventCheckEnd(GMRumbleScript *p_script);
extern void gmRumbleUpdateEventLinks(GMRumblePlayer *rplayer);
extern sb32 gmRumbleGetMotorUpdateStatus(ub8 *is_active, GMRumbleScript *p_script, s32 player);
extern void gmRumbleActorProcUpdate(GObj *rumble_gobj);
extern sb32 gmRumbleCheckSetEventID(GMRumblePlayer *rplayer, s32 index, s32 rumble_timer);
extern GMRumbleScript* gmRumbleGetEventPriorityRelink(GMRumblePlayer *rplayer, s32 index);
extern void gmRumbleSetPlayerRumbleParams(s32 player, s32 index, s32 rumble_timer);
extern void func_ovl2_801155C4(s32 player);
extern void gmRumbleStopRumbleID(s32 player, s32 index);
extern void gmRumbleMakeActor(void);
extern void gmRumbleInitPlayers(void);
extern void gmRumbleResumeProcessAll(void);

#if defined(PORT) && defined(REGION_REMIX)
#define GMRUMBLE_REMIX_KEEP_ID 0x17 /* SuperMushroom.asm:580 "non-valid rumble_id": no gmRumbleStopRumbleID call matches it */

extern void gmRumbleRemixSetPlayerRumbleKeepOnStatus(s32 player, s32 rumble_id, s32 rumble_timer);
#endif

#endif
