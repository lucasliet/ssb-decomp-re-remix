/**
 * sc1premixintro.c - per-fighter data of the 1P title card, see sc1premixintro.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "remix/remix_action_layout.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include "sc/sc1pmode/sc1premixintro.h"

#define SC1PREMIX_INTRO_NAME_HEADER 0x10

static sb32 sc1PRemixIntroIsKnownKind(s32 fkind)
{
    return ((fkind >= 0) && (fkind < REMIX_ACTION_ID_COUNT)) ? TRUE : FALSE;
}

s32 sc1PRemixIntroPreviewKind(s32 fkind)
{
    if (sc1PRemixIntroIsKnownKind(fkind) == FALSE)
    {
        return REMIX_ID_MARIO;
    }
    return (s32)remix_ram_image_read_word(REMIX_SYM_Character__singleplayer_vs_preview__table + (fkind * 4));
}

intptr_t sc1PRemixIntroNameOffset(s32 fkind)
{
    if (sc1PRemixIntroIsKnownKind(fkind) == FALSE)
    {
        fkind = REMIX_ID_MARIO;
    }
    return (intptr_t)remix_ram_image_read_word(REMIX_SYM_SinglePlayer__name_texture_table + (fkind * 4)) + SC1PREMIX_INTRO_NAME_HEADER;
}

sb32 sc1PRemixIntroNameIsInCardFile(s32 fkind)
{
    return ((fkind == REMIX_ID_METAL) || (fkind == REMIX_ID_GDONKEY) || (fkind == REMIX_ID_BOSS)) ? TRUE : FALSE;
}

u32 sc1PRemixIntroAnnounceCall(s32 fkind)
{
    if (sc1PRemixIntroIsKnownKind(fkind) == FALSE)
    {
        fkind = REMIX_ID_MARIO;
    }
    return remix_ram_image_read_half(REMIX_SYM_CharacterSelect__fgm_table + (fkind * 2));
}

u32 sc1PRemixIntroNameDelay(s32 fkind)
{
    if (sc1PRemixIntroIsKnownKind(fkind) == FALSE)
    {
        fkind = REMIX_ID_MARIO;
    }
    return remix_ram_image_read_word(REMIX_SYM_SinglePlayer__name_delay_table + (fkind * 4));
}

#endif
