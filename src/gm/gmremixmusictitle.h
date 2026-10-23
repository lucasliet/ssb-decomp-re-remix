#ifndef _GMREMIXMUSICTITLE_H_
#define _GMREMIXMUSICTITLE_H_

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Shows the name of the playing track and its game when the match countdown starts (BGM.asm show_music_title_, hooked
 * at the GO lights). Does nothing when Music Title at Match Start or Play Music is off.
 */
void gmRemixMusicTitleShowAtCountdown(void);

#endif

#endif
