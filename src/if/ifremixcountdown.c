/**
 * ifremixcountdown.c - stoplight countdown sounds of the Remix stages, see ifremixcountdown.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixcountdown.h"
#include "if/ifremixannouncer.h"
#include <sc/scene.h>
#include <sc/scmanager.h>
#include <sys/utils.h>

#define IFREMIX_COUNTDOWN_OPENING_FGM_FIRST 0x479
#define IFREMIX_COUNTDOWN_OPENING_FGM_COUNT 3

extern void *func_800269C0_275C0(u16 id);

/** PokemonAnnouncer.asm opening_comment: a random clip of 0x479 to 0x47B while the announcer is active. */
static void ifRemixCountdownPlayOpeningComment(void)
{
    if (ifRemixAnnouncerIsActive() == FALSE)
    {
        return;
    }
    func_800269C0_275C0(IFREMIX_COUNTDOWN_OPENING_FGM_FIRST + syUtilsRandIntRange(IFREMIX_COUNTDOWN_OPENING_FGM_COUNT));
}

void ifRemixCountdownPlayStep(IFRemixCountdownStep step, u16 voice_fgm)
{
    IFRemixCountdownSounds sounds;

    if (step == nIFRemixCountdownStepTwo)
    {
        ifRemixCountdownPlayOpeningComment();
    }
    sounds = ifRemixCountdownRulesGetSounds(gSCManagerBattleState->gkind, step);

    if (sounds.extra_fgm == IFREMIX_COUNTDOWN_NO_FGM)
    {
        func_800269C0_275C0(voice_fgm);
        return;
    }
    if (sounds.is_voice_replaced != FALSE)
    {
        func_800269C0_275C0(sounds.extra_fgm);
        return;
    }
    if (sounds.is_extra_first != FALSE)
    {
        func_800269C0_275C0(sounds.extra_fgm);
        func_800269C0_275C0(voice_fgm);
        return;
    }
    func_800269C0_275C0(voice_fgm);
    func_800269C0_275C0(sounds.extra_fgm);
}

#endif
