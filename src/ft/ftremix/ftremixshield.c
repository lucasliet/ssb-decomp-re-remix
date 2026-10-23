/**
 * ftremixshield.c - Smash Remix Shield option (src/Shield.asm color_fix_, css/Shield.asm): the env colour of a fighter shield.
 *
 * The colours 1 to 15 are the rows of Shield.asm table_custom (Red to White). Default (0), Vanilla (16) and the shield damage
 * flash keep the vanilla colour of efManagerShieldProcDisplay.
 *
 * REMIX-TODO(Shield): Costume (17) and the Yoshi egg colour are not ported. Costume needs Character.costume_shield_color, whose
 * vanilla rows are literal byte data in src/Character.asm and are not copied here. The egg colour (Shield.asm
 * alter_yoshi_egg_color_) selects an egg palette index through Render.MODEL_PART_IMAGE_INIT_, which the port does not have;
 * efManagerYoshiShieldProcDisplay keeps the vanilla tint.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <gm/gmremixpanel.h>

#define FTREMIX_SHIELD_FIRST_CUSTOM 1
#define FTREMIX_SHIELD_LAST_CUSTOM 15

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

/**
 * Env colour of the shield of a port (asm Shield.color_fix_): the Shield option colour when it is 1 to 15, else the vanilla
 * colour passed in. The shield damage flash (is_damage_shield) always keeps the vanilla colour.
 */
SYColorRGB ftRemixShieldGetEnvColor(s32 port, sb32 is_damage_shield, SYColorRGB vanilla)
{
    s32 option = gmRemixPanel(SHIELD, port);

    if (is_damage_shield || (option < FTREMIX_SHIELD_FIRST_CUSTOM) || (option > FTREMIX_SHIELD_LAST_CUSTOM))
    {
        return vanilla;
    }
    return sFTRemixShieldColors[option - FTREMIX_SHIELD_FIRST_CUSTOM];
}

#endif
