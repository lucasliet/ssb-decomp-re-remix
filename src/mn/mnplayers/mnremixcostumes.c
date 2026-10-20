/**
 * mnremixcostumes.c - Smash Remix costume data and the C-button cycle of the CSS (src/Costumes.asm select_,
 * src/Character.asm default_costume.table).
 *
 * `Character.default_costume.table` is 8 bytes per character id: four costume ids (one per C button), then the red,
 * blue and green team costume ids. `Costumes.select_.num_costumes` holds, per id, the highest native costume id of the
 * model (0-based). C right/left step the costume id through 0..max with wrap, C up/down step the shade through 0..2.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "mn/mnplayers/mnremixcostumes.h"

#define MNREMIX_DEFAULT_COSTUME_ROW_SIZE 8
#define MNREMIX_DEFAULT_COSTUME_TEAM_OFFSET 4
#define MNREMIX_COSTUME_TEAM_COUNT 3
#define MNREMIX_COSTUME_BUTTON_COUNT 4

/** Byte at a Remix RAM address. The image keeps every u32 word in host order, so bytes are taken from the word. */
static sb32 mnRemixCostumeReadByte(u32 ram_addr, u8 *value)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr & ~3U);

    if (word == NULL)
    {
        return FALSE;
    }
    *value = (u8)(*word >> ((3 - (ram_addr & 3)) * 8));

    return TRUE;
}

static sb32 mnRemixCostumeReadDefault(s32 fkind, s32 index, s32 *costume)
{
    u8 value;

    if ((fkind < 0) || !mnRemixCostumeReadByte(REMIX_SYM_Character__default_costume__table + (fkind * MNREMIX_DEFAULT_COSTUME_ROW_SIZE) + index, &value))
    {
        return FALSE;
    }
    *costume = value;

    return TRUE;
}

/** Costume id of the C button `button` (0 up, 1 right, 2 down, 3 left) from the default costume table. */
sb32 mnRemixCostumeGetDefaultRoyal(s32 fkind, s32 button, s32 *costume)
{
    if ((button < 0) || (button >= MNREMIX_COSTUME_BUTTON_COUNT))
    {
        return FALSE;
    }
    return mnRemixCostumeReadDefault(fkind, button, costume);
}

/** Costume id of team `team` (0 red, 1 blue, 2 green) from the default costume table. */
sb32 mnRemixCostumeGetDefaultTeam(s32 fkind, s32 team, s32 *costume)
{
    if ((team < 0) || (team >= MNREMIX_COSTUME_TEAM_COUNT))
    {
        return FALSE;
    }
    return mnRemixCostumeReadDefault(fkind, MNREMIX_DEFAULT_COSTUME_TEAM_OFFSET + team, costume);
}

/** Highest native costume id of `fkind` (Costumes.select_.num_costumes). */
s32 mnRemixCostumeGetMaxID(s32 fkind)
{
    u8 value;

    if ((fkind < 0) || !mnRemixCostumeReadByte(REMIX_SYM_Costumes__select___num_costumes + fkind, &value))
    {
        return 0;
    }
    return value;
}

/** Applies one C button press: right/left cycle the costume (shade resets), up/down cycle the shade. */
void mnRemixCostumeApplyButton(s32 fkind, s32 button, s32 *costume, s32 *shade)
{
    s32 max_costume = mnRemixCostumeGetMaxID(fkind);

    switch (button)
    {
    case MNREMIX_COSTUME_BUTTON_RIGHT:
        *shade = 0;
        *costume = (*costume < max_costume) ? (*costume + 1) : 0;
        break;

    case MNREMIX_COSTUME_BUTTON_LEFT:
        *shade = 0;
        *costume = (*costume > 0) ? (*costume - 1) : max_costume;
        break;

    case MNREMIX_COSTUME_BUTTON_UP:
        *shade = (*shade < MNREMIX_SHADE_MAX) ? (*shade + 1) : 0;
        break;

    case MNREMIX_COSTUME_BUTTON_DOWN:
        *shade = (*shade > 0) ? (*shade - 1) : MNREMIX_SHADE_MAX;
        break;
    }
}

#endif
