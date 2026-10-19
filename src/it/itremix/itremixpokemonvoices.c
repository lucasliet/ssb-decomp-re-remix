/**
 * itremixpokemonvoices.c - Smash Remix Japanese Pokemon voices (Item.asm pokemon_variant_sfx, Toggles.asm
 * entry_pokemon_voices).
 *
 * Values of entry_pokemon_voices: 0 DEFAULT, 1 JAPANESE, 2 RANDOM. RANDOM swaps on RNG bit 3, so each voice has a 50%
 * chance of the Japanese version. The ten Pokemon voice sites call itRemixPokemonVoiceFgm on their FGM id.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include <sys/utils.h>

#define ITREMIX_POKEMON_VOICE_JAPANESE 1
#define ITREMIX_POKEMON_VOICE_RANDOM 2
/* asm: the RNG word is tested with andi 0x8 (the word ends in 0 or 8, bit 3 decides) */
#define ITREMIX_POKEMON_VOICE_RNG_BIT 0x8

typedef struct ITRemixPokemonVoicePair
{
    u16 us_fgm;
    u16 jp_fgm;
} ITRemixPokemonVoicePair;

/*
 * FGM ids from FGM.asm (pokemon.u and pokemon.j). Saffron Chansey has no Japanese voice of its own: the asm plays
 * Chansey J for it (Item.asm line 2637).
 */
static const ITRemixPokemonVoicePair sITRemixPokemonVoicePairs[] =
{
    { 0x0138, 0x0553 }, /* SNORLAX_1 */
    { 0x0137, 0x0554 }, /* SNORLAX_2 */
    { 0x0143, 0x0551 }, /* GOLDEEN */
    { 0x0139, 0x054D }, /* BLASTOISE */
    { 0x013A, 0x054E }, /* CHANSEY */
    { 0x0135, 0x0552 }, /* KOFFING */
    { 0x013C, 0x0550 }, /* CLEFAIRY */
    { 0x0228, 0x0555 }, /* SAF_VENUSAUR */
    { 0x0229, 0x054F }, /* SAF_CHARMANDER */
    { 0x022A, 0x054E }, /* SAF_CHANSEY */
};

/* asm pokemon_variant_sfx: TRUE when this call should play the Japanese voice. RANDOM reads the RNG word without a roll. */
static sb32 itRemixPokemonVoiceUsesJapanese(void)
{
    s32 mode = gmRemixToggle(pokemon_voices);

    if (mode == ITREMIX_POKEMON_VOICE_JAPANESE)
    {
        return TRUE;
    }
    if (mode == ITREMIX_POKEMON_VOICE_RANDOM)
    {
        return (syUtilsRandSeed() & ITREMIX_POKEMON_VOICE_RNG_BIT) != 0;
    }
    return FALSE;
}

/* Japanese FGM id of a U.S. Pokemon voice, or the id itself when the voice has no Japanese version. */
static u16 itRemixPokemonJapaneseFgm(u16 fgm_id)
{
    s32 index;

    for (index = 0; index < ARRAY_COUNT(sITRemixPokemonVoicePairs); index++)
    {
        if (sITRemixPokemonVoicePairs[index].us_fgm == fgm_id)
        {
            return sITRemixPokemonVoicePairs[index].jp_fgm;
        }
    }
    return fgm_id;
}

/** pokemon_variant_sfx (Item.asm): the FGM id to play at a Pokemon voice site, swapped to Japanese when the setting asks. */
u16 itRemixPokemonVoiceFgm(u16 fgm_id)
{
    if (itRemixPokemonVoiceUsesJapanese() == FALSE)
    {
        return fgm_id;
    }
    return itRemixPokemonJapaneseFgm(fgm_id);
}

#endif
