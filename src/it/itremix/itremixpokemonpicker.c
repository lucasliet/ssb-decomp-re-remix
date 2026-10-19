/**
 * itremixpokemonpicker.c - Smash Remix Pokemon item picker (Item.asm whos_that_pokemon at RAM 0x80173260 and
 * whos_that_pokemon_setup at RAM 0x8016F218, Toggles.asm entry_pokemon_*).
 *
 * whos_that_pokemon replaces the Mew check and the monster roll of itMainMakeMonster. The asm jumps back to the vanilla
 * tail at 0x80173348 after its spawn, so the caller keeps the vel setup before the hook and the owner fields after it.
 * The picker keeps its own state (last picks and the roll range) in sITRemixPokemonSlots, reset by
 * whos_that_pokemon_setup, which replaces the vanilla itManagerInitMonsterVars body. Vanilla gITManagerMonsterData is not
 * read or written in Remix.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include <it/item.h>
#include <it/itmanager.h>
#include <sys/objdef.h>
#include <sys/utils.h>

/* asm POKEMON_COUNT: Onix (0x20) to Clefairy (0x2B). Mew (0x2C) is outside the roll. */
#define ITREMIX_POKEMON_COMMON_COUNT 12
/* asm _mew_selected: the roll range stops shrinking at this value */
#define ITREMIX_POKEMON_MIN_RANGE 10
/* asm: picks are recorded and excluded from the roll only with this many Pokemon ON */
#define ITREMIX_POKEMON_RECENT_MIN_ON 3
/* asm: Global.get_random_int_(151) rolls Mew (1 in 151) */
#define ITREMIX_POKEMON_MEW_ROLL 151
#define ITREMIX_POKEMON_SLOT_LAST 0
#define ITREMIX_POKEMON_SLOT_PREV 1
/* asm POKEMON_SLOTS: candidate slots start here and hold the compacted list of Pokemon kinds that can be rolled */
#define ITREMIX_POKEMON_SLOT_FIRST 2
#define ITREMIX_POKEMON_SLOT_RANGE 0x14
#define ITREMIX_POKEMON_SLOTS_NUM 0x15
/* asm whos_that_pokemon_setup: 0xFF marks an empty pick, no kind is 0xFF */
#define ITREMIX_POKEMON_SLOT_EMPTY 0xFF

static u8 sITRemixPokemonSlots[ITREMIX_POKEMON_SLOTS_NUM];

/* Toggle of the Pokemon at asm pokemon_table[index] (0 is Onix, 11 is Clefairy). */
static sb32 itRemixPokemonIsOn(s32 index)
{
    switch (index)
    {
        case 0: return gmRemixToggle(pokemon_onix);
        case 1: return gmRemixToggle(pokemon_snorlax);
        case 2: return gmRemixToggle(pokemon_goldeen);
        case 3: return gmRemixToggle(pokemon_meowth);
        case 4: return gmRemixToggle(pokemon_charizard);
        case 5: return gmRemixToggle(pokemon_beedrill);
        case 6: return gmRemixToggle(pokemon_blastoise);
        case 7: return gmRemixToggle(pokemon_chansey);
        case 8: return gmRemixToggle(pokemon_starmie);
        case 9: return gmRemixToggle(pokemon_hitmonlee);
        case 10: return gmRemixToggle(pokemon_koffing);
        case 11: return gmRemixToggle(pokemon_clefairy);
    }
    return FALSE;
}

/* asm pokem_on: number of the twelve rollable Pokemon that are ON (Mew excluded). */
static s32 itRemixPokemonCountOn(void)
{
    s32 index;
    s32 on_num = 0;

    for (index = 0; index < ITREMIX_POKEMON_COMMON_COUNT; index++)
    {
        on_num += itRemixPokemonIsOn(index);
    }
    return on_num;
}

/* asm random_mew: the 1 in 151 Mew roll, skipped when Mew was one of the last two picks. */
static sb32 itRemixPokemonRollMew(void)
{
    if (syUtilsRandIntRange(ITREMIX_POKEMON_MEW_ROLL) != 0)
    {
        return FALSE;
    }
    return (sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_LAST] != nITKindMew) &&
        (sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_PREV] != nITKindMew);
}

