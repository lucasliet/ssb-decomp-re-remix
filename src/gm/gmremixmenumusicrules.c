/**
 * gmremixmenumusicrules.c - pure rules of the Menu Music trap easter egg, see gmremixmenumusicrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmenumusicrules.h"

int gmRemixMenuMusicRulesCountTrapPress(int count)
{
    int presses = count + 1;

    if (presses == 0)
    {
        presses = 1;
    }
    if (presses >= GMREMIX_MENU_MUSIC_TRAP_PRESSES)
    {
        return GMREMIX_MENU_MUSIC_TRAP_ACTIVE;
    }
    return presses;
}

int gmRemixMenuMusicRulesTrapIsActive(int count)
{
    return count < 0;
}

int gmRemixMenuMusicRulesCountAuditionPress(int raw_value, int resolved_value, int count)
{
    if (resolved_value != GMREMIX_MENU_MUSIC_RULES_FILESELECT_SM64)
    {
        return 0;
    }
    if (raw_value != GMREMIX_MENU_MUSIC_RULES_FILESELECT_SM64)
    {
        count = 0;
    }
    return gmRemixMenuMusicRulesCountTrapPress(count);
}

int gmRemixMenuMusicRulesRandomStep(int current_bgm)
{
    if ((current_bgm == GMREMIX_MENU_MUSIC_RULES_BGM_MAIN) || (current_bgm == GMREMIX_MENU_MUSIC_RULES_BGM_CHARACTER_SELECT))
    {
        return GMREMIX_MENU_MUSIC_RANDOM_SCREEN_TRACK;
    }
    if ((current_bgm == GMREMIX_MENU_MUSIC_RULES_BGM_DATA) || (current_bgm < 0))
    {
        return GMREMIX_MENU_MUSIC_RANDOM_REROLL;
    }
    return GMREMIX_MENU_MUSIC_RANDOM_KEEP;
}

int gmRemixMenuMusicRulesSettingsCloseResets(int play_music, int current_bgm)
{
    return (play_music == 0) || (current_bgm < 0);
}

int gmRemixMenuMusicRulesVolumeResets(int play_music, int current_bgm)
{
    return (play_music != 0) && (current_bgm < 0);
}

int gmRemixMenuMusicRulesResultsBgmShouldReplay(int is_stopped, int is_exiting)
{
    return is_stopped && !is_exiting;
}

#endif
