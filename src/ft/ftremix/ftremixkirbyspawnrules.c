/**
 * ftremixkirbyspawnrules.c - pure rules of the CSS Kirby Hat and of the power flags a fighter spawn sets, see
 * ftremixkirbyspawnrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixkirbyspawnrules.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_KIRBYSPAWN_HAT_WOLF 0x1A
#define FTREMIX_KIRBYSPAWN_HAT_CONKER 0x19
#define FTREMIX_KIRBYSPAWN_HAT_MARTH 0x1C
#define FTREMIX_KIRBYSPAWN_HAT_SLIPPY 0x23
#define FTREMIX_KIRBYSPAWN_HAT_PEPPY 0x24
#define FTREMIX_KIRBYSPAWN_HAT_GOEMON 0x22
#define FTREMIX_KIRBYSPAWN_HAT_EBISUMARU 0x28
#define FTREMIX_KIRBYSPAWN_HAT_ROY 0x2C
#define FTREMIX_KIRBYSPAWN_HAT_LANKY 0x2E

#define FTREMIX_KIRBYSPAWN_GUN_WOLF 0x1D830
#define FTREMIX_KIRBYSPAWN_GUN_CONKER 0x1D860
#define FTREMIX_KIRBYSPAWN_GUN_MARTH 0x1D890
#define FTREMIX_KIRBYSPAWN_GUN_SLIPPY 0x1D8C0
#define FTREMIX_KIRBYSPAWN_GUN_PEPPY 0x1D8F0
#define FTREMIX_KIRBYSPAWN_GUN_GOEMON 0x1D920
#define FTREMIX_KIRBYSPAWN_GUN_EBISUMARU 0x1D950
#define FTREMIX_KIRBYSPAWN_GUN_ROY 0x1D980
#define FTREMIX_KIRBYSPAWN_GUN_LANKY 0x1D9B0

/* KirbyHats.asm spawn_with_table_: Remix fighter id of each CSS Kirby Hat value 0..41 (Random, 42, is rolled by the caller). */
static const unsigned char sFTRemixKirbySpawnPanelFighter[FTREMIX_KIRBYSPAWN_PANEL_MAGIC + 1] = {
    REMIX_ID_KIRBY, REMIX_ID_MARIO, REMIX_ID_FOX, REMIX_ID_DONKEY, REMIX_ID_SAMUS, REMIX_ID_LUIGI, REMIX_ID_LINK,
    REMIX_ID_YOSHI, REMIX_ID_CAPTAIN, REMIX_ID_PIKACHU, REMIX_ID_JIGGLY, REMIX_ID_NESS, REMIX_ID_FALCO, REMIX_ID_GND,
    REMIX_ID_YLINK, REMIX_ID_DRM, REMIX_ID_WARIO, 0x34, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3E, REMIX_ID_MARINA,
    REMIX_ID_DEDEDE, REMIX_ID_GOEMON, REMIX_ID_BANJO, REMIX_ID_CRASH, REMIX_ID_PEACH, 0x22, 0x26, REMIX_ID_PEPPY,
    REMIX_ID_SLIPPY, REMIX_ID_ROY, REMIX_ID_DRL, REMIX_ID_LANKY, REMIX_ID_DRAGONKING, REMIX_ID_EBI, 0x36, REMIX_ID_SSONIC,
    REMIX_ID_NONE,
};

int ftRemixKirbySpawnPanelFighter(int panel_value, int random_roll)
{
    if (panel_value == FTREMIX_KIRBYSPAWN_PANEL_RANDOM)
    {
        if ((random_roll < 0) || (random_roll >= FTREMIX_KIRBYSPAWN_PANEL_FIGHTER_COUNT))
        {
            return REMIX_ID_KIRBY;
        }
        return sFTRemixKirbySpawnPanelFighter[1 + random_roll];
    }
    if ((panel_value < FTREMIX_KIRBYSPAWN_PANEL_NONE) || (panel_value > FTREMIX_KIRBYSPAWN_PANEL_MAGIC))
    {
        return REMIX_ID_KIRBY;
    }
    return sFTRemixKirbySpawnPanelFighter[panel_value];
}

int ftRemixKirbySpawnMagicHatStartsOn(int spawn_copy_kind, int panel_value)
{
    return ((spawn_copy_kind == REMIX_ID_NONE) || (panel_value == FTREMIX_KIRBYSPAWN_PANEL_MAGIC)) ? 1 : 0;
}

int ftRemixKirbySpawnTauntLosesPower(int panel_value)
{
    return (panel_value == FTREMIX_KIRBYSPAWN_PANEL_NONE) ? 1 : 0;
}

unsigned int ftRemixKirbyGunHatOffset(int hat_id)
{
    switch (hat_id)
    {
    case FTREMIX_KIRBYSPAWN_HAT_WOLF: return FTREMIX_KIRBYSPAWN_GUN_WOLF;
    case FTREMIX_KIRBYSPAWN_HAT_CONKER: return FTREMIX_KIRBYSPAWN_GUN_CONKER;
    case FTREMIX_KIRBYSPAWN_HAT_MARTH: return FTREMIX_KIRBYSPAWN_GUN_MARTH;
    case FTREMIX_KIRBYSPAWN_HAT_SLIPPY: return FTREMIX_KIRBYSPAWN_GUN_SLIPPY;
    case FTREMIX_KIRBYSPAWN_HAT_PEPPY: return FTREMIX_KIRBYSPAWN_GUN_PEPPY;
    case FTREMIX_KIRBYSPAWN_HAT_GOEMON: return FTREMIX_KIRBYSPAWN_GUN_GOEMON;
    case FTREMIX_KIRBYSPAWN_HAT_EBISUMARU: return FTREMIX_KIRBYSPAWN_GUN_EBISUMARU;
    case FTREMIX_KIRBYSPAWN_HAT_ROY: return FTREMIX_KIRBYSPAWN_GUN_ROY;
    case FTREMIX_KIRBYSPAWN_HAT_LANKY: return FTREMIX_KIRBYSPAWN_GUN_LANKY;
    default: return 0;
    }
}

#endif
