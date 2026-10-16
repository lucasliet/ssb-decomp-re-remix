/**
 * itremixscreenflash.c - Remix screen flashes of Flashbang and Deku Nut.
 *
 * The asm stores a blend-colour command pointer at RAM 0x80131A40 (Flashbang.asm
 * _check_flash, DekuNut.asm _check_flash). That address is sIFScreenFlashColAnim,
 * the colour animation that the IF screen flash draws (if/ifscreenflash.c).
 */

#ifdef PORT

#include "it/itremix/itremixscreenflash.h"
#include <ft/ftparam.h>
#include <gm/gmdef.h>
#include <gm/gmtypes.h>

#define ITREMIX_SCREEN_FLASH_LENGTH 60

extern GMColAnim sIFScreenFlashColAnim;

/* Flashbang.asm flash_array_: white flash that fades out over 8 frames, then waits 4 frames. */
static u32 sITRemixScreenFlashFlashbangScript[] =
{
    gmColCommandSetColor1(0xDD, 0xDD, 0xDD, 0x88),
    gmColCommandBlendColor1(8, 0xFF, 0xFF, 0xFF, 0x00),
    gmColCommandWait(4),
    gmColCommandEnd()
};

/* DekuNut.asm flash_array_: white flash that fades out over 8 frames, then waits 8 frames. */
static u32 sITRemixScreenFlashDekuNutScript[] =
{
    gmColCommandSetColor1(0xDD, 0xDD, 0xDD, 0xCC),
    gmColCommandBlendColor1(8, 0xFF, 0xFF, 0xFF, 0x00),
    gmColCommandWait(8),
    gmColCommandEnd()
};

/** Replaces the IF screen flash with `script`, the way ftParamCheckSetColAnimID does for table ids. */
static void itRemixScreenFlashPlay(u32 *script)
{
    GMColAnim *colanim = &sIFScreenFlashColAnim;

    ftParamResetColAnim(colanim);
    colanim->cs[0].p_script = script;
    colanim->cs[0].color_event_timer = 0;
    colanim->cs[0].script_id = 0;
    colanim->length = ITREMIX_SCREEN_FLASH_LENGTH;
}

/** Flashbang screen flash, skipped while the flash guard is on. */
void itRemixScreenFlashFlashbang(void)
{
    if (ITREMIX_FLASH_GUARD_ON)
    {
        return;
    }
    itRemixScreenFlashPlay(sITRemixScreenFlashFlashbangScript);
}

/** Deku Nut screen flash, skipped while the flash guard is on. */
void itRemixScreenFlashDekuNut(void)
{
    if (ITREMIX_FLASH_GUARD_ON)
    {
        return;
    }
    itRemixScreenFlashPlay(sITRemixScreenFlashDekuNutScript);
}

#endif
