/**
 * ftremixmagichatrules.c - pure rules of the '???' Magic Kirby Hat, see ftremixmagichatrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixmagichatrules.h"
#include "remix/remix_roster_data.h"

#define FTREMIXMAGICHAT_TABLE_SIZE 43
#define FTREMIXMAGICHAT_AMMO_OFFSET 0x0AE0
#define FTREMIXMAGICHAT_AMMO_OFFSET_DONKEY 0x0AE8

/* Action.KIRBY ids (Action.asm:1162) that bound the copied specials, and Kirby.Action.GND_NSP_Ground. */
#define FTREMIXMAGICHAT_STATUS_MARIO_FIREBALL 0xE7
#define FTREMIXMAGICHAT_STATUS_STARING1 0xFA
#define FTREMIXMAGICHAT_STATUS_LIGHTNING 0xFC
#define FTREMIXMAGICHAT_STATUS_FINAL_CUTTER 0x100
#define FTREMIXMAGICHAT_STATUS_STONE_START 0x104
#define FTREMIXMAGICHAT_STATUS_REMIX_FIRST 0x12F

/* magic_kirby_hat_charge: fighter id and full charge of each row. */
static const int sFTRemixMagicHatChargeFighters[FTREMIXMAGICHAT_CHARGE_SLOTS] =
{
    REMIX_ID_DONKEY, REMIX_ID_SAMUS, REMIX_ID_DSAMUS, REMIX_ID_MTWO, REMIX_ID_SHEIK, REMIX_ID_PEPPY
};
static const int sFTRemixMagicHatChargeMaxes[FTREMIXMAGICHAT_CHARGE_SLOTS] = { 10, 7, 7, 7, 6, 5 };

/* KirbyHats.asm spawn_with_table_. */
static const int sFTRemixMagicHatFighters[FTREMIXMAGICHAT_TABLE_SIZE] =
{
    REMIX_ID_KIRBY, REMIX_ID_MARIO, REMIX_ID_FOX, REMIX_ID_DONKEY, REMIX_ID_SAMUS, REMIX_ID_LUIGI, REMIX_ID_LINK,
    REMIX_ID_YOSHI, REMIX_ID_CAPTAIN, REMIX_ID_PIKACHU, REMIX_ID_JIGGLY, REMIX_ID_NESS, REMIX_ID_FALCO, REMIX_ID_GND,
    REMIX_ID_YLINK, REMIX_ID_DRM, REMIX_ID_WARIO, REMIX_ID_BOWSER, REMIX_ID_WOLF, REMIX_ID_CONKER, REMIX_ID_MTWO,
    REMIX_ID_MARTH, REMIX_ID_SONIC, REMIX_ID_SHEIK, REMIX_ID_MARINA, REMIX_ID_DEDEDE, REMIX_ID_GOEMON, REMIX_ID_BANJO,
    REMIX_ID_CRASH, REMIX_ID_PEACH, REMIX_ID_DSAMUS, REMIX_ID_LUCAS, REMIX_ID_PEPPY, REMIX_ID_SLIPPY, REMIX_ID_ROY,
    REMIX_ID_DRL, REMIX_ID_LANKY, REMIX_ID_DRAGONKING, REMIX_ID_EBI, REMIX_ID_PIANO, REMIX_ID_SSONIC, REMIX_ID_NONE,
    REMIX_ID_KIRBY
};

int ftRemixMagicHatShouldRestore(int status_id)
{
    if (status_id >= FTREMIXMAGICHAT_STATUS_REMIX_FIRST)
    {
        return 0;
    }
    if (status_id < FTREMIXMAGICHAT_STATUS_MARIO_FIREBALL)
    {
        return 1;
    }
    if (status_id < FTREMIXMAGICHAT_STATUS_STARING1)
    {
        return 0;
    }
    if (status_id < FTREMIXMAGICHAT_STATUS_LIGHTNING)
    {
        return 1;
    }
    if (status_id < FTREMIXMAGICHAT_STATUS_FINAL_CUTTER)
    {
        return 0;
    }
    return status_id < FTREMIXMAGICHAT_STATUS_STONE_START;
}

int ftRemixMagicHatChargeSlot(int fkind)
{
    int slot;

    for (slot = 0; slot < FTREMIXMAGICHAT_CHARGE_SLOTS; slot++)
    {
        if (sFTRemixMagicHatChargeFighters[slot] == fkind)
        {
            return slot;
        }
    }
    return -1;
}

int ftRemixMagicHatChargeMax(int slot)
{
    if ((slot < 0) || (slot >= FTREMIXMAGICHAT_CHARGE_SLOTS))
    {
        return 0;
    }
    return sFTRemixMagicHatChargeMaxes[slot];
}

int ftRemixMagicHatAmmoOffset(int fkind)
{
    return (fkind == REMIX_ID_DONKEY) ? FTREMIXMAGICHAT_AMMO_OFFSET_DONKEY : FTREMIXMAGICHAT_AMMO_OFFSET;
}

int ftRemixMagicHatFighterFromRoll(int roll)
{
    if ((roll < 0) || (roll >= FTREMIXMAGICHAT_TABLE_SIZE))
    {
        return REMIX_ID_KIRBY;
    }
    return sFTRemixMagicHatFighters[roll];
}

#endif
