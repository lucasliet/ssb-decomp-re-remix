#ifndef _GMREMIXBGMLOG_H_
#define _GMREMIXBGMLOG_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * SSB64_BGM_LOG=1: one `[bgm]` line at every BGM start and stop, read by the headless smoke tests. Off by default; an
 * unset, empty or "0" variable means off. `t` is the battle tic, or 0 outside a battle.
 */

/**
 * Logs `[bgm] play t=<tic> track=<id> player=<n>`.
 *
 * @param sngplayer BGM player index.
 * @param bgm Track id actually started, after every substitution.
 */
extern void gmRemixBgmLogPlay(s32 sngplayer, u32 bgm);

/**
 * Logs `[bgm] stop t=<tic> player=<n>`.
 *
 * @param sngplayer BGM player index being stopped.
 */
extern void gmRemixBgmLogStop(s32 sngplayer);

#endif

#endif
