#ifndef _GRREMIXRTTF_H_
#define _GRREMIXRTTF_H_

/*
 * Race to the Finish (Remix 1P) bumpers. Hazards.asm rttf_bumpers_ patches grBonus3MakeBumpers (0x8010B508) so that
 * the RTTF map reads its bumper anim joints at another offset. The patch only runs on Remix stage REMIX_RTTF.
 */

#include <ssb_types.h>

#define GRREMIX_RTTF_BUMPERS_ANIM_JOINT_OFFSET 0x01E8 /* vanilla llGRBonus3MapBumpersAnimJoint is 0x0110 */

/** Offset of the bumper anim joints in the current stage map: the RTTF value on Race to the Finish, the vanilla one otherwise. */
s32 grRemixBonus3GetBumpersAnimJointOffset(void);

#endif
