#ifndef _ITREMIXSCREENFLASH_H_
#define _ITREMIXSCREENFLASH_H_

#ifdef PORT

#include <ssb_types.h>

/* Toggles.entry_flash_guard has no port accessor yet. FALSE (the Remix default) draws the flash until the settings menu (ROADMAP M4) wires the toggle. */
#define ITREMIX_FLASH_GUARD_ON FALSE

/** Starts the Flashbang screen flash (Flashbang.asm _check_flash) unless the flash guard is on. */
extern void itRemixScreenFlashFlashbang(void);
/** Starts the Deku Nut screen flash (DekuNut.asm _check_flash) unless the flash guard is on. */
extern void itRemixScreenFlashDekuNut(void);

#endif

#endif
