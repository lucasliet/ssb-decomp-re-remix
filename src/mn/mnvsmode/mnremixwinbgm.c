/**
 * mnremixwinbgm.c - Smash Remix winner BGM (src/resultsscreen.asm get_victory_bgm_, Character.winner_bgm.table).
 *
 * The table holds one code address per Remix character id. A Remix address points at `add_victory_bgm(bgm)`, three
 * instructions whose last one loads the BGM id (`ori a1, zero, bgm`). A vanilla address means the fighter uses its
 * parent's victory music.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define MNREMIX_WINBGM_IMAGE_BASE 0x80400000U
#define MNREMIX_WINBGM_ORI_A1_ZERO 0x34050000U
#define MNREMIX_WINBGM_ROUTINE_WORDS 6

static u32 mnRemixWinBGMReadWord(u32 ram_addr)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr);

    return (word != NULL) ? *word : 0;
}

/** BGM id of the winner `fkind` from Character.winner_bgm.table, or -1 when the entry is a vanilla routine. */
s32 mnRemixResultsGetWinBGM(s32 fkind)
{
    u32 routine = mnRemixWinBGMReadWord(REMIX_SYM_Character__winner_bgm__table + (fkind * 4));
    s32 i;

    if (routine < MNREMIX_WINBGM_IMAGE_BASE)
    {
        return -1;
    }
    for (i = 0; i < MNREMIX_WINBGM_ROUTINE_WORDS; i++)
    {
        u32 instruction = mnRemixWinBGMReadWord(routine + (i * 4));

        if ((instruction & 0xFFFF0000U) == MNREMIX_WINBGM_ORI_A1_ZERO)
        {
            return (s32)(instruction & 0xFFFFU);
        }
    }
    return -1;
}

#endif
