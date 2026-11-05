/**
 * ftremixalternatevoicerules.c - pure rule of Wario's alternate neutral special voice, see ftremixalternatevoicerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixalternatevoicerules.h"

int ftRemixAlternateVoiceShouldSwap(int voice_id, unsigned int button_hold, int is_flag_set)
{
    if (voice_id != FTREMIX_ALTERNATE_VOICE_WARIO_NSP)
    {
        return 0;
    }
    if ((button_hold & FTREMIX_ALTERNATE_VOICE_BUTTON_L) == 0)
    {
        return 0;
    }
    return (is_flag_set == 0) ? 1 : 0;
}

#endif
