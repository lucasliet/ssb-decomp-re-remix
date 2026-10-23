/**
 * gmremixmusictitle.c - Music Title at Match Start toggle of the Remix settings (BGM.asm show_music_title_).
 *
 * The vanilla banner is drawn with the Remix font, which the port does not draw; the message goes through the engine
 * overlay (port/remix/remix_music_title.cpp) instead. The asm shows the banner until GO! appears, the overlay fades
 * after GMREMIX_MUSIC_TITLE_SECONDS.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmusictitle.h"
#include "gm/gmremixtoggle.h"
#include <sys/audio.h>
#include "remix/remix_music_title.h"

#define GMREMIX_MUSIC_TITLE_SECONDS 4.0f

void gmRemixMusicTitleShowAtCountdown(void)
{
    if (!gmRemixToggle(show_music_title) || !gmRemixToggle(play_music))
    {
        return;
    }
    remix_music_title_show(syAudioGetBGMPlaying(0), GMREMIX_MUSIC_TITLE_SECONDS);
}

#endif