/*
 * asm no_mew loop: writes the ON Pokemon, in kind order, into the candidate slots. The last and second last picks are
 * skipped and do not take a slot, so the slots stay compacted; slots past the last written one keep older values, as the
 * asm does.
 */
static void itRemixPokemonStoreCandidates(void)
{
    s32 kind;
    s32 slot = ITREMIX_POKEMON_SLOT_FIRST;

    for (kind = nITKindIwark; kind < nITKindMew; kind++)
    {
        if ((kind == sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_LAST]) ||
            (kind == sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_PREV]))
        {
            continue;
        }
        sITRemixPokemonSlots[slot] = itRemixPokemonIsOn(kind - nITKindIwark) ? kind : 0;
        slot++;
    }
}

/* asm get_pokemon_random_offset: the first roll uses the stored range, re-rolls use 12 with fewer than 3 Pokemon ON. */
static s32 itRemixPokemonRerollRange(s32 on_num)
{
    if (on_num < ITREMIX_POKEMON_RECENT_MIN_ON)
    {
        return ITREMIX_POKEMON_COMMON_COUNT;
    }
    return sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_RANGE];
}

/* asm no_mew: rolls one ON Pokemon kind, re-rolling until the slot is not empty. */
static s32 itRemixPokemonRollCommon(s32 on_num)
{
    s32 index;
    s32 kind;

    itRemixPokemonStoreCandidates();
    index = syUtilsRandIntRange(sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_RANGE]);
    kind = sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_FIRST + index];
    while (kind == 0)
    {
        index = syUtilsRandIntRange(itRemixPokemonRerollRange(on_num));
        kind = sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_FIRST + index];
    }
    return kind;
}

/* asm whos_that_pokemon branches: no Pokemon ON gives Mew or a Poke Ball, otherwise the Mew roll or a common roll. */
static s32 itRemixPokemonPickKind(s32 on_num, sb32 is_mew_on)
{
    if (on_num == 0)
    {
        return is_mew_on ? nITKindMew : nITKindMBall;
    }
    if (is_mew_on && itRemixPokemonRollMew())
    {
        return nITKindMew;
    }
    return itRemixPokemonRollCommon(on_num);
}

/*
 * asm _mew_selected: lowers the roll range down to 10 on every spawn and, with 3 or more Pokemon ON, shifts the pick into
 * the last picks. Both steps run for Mew, a Poke Ball and common picks alike.
 */
static void itRemixPokemonRecordPick(s32 on_num, s32 kind)
{
    if (sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_RANGE] != ITREMIX_POKEMON_MIN_RANGE)
    {
        sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_RANGE]--;
    }
    if (on_num < ITREMIX_POKEMON_RECENT_MIN_ON)
    {
        return;
    }
    sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_PREV] = sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_LAST];
    sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_LAST] = kind;
}

/** whos_that_pokemon (Item.asm 0x80173260): spawns the Pokemon picked by the Poke Ball roll, replacing the vanilla Mew check and roll. */
GObj* itRemixPokemonMakeMonster(GObj *item_gobj, Vec3f *vel)
{
    s32 on_num = itRemixPokemonCountOn();
    sb32 is_mew_on = gmRemixToggle(pokemon_mew);
    s32 kind = itRemixPokemonPickKind(on_num, is_mew_on);

    itRemixPokemonRecordPick(on_num, kind);
    return itManagerMakeItemKind(
        item_gobj,
        kind,
        &DObjGetStruct(item_gobj)->translate.vec.f,
        vel,
        (ITEM_FLAG_COLLPROJECT | ITEM_FLAG_PARENT_ITEM)
    );
}

/** whos_that_pokemon_setup (Item.asm 0x8016F218): clears the last picks and sets the roll range to 12; replaces itManagerInitMonsterVars. */
void itRemixPokemonResetPicker(void)
{
    sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_LAST] = ITREMIX_POKEMON_SLOT_EMPTY;
    sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_PREV] = ITREMIX_POKEMON_SLOT_EMPTY;
    sITRemixPokemonSlots[ITREMIX_POKEMON_SLOT_RANGE] = ITREMIX_POKEMON_COMMON_COUNT;
}

#endif
