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
#include <sys/utils.h>
#include "ft/ftremix/ftremixpartimage.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "mn/mnplayers/mnremixcostumes.h"

#define MNREMIX_DEFAULT_COSTUME_ROW_SIZE 8
#define MNREMIX_DEFAULT_COSTUME_TEAM_OFFSET 4
#define MNREMIX_COSTUME_TEAM_COUNT 3
#define MNREMIX_COSTUME_BUTTON_COUNT 4
#define MNREMIX_COSTUME_RANDOM_ATTEMPTS 64

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

/**
 * Costume a Random pick gets (Costumes.get_random_legal_costume_): a roll of get_random_int_(max) over the vanilla ids, the skipped ids
 * and the extra costumes (0 to max - 1, so the last id is never rolled), rolled again while it lands on a skipped id. A character
 * with no extra costumes rolls over its vanilla ids only.
 */
s32 mnRemixCostumeGetRandomLegal(s32 fkind)
{
    s32 max_costume = ftRemixPartImageGetMaxCostumeID(fkind);
    s32 attempt;

    if (max_costume <= 0)
    {
        return 0;
    }
    for (attempt = 0; attempt < MNREMIX_COSTUME_RANDOM_ATTEMPTS; attempt++)
    {
        s32 costume = syUtilsRandIntRange(max_costume);

        if (ftRemixPartImageIsLegalRandomCostume(fkind, costume) != FALSE)
        {
            return costume;
        }
    }
    return 0;
}

/** Applies one C button press: right/left cycle the costume (shade resets, skipped ids and extra costumes included), up/down cycle the shade. */
void mnRemixCostumeApplyButton(s32 fkind, s32 button, s32 *costume, s32 *shade)
{
    switch (button)
    {
    case MNREMIX_COSTUME_BUTTON_RIGHT:
        *shade = 0;
        *costume = ftRemixPartImageGetNextCostume(fkind, *costume);
        break;

    case MNREMIX_COSTUME_BUTTON_LEFT:
        *shade = 0;
        *costume = ftRemixPartImageGetPrevCostume(fkind, *costume);
        break;

    case MNREMIX_COSTUME_BUTTON_UP:
        *shade = (*shade < MNREMIX_SHADE_MAX) ? (*shade + 1) : 0;
        break;

    case MNREMIX_COSTUME_BUTTON_DOWN:
        *shade = (*shade > 0) ? (*shade - 1) : MNREMIX_SHADE_MAX;
        break;
    }
}

void mnRemixCostumeApplyPartImage(GObj *fighter_gobj)
{
    FTStruct *fp;

    if (fighter_gobj == NULL)
    {
        return;
    }
    fp = ftGetStruct(fighter_gobj);

    if (fp != NULL)
    {
        ftRemixPartImageApplyFighter(fp);
    }
}

#endif
