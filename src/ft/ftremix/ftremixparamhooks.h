#ifndef _FTREMIXPARAMHOOKS_H_
#define _FTREMIXPARAMHOOKS_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Remix routines that ftparam.c calls (REGION_REMIX only). Each routine checks the fighter kind itself and is a no-op for
 * other fighters.
 */

#include <ft/fighter.h>

/* Character.gfx_routine_end (get_gfx_routine_end_, 0x800E9948) in ftParamResetStatUpdateColAnim, ftremixchargeflash.c */
void ftRemixRestartChargeFlash(GObj *fighter_gobj);

/* Wario.asm alternate_voice_ (0x800E80FC) in ftParamPlayVoice, ftremixalternatevoice.c */
u16 ftRemixAlternateVoiceId(FTStruct *fp, u16 voice_id);

/* Slippy.asm hips_y_translation_ (0x800E83D8) in ftParamUpdateAnimKeys, slippy/ftremixslippyhips.c */
void ftRemixPlayFighterJointAnim(FTStruct *fp, DObj *joint);


/* KirbyHats.asm use_extended_special_parts_pause_ gun branch (0x800E8D04) in ftParamSetModelPartID, ftremixkirbygun.c */
FTModelPartDesc* ftRemixKirbyGunPartDesc(FTStruct *fp, s32 joint_id, FTModelPartDesc *container_desc);

#endif

#endif
