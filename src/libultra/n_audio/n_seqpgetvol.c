#include "common.h"
#include <n_audio/n_libaudio.h>
#if defined(PORT) && defined(REGION_REMIX)
#include "gm/gmremixmidi.h"
#endif

// 0x8002F6C0
s16 n_alSeqpGetVol(N_ALSeqPlayer* seqp)
{
#if defined(PORT) && defined(REGION_REMIX)
    return (seqp->vol * gmRemixMidiMasterVolume(seqp->masterVol)) >> 7;
#else
    return (seqp->vol * seqp->masterVol) >> 7;
#endif
}
