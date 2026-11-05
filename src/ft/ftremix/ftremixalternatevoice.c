/**
 * ftremixalternatevoice.c - Wario.asm alternate_voice_ run from ftParamPlayVoice.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixalternatevoicerules.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixparamhooks.h"

#define FTREMIX_ALTERNATE_VOICE_N64_FLAG 0x0ADC

/**
 * Voice a fighter plays: Wario's neutral special voice becomes the alternate one the first time it plays with L held. The
 * hook runs for every fighter, as in the asm, and keeps the voice of any other FGM.
 *
 * @param fp Fighter about to play the voice.
 * @param voice_id FGM id requested.
 * @return FGM id to play.
 */
u16 ftRemixAlternateVoiceId(FTStruct *fp, u16 voice_id)
{
    s32 *flag = ftRemixFighterWord(fp, FTREMIX_ALTERNATE_VOICE_N64_FLAG);

    if (!ftRemixAlternateVoiceShouldSwap(voice_id, fp->input.pl.button_hold, *flag))
    {
        return voice_id;
    }
    *flag = TRUE;

    return FTREMIX_ALTERNATE_VOICE_WARIO_NSP_ALTERNATE;
}

#endif
