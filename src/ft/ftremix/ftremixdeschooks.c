/**
 * ftremixdeschooks.c - per-fighter FighterDescriptor hooks for Remix synths.
 *
 * Some Remix patches hard-code a character id inside vanilla routines. The port
 * already exposes those sites as per-descriptor callbacks (fighter_registry.h);
 * this file installs the ported Remix routines on the matching synth.
 */

#ifdef PORT

#include "ftremix.h"
#include "fighter_registry.h"
#include "ftremixhooks.h"
#include "ft/ftremix/ftremixcaptureinterrupt.h"

int ftRemixCrashNSPShieldHitlagSkip(GObj *fighter_gobj, int status_id);
int ftRemixCrashDSPECBOverride(FTStruct *fp, int next_status_id, float *out_upper, float *out_middle);

#define FTREMIX_ACTION_THROWN_DK 0xB8 /* Action.ThrownDK, Action.asm line 189 */
#define FTREMIX_SKELETON_COLANIM_METAL 0x10 /* electric_hit entry of the metal and polygon fighters: no skeleton */

void ftRemixBindDescriptorHooks(s32 remix_id, struct FighterDescriptor *descriptor)
{
    descriptor->entry_make_effect = ftRemixEntryMakeEffect;

    switch (remix_id)
    {
    case REMIX_ID_CRASH:
        descriptor->shield_hitlag_skip = ftRemixCrashNSPShieldHitlagSkip; /* CrashSpecial.asm shield_hitlag_patch_ */
        descriptor->ecb_override = ftRemixCrashDSPECBOverride;            /* CrashSpecial.asm dig_ecb_patch_ */
        break;

    case REMIX_ID_WARIO:
        descriptor->custom_capture_action = FTREMIX_ACTION_THROWN_DK; /* Wario.asm:624 table_patch_start(custom_capture_action) */
        descriptor->custom_capture_dk_interrupt = ftRemixWarioCaptureDKInterrupt; /* Wario.asm:633 capture_action_fix_ */
        break;

    case REMIX_ID_MARINA:
        descriptor->custom_capture_dk_interrupt = ftRemixMarinaCaptureDKInterrupt; /* Marina.asm:695 capture_action_fix_ */
        break;

    case REMIX_ID_PIANO:
        descriptor->custom_capture_action = FTREMIX_ACTION_THROWN_DK;       /* Piano.asm:387 table_patch_start(custom_capture_action) */
        descriptor->custom_capture_dk_interrupt = ftRemixPianoCaptureDKInterrupt; /* Piano.asm capture_action_fix_ */
        break;

    case REMIX_ID_DRAGONKING:
    case REMIX_ID_MLUIGI:
        descriptor->skeleton_col_anim_base = FTREMIX_SKELETON_COLANIM_METAL; /* DragonKing.asm:342, MLuigi.asm:92 table_patch_start(electric_hit) */
        break;

    default:
        break;
    }
}

#endif
