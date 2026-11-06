/**
 * mnremixtwelvecbdkmoderules.c - letter patterns of the DK Mode easter egg, see mnremixtwelvecbdkmoderules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixtwelvecbdkmoderules.h"

/* DKMode.asm dk_mode_table_K, dk_mode_table_D and dk_mode_table_L. */
#define MNREMIX_TCB_DK_MODE_MASK_K 0xAACCAAu
#define MNREMIX_TCB_DK_MODE_MASK_D 0xCCAACCu
#define MNREMIX_TCB_DK_MODE_MASK_L 0x88EEEEu

int mnRemixTwelveCBDKModeMatchesLetter(const int *fkinds, unsigned int letter_mask)
{
    int slot;

    for (slot = 0; slot < MNREMIX_TCB_DK_MODE_SLOTS; slot++)
    {
        int is_different = (fkinds[slot] != fkinds[0]) ? 1 : 0;
        int must_differ = (int)((letter_mask >> slot) & 1u);

        if (is_different != must_differ)
        {
            return 0;
        }
    }
    return 1;
}

int mnRemixTwelveCBDKModeSelect(const int *player_one_fkinds, const int *player_two_fkinds)
{
    if (mnRemixTwelveCBDKModeMatchesLetter(player_two_fkinds, MNREMIX_TCB_DK_MODE_MASK_K) == 0)
    {
        return MNREMIX_TCB_DK_MODE_OFF;
    }
    if (mnRemixTwelveCBDKModeMatchesLetter(player_one_fkinds, MNREMIX_TCB_DK_MODE_MASK_D) != 0)
    {
        return MNREMIX_TCB_DK_MODE_DK;
    }
    if (mnRemixTwelveCBDKModeMatchesLetter(player_one_fkinds, MNREMIX_TCB_DK_MODE_MASK_L) != 0)
    {
        return MNREMIX_TCB_DK_MODE_LANKY;
    }
    return MNREMIX_TCB_DK_MODE_OFF;
}

#endif
