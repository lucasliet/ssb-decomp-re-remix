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

int ftRemixCrashNSPShieldHitlagSkip(GObj *fighter_gobj, int status_id);
int ftRemixCrashDSPECBOverride(FTStruct *fp, int next_status_id, float *out_upper, float *out_middle);

#define FTREMIX_ACTION_THROWN_DK 0xB8 /* Action.ThrownDK, Action.asm line 189 */

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
        break;

    case REMIX_ID_PIANO:
        descriptor->custom_capture_action = FTREMIX_ACTION_THROWN_DK;       /* Piano.asm:387 table_patch_start(custom_capture_action) */
        descriptor->custom_capture_dk_interrupt = ftRemixPianoCaptureDKInterrupt; /* Piano.asm capture_action_fix_ */
        break;

    default:
        break;
    }
}

#endif
