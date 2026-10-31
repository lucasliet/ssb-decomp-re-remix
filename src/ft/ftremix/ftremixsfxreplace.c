/*
 * Smash Remix SFXReplace.asm: on the Flat Zone stages (Stages.asm FLAT_ZONE and FLAT_ZONE_2) the footstep, landing and
 * hard landing sounds of PlayFGM motion events become the three gaw sounds, and the down bounce sound becomes the hard
 * landing one. The asm has no toggle. The motion event replacement runs before ftRemixJapaneseMotionSoundFgm, as the asm
 * hook at 0x800DF864 runs before the japanese_sfx_ hook at 0x800DF86C.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "gr/grremix/grremixids.h"
#include "sc/scene.h"
#include "sc/scmanager.h"

/* Global.asm current_screen values, the same numbers as the Special Zoom screen checks */
#define FTREMIX_SFX_SCREEN_VS_BATTLE 0x16
#define FTREMIX_SFX_SCREEN_TRAINING_MODE 0x36
#define FTREMIX_SFX_SCREEN_REMIX_MODES 0x77

/* SFXReplace.asm snd_gaw_biip, snd_gaw_boop and snd_gaw_hardland, added to the bank by FGM.asm */
#define FTREMIX_SFX_GAW_BIIP 0x602
#define FTREMIX_SFX_GAW_BOOP 0x603
#define FTREMIX_SFX_GAW_HARDLAND 0x604

/* SFXReplace.asm STEP_SOUND_TABLE, the last entry is the Bowser footstep */
static const u16 sFTRemixSFXStepSounds[] =
{
    0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73,
    0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7C, 0x7D, 0x7E, 0x7F, 0x82,
    0x368
};

/* SFXReplace.asm LAND_SOUND_TABLE */
static const u16 sFTRemixSFXLandSounds[] =
{
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x5A
};

/* SFXReplace.asm HARDLAND_SOUND_TABLE */
static const u16 sFTRemixSFXHardLandSounds[] =
{
    0xC2, 0x121, 0x123, 0x124, 0x126, 0x127, 0x12B
};

/**
 * TRUE when the current screen is a match, where the asm replaces the motion event sounds (_check_if_in_match).
 */
static sb32 ftRemixSFXReplaceIsMatchScreen(void)
{
    s32 screen = gSCManagerSceneData.scene_curr;

    return (screen == FTREMIX_SFX_SCREEN_VS_BATTLE) || (screen == FTREMIX_SFX_SCREEN_TRAINING_MODE) || (screen == FTREMIX_SFX_SCREEN_REMIX_MODES);
}

/**
 * TRUE when the current stage is one of the two Flat Zone stages (_check_stage in both hooks).
 */
static sb32 ftRemixSFXReplaceIsFlatZone(void)
{
    s32 stage = gSCManagerBattleState->gkind;

    return (stage == nGRRemixStage_FLAT_ZONE) || (stage == nGRRemixStage_FLAT_ZONE_2);
}

/**
 * TRUE when fgm_id is one of the first count entries of table.
 */
static sb32 ftRemixSFXReplaceTableHas(const u16 *table, s32 count, u16 fgm_id)
{
    s32 i;

    for (i = 0; i < count; i++)
    {
        if (table[i] == fgm_id)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Returns the replacement of a motion event fgm id in a match on a Flat Zone stage, or fgm_id when no table has it.
 * The asm checks the step table first, then the landing table, then the hard landing table.
 */
static u16 ftRemixSFXReplaceMotionFgm(u16 fgm_id)
{
    if (ftRemixSFXReplaceTableHas(sFTRemixSFXStepSounds, ARRAY_COUNT(sFTRemixSFXStepSounds), fgm_id))
    {
        return FTREMIX_SFX_GAW_BIIP;
    }
    if (ftRemixSFXReplaceTableHas(sFTRemixSFXLandSounds, ARRAY_COUNT(sFTRemixSFXLandSounds), fgm_id))
    {
        return FTREMIX_SFX_GAW_BOOP;
    }
    if (ftRemixSFXReplaceTableHas(sFTRemixSFXHardLandSounds, ARRAY_COUNT(sFTRemixSFXHardLandSounds), fgm_id))
    {
        return FTREMIX_SFX_GAW_HARDLAND;
    }
    return fgm_id;
}

/**
 * Returns the sound value of a PlayFGM motion event after the SFXReplace replacement (sfx_replace, 0x800DF864 in
 * ftMainParseMotionEvent). The result is still unmasked in the asm, and ftRemixJapaneseMotionSoundFgm masks it to 16 bits.
 */
u32 ftRemixSFXReplaceMotionSound(u32 value)
{
    u16 fgm_id = (u16) (value & 0xFFFF);

    if (ftRemixSFXReplaceIsMatchScreen() == FALSE)
    {
        return value;
    }
    if (ftRemixSFXReplaceIsFlatZone() == FALSE)
    {
        return value;
    }
    return ftRemixSFXReplaceMotionFgm(fgm_id);
}

/**
 * Returns the fgm id of the down bounce sound (sfx_replace_down_bounce, 0x80144470 in ftCommonDownBounceUpdateEffects).
 * This hook checks only the stage, not the screen, as in the asm.
 */
u16 ftRemixSFXReplaceDownBounceSound(u16 fgm_id)
{
    if (ftRemixSFXReplaceIsFlatZone() == FALSE)
    {
        return fgm_id;
    }
    return FTREMIX_SFX_GAW_HARDLAND;
}

#endif
