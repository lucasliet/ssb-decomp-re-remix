/**
 * mnremixresults.c - winner data of the Smash Remix VS results screen (src/resultsscreen.asm add_to_results_screen and the
 * get_winner_fgm_, get_str_wins_lx_, get_str_winner_info_ and winner_logo_*_fix_ patches).
 *
 * Remix keeps one entry per character id in the Character.winner_fgm, str_wins_lx, str_winner_ptr / lx / scale and sound_type
 * tables and in the ResultsScreen.series_logo_*_table tables. Everything is read at run time from the Remix RAM image
 * (port/remix/remix_ram_image.h); the 12 playable fighters keep the vanilla tables of the results screen.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "mn/mndata/mnremixcharacters.h"
#include "mn/mnvsmode/mnremixresults.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define MNREMIX_RESULTS_WORD_SIZE 4

static u32 mnRemixResultsReadTableWord(u32 table, s32 fkind)
{
    return remix_ram_image_read_word(table + (fkind * MNREMIX_RESULTS_WORD_SIZE));
}

static unsigned int mnRemixResultsReadImageByte(unsigned int ram_addr)
{
    return remix_ram_image_read_byte(ram_addr);
}

u32 mnRemixResultsGetWinnerFgm(s32 fkind)
{
    return mnRemixResultsReadTableWord(REMIX_SYM_Character__winner_fgm__table, fkind);
}

f32 mnRemixResultsGetWinsX(s32 fkind)
{
    return mnRemixResultsRulesBitsToFloat(mnRemixResultsReadTableWord(REMIX_SYM_Character__str_wins_lx__table, fkind));
}

sb32 mnRemixResultsIsSingularWin(s32 fkind)
{
    u32 sound_type = remix_ram_image_read_byte(REMIX_SYM_Character__sound_type__table + fkind);

    return (mnRemixResultsRulesUsesSingularWin(fkind, (sound_type == REMIX_SOUND_J) ? 1 : 0) != 0) ? TRUE : FALSE;
}

sb32 mnRemixResultsGetWinnerName(s32 fkind, char *name, f32 *x, f32 *scale)
{
    u32 name_addr = mnRemixResultsReadTableWord(REMIX_SYM_Character__str_winner_ptr__table, fkind);

    if ((name_addr == 0) || (mnRemixResultsRulesCopyName(mnRemixResultsReadImageByte, name_addr, name, MNREMIX_RESULTS_NAME_BUFFER_SIZE) == 0))
    {
        return FALSE;
    }
    *x = mnRemixResultsRulesBitsToFloat(mnRemixResultsReadTableWord(REMIX_SYM_Character__str_winner_lx__table, fkind));
    *scale = mnRemixResultsRulesBitsToFloat(mnRemixResultsReadTableWord(REMIX_SYM_Character__str_winner_scale__table, fkind));

    return TRUE;
}

sb32 mnRemixResultsGetEmblem(s32 fkind, intptr_t *dobjdesc, intptr_t *mobjsub, intptr_t *matanim)
{
    MNRemixCharactersEmblem emblem;

    if (mnRemixCharactersGetEmblem(fkind, &emblem) == FALSE)
    {
        return FALSE;
    }
    *dobjdesc = emblem.dobjdesc;
    *mobjsub = emblem.mobjsub;
    *matanim = emblem.matanim;

    return TRUE;
}

#endif
