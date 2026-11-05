/**
 * ftremixshield.c - Smash Remix Shield option (src/Shield.asm color_fix_, css/Shield.asm): the env colour of a fighter shield.
 *
 * The colours 1 to 15 are the rows of Shield.asm table_custom (Red to White). Default (0), Vanilla (16) and the shield damage
 * flash keep the vanilla colour of efManagerShieldProcDisplay.
 *
 * The Costume option (17) takes the colour index of the fighter costume from Character.costume_shield_color
 * (ftremixshieldcostume.c).
 *
 * Not portable, Yoshi egg colour (Shield.asm alter_yoshi_egg_color_, patches 0x801013A8, 0x8010318C, 0x8015EAF4, 0x8014D080,
 * 0x8014C984, 0x80102F5C): the asm rewrites the egg display lists (a 0xDE000000 branch over the TLUT load at +0x50, +0x58 or
 * +0x48) and calls Render.MODEL_PART_IMAGE_INIT_ to attach a model part image with a 16 entry palette array (Shield.asm
 * table_palettes, table_entry_palettes: 32 literal TLUTs), then writes the palette index as a float at +0x88. The port has no
 * model part image support (no MODEL_PART_IMAGE or palette array code in decomp/src or port), and the egg display lists are
 * relocatable file data. Character.yoshi_egg_color.table is never filled by any Remix file, so the index is 0 (the unchanged
 * palette) for every option but Vanilla (also 0) and Costume, which uses the costume index. The egg keeps the vanilla colour.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <gm/gmremixpanel.h>

#define FTREMIX_SHIELD_FIRST_CUSTOM 1
#define FTREMIX_SHIELD_LAST_CUSTOM 15
#define FTREMIX_SHIELD_COSTUME 17

static const SYColorRGB sFTRemixShieldColors[FTREMIX_SHIELD_LAST_CUSTOM] =
{
    { 0xFF, 0x00, 0x00 }, /* Red */
    { 0xFF, 0x80, 0x00 }, /* Orange */
    { 0xFF, 0xFF, 0x00 }, /* Yellow */
    { 0x80, 0xFF, 0x00 }, /* Lime */
    { 0x00, 0xFF, 0x00 }, /* Green */
    { 0x00, 0xFF, 0x80 }, /* Turquoise */
    { 0x00, 0xFF, 0xFF }, /* Cyan */
    { 0x00, 0x80, 0xFF }, /* Azure */
    { 0x00, 0x00, 0xFF }, /* Blue */
    { 0x80, 0x00, 0xFF }, /* Purple */
    { 0xFF, 0x00, 0xFF }, /* Magenta */
    { 0xFF, 0x00, 0x80 }, /* Pink */
    { 0xA8, 0x40, 0x20 }, /* Brown */
    { 0x00, 0x00, 0x00 }, /* Black */
    { 0xA0, 0xA0, 0xA0 }  /* White */
};

/** Env colour of the Shield.asm table_custom row, where the row 0 (Default) is the black 0x00000000. */
static SYColorRGB ftRemixShieldGetTableColor(s32 color_index)
{
    SYColorRGB black = { 0x00, 0x00, 0x00 };

    if (color_index == 0)
    {
        return black;
    }
    return sFTRemixShieldColors[color_index - FTREMIX_SHIELD_FIRST_CUSTOM];
}

/** Costume option (Shield.asm color_fix_ _costume): the Character.costume_shield_color row of the fighter. */
static SYColorRGB ftRemixShieldGetCostumeEnvColor(const FTStruct *fp, SYColorRGB vanilla)
{
    s32 color_index = ftRemixShieldGetCostumeColorIndex(fp);

    if ((color_index < 0) || (color_index > FTREMIX_SHIELD_LAST_CUSTOM))
    {
        return vanilla;
    }
    return ftRemixShieldGetTableColor(color_index);
}

/**
 * Env colour of the shield of a fighter (asm Shield.color_fix_): the Shield option colour of its port when it is 1 to 15,
 * the colour of its costume when it is Costume, else the vanilla colour passed in. The shield damage flash (is_damage_shield)
 * and a shield with no fighter always keep the vanilla colour.
 */
SYColorRGB ftRemixShieldGetEnvColor(GObj *fighter_gobj, sb32 is_damage_shield, SYColorRGB vanilla)
{
    FTStruct *fp;
    s32 option;

    if ((fighter_gobj == NULL) || is_damage_shield)
    {
        return vanilla;
    }
    fp = ftGetStruct(fighter_gobj);
    option = gmRemixPanel(SHIELD, fp->player);

    if (option == FTREMIX_SHIELD_COSTUME)
    {
        return ftRemixShieldGetCostumeEnvColor(fp, vanilla);
    }
    if ((option < FTREMIX_SHIELD_FIRST_CUSTOM) || (option > FTREMIX_SHIELD_LAST_CUSTOM))
    {
        return vanilla;
    }
    return ftRemixShieldGetTableColor(option);
}

#endif
