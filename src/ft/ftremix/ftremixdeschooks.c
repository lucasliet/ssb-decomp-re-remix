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

int ftRemixCrashNSPShieldHitlagSkip(GObj *fighter_gobj, int status_id);
int ftRemixCrashDSPECBOverride(FTStruct *fp, int next_status_id, float *out_upper, float *out_middle);

void ftRemixBindDescriptorHooks(s32 remix_id, struct FighterDescriptor *descriptor)
{
    switch (remix_id)
    {
    case REMIX_ID_CRASH:
        descriptor->shield_hitlag_skip = ftRemixCrashNSPShieldHitlagSkip; /* CrashSpecial.asm shield_hitlag_patch_ */
        descriptor->ecb_override = ftRemixCrashDSPECBOverride;            /* CrashSpecial.asm dig_ecb_patch_ */
        break;

    default:
        break;
    }
}

#endif
