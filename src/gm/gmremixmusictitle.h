#ifndef _GMREMIXMUSICTITLE_H_
#define _GMREMIXMUSICTITLE_H_

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Shows the banner with the name of the playing track and its game when the match countdown starts (BGM.asm
 * show_music_title_, hooked at the GO lights). Does nothing when Music Title at Match Start or Play Music is off, or when
 * the track has no title.
 */
void gmRemixMusicTitleShowAtCountdown(void);

/** Takes the banner down when the GO! text appears (BGM.asm show_music_title_._destroy). */
void gmRemixMusicTitleHideAtGo(void);

#endif

#endif
