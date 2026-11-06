/**
 * gmremixtoggleactionrules.c - state-free rules of the A-press routines of the Remix settings, see gmremixtoggleactionrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtoggleactionrules.h"

#define GMREMIX_PUNISH_OFF_BLASTCORPS_FGM 0x537

/* Toggles.asm punish_fgm_: FGM id of the values 1 (7% Damage) to 11 (Random), in value order. */
static const int sGMRemixPunishFgms[GMREMIX_PUNISH_MODE_RANDOM] =
{
    0x1AB, 0x1A4, 0x339, 0x4E5, 0x151, 0x437, 0x312, 0x24B, 0x23C, 0x456, 0x03A,
};

int gmRemixToggleActionGetPunishFgm(int punish_mode, int menu_music)
{
    if ((punish_mode >= 1) && (punish_mode <= GMREMIX_PUNISH_MODE_RANDOM))
    {
        return sGMRemixPunishFgms[punish_mode - 1];
    }
    if ((punish_mode == 0) && (menu_music == GMREMIX_PUNISH_MENU_MUSIC_BLASTCORPS))
    {
        return GMREMIX_PUNISH_OFF_BLASTCORPS_FGM;
    }
    return 0;
}

int gmRemixToggleActionGetAuditionDefaultValue(int roll)
{
    if (roll < GMREMIX_AUDITION_CHANCE_64)
    {
        return GMREMIX_AUDITION_VALUE_64;
    }
    return roll - GMREMIX_AUDITION_CHANCE_64 + GMREMIX_AUDITION_VALUE_MELEE;
}

int gmRemixToggleActionGetToggleAllValue(int first_value)
{
    return first_value ^ 1;
}

int gmRemixToggleActionGetPreviewRemembered(int remembered_bgm, int playing_bgm)
{
    if (remembered_bgm != GMREMIX_PREVIEW_IDLE)
    {
        return remembered_bgm;
    }
    return playing_bgm;
}

int gmRemixToggleActionGetPreviewRestoreStep(int remembered_bgm)
{
    if (remembered_bgm == GMREMIX_PREVIEW_IDLE)
    {
        return GMREMIX_PREVIEW_RESTORE_NOTHING;
    }
    if (remembered_bgm < 0)
    {
        return GMREMIX_PREVIEW_RESTORE_STOP;
    }
    return GMREMIX_PREVIEW_RESTORE_PLAY;
}

#endif
