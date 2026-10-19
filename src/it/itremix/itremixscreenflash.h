#ifndef _ITREMIXSCREENFLASH_H_
#define _ITREMIXSCREENFLASH_H_

#ifdef PORT

#include <ssb_types.h>

#ifdef REGION_REMIX
#include "gm/gmremixtoggle.h"
/* Flash Guard (Toggles.asm entry_flash_guard): the item flashes are skipped while it is on, as in Flashbang.asm and DekuNut.asm. */
#define ITREMIX_FLASH_GUARD_ON gmRemixToggle(flash_guard)
#else
#define ITREMIX_FLASH_GUARD_ON FALSE
#endif

/** Starts the Flashbang screen flash (Flashbang.asm _check_flash) unless the flash guard is on. */
extern void itRemixScreenFlashFlashbang(void);
/** Starts the Deku Nut screen flash (DekuNut.asm _check_flash) unless the flash guard is on. */
extern void itRemixScreenFlashDekuNut(void);

#endif

#endif
