/**
 * gmremixmenumusic.c - Menu Music toggle of the Remix settings (BGM.asm alt_menu_music_, Toggles.asm entry_menu_music).
 *
 * RANDOM ALL picks from the Random Music list without its gate (BGM.asm 635-668), custom MIDI entries included
 * (grremixrandommusic.c). The "do not restart the track already playing" check (BGM.safe_id) is ported in
 * gmRemixMenuMusicKeepsCurrent. FILESELECT_SM64 turns into its trap variant after four presses in a row (Toggles.itsatrap).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmenumusic.h"
#include "gm/gmremixmenumusicrules.h"
#include "gm/gmremixtoggle.h"
#include "gm/gmremixtoggleactionrules.h"
#include "gr/grremix/grremixrandommusic.h"
#include <gm/gmsound.h>
#include <sys/audio.h>
#include <sys/utils.h>

/* Toggles.asm menu_music values, in the order of string_table_menu_music. */
#define GMREMIX_MENU_MUSIC_DEFAULT 0
#define GMREMIX_MENU_MUSIC_64 1
#define GMREMIX_MENU_MUSIC_MELEE 2
#define GMREMIX_MENU_MUSIC_MENU2 3
#define GMREMIX_MENU_MUSIC_BRAWL 4
#define GMREMIX_MENU_MUSIC_GOLDENEYE 5
#define GMREMIX_MENU_MUSIC_MARIOTENNIS 6
#define GMREMIX_MENU_MUSIC_FILESELECT_SM64 7
#define GMREMIX_MENU_MUSIC_BLASTCORPS 8
#define GMREMIX_MENU_MUSIC_DKR 9
#define GMREMIX_MENU_MUSIC_MK64 10
#define GMREMIX_MENU_MUSIC_SBK 11
#define GMREMIX_MENU_MUSIC_MARIOPARTY 12
#define GMREMIX_MENU_MUSIC_MARIOARTIST 13
#define GMREMIX_MENU_MUSIC_PERFECTDARK 14
#define GMREMIX_MENU_MUSIC_CTR 15
#define GMREMIX_MENU_MUSIC_K64 16
#define GMREMIX_MENU_MUSIC_BOMBERMAN 17
#define GMREMIX_MENU_MUSIC_SONIC_R 18
#define GMREMIX_MENU_MUSIC_RANDOM 19
#define GMREMIX_MENU_MUSIC_RANDOM_CLASSICS 20
#define GMREMIX_MENU_MUSIC_RANDOM_ALL 21
#define GMREMIX_MENU_MUSIC_OFF 22

/* Remix BGM ids of the menu tracks (BGM.asm menu.MAIN_*). */
#define GMREMIX_BGM_MAIN_MELEE 71
#define GMREMIX_BGM_MAIN_BRAWL 108
#define GMREMIX_BGM_MAIN_MENU2 224
#define GMREMIX_BGM_MAIN_GOLDENEYE 231
#define GMREMIX_BGM_MAIN_MARIOTENNIS 248
#define GMREMIX_BGM_MAIN_FILESELECT_SM64 259
#define GMREMIX_BGM_MAIN_TRAP_SM64 260
#define GMREMIX_BGM_MAIN_BLASTCORPS 262
#define GMREMIX_BGM_MAIN_DKR 280
#define GMREMIX_BGM_MAIN_MK64 283
#define GMREMIX_BGM_MAIN_SBK 296
#define GMREMIX_BGM_MAIN_MARIOARTIST 307
#define GMREMIX_BGM_MAIN_MARIOPARTY 309
#define GMREMIX_BGM_MAIN_PERFECTDARK 312
#define GMREMIX_BGM_MAIN_CTR 319
#define GMREMIX_BGM_MAIN_K64 338
/* MIDI.id.STAGESELBM64 and MIDI.id.SONIC_R (midi.asm:681, 683), the tracks of Menu Music 17 and 18. */
#define GMREMIX_BGM_MAIN_BOMBERMAN 369
#define GMREMIX_BGM_MAIN_SONIC_R 371

/* Values 2 to 18 play the track listed here, in value order. */
static const u32 sMenuMusicTrackBGM[] =
{
    GMREMIX_BGM_MAIN_MELEE,
    GMREMIX_BGM_MAIN_MENU2,
    GMREMIX_BGM_MAIN_BRAWL,
    GMREMIX_BGM_MAIN_GOLDENEYE,
    GMREMIX_BGM_MAIN_MARIOTENNIS,
    GMREMIX_BGM_MAIN_FILESELECT_SM64,
    GMREMIX_BGM_MAIN_BLASTCORPS,
    GMREMIX_BGM_MAIN_DKR,
    GMREMIX_BGM_MAIN_MK64,
    GMREMIX_BGM_MAIN_SBK,
    GMREMIX_BGM_MAIN_MARIOPARTY,
    GMREMIX_BGM_MAIN_MARIOARTIST,
    GMREMIX_BGM_MAIN_PERFECTDARK,
    GMREMIX_BGM_MAIN_CTR,
    GMREMIX_BGM_MAIN_K64,
    GMREMIX_BGM_MAIN_BOMBERMAN,
    GMREMIX_BGM_MAIN_SONIC_R,
};

/* Toggles.itsatrap: presses of Menu Music on FILESELECT_SM64 in a row, GMREMIX_MENU_MUSIC_TRAP_ACTIVE while the trap plays. */
static s32 sGMRemixMenuMusicTrapCount = 0;

/* BGM.asm 649-653: with no track on in the Random Music list RANDOM ALL plays one of four hidden GameBoy songs. */
#define GMREMIX_MENU_MUSIC_GAMEBOY_SONGS 4
#define GMREMIX_BGM_GAMEBOY_FIRST 0xA2
#define GMREMIX_MENU_MUSIC_NO_RANDOM_TRACK 0xFFFFFFFFU

/* Title screen chance roll out of 100, BGM.asm CHANCE_64 / CHANCE_MELEE / CHANCE_MENU2 / CHANCE_BRAWL. */
#define GMREMIX_MENU_MUSIC_CHANCE_ROLL 100
#define GMREMIX_MENU_MUSIC_CHANCE_64 85
#define GMREMIX_MENU_MUSIC_CHANCE_MELEE 5
#define GMREMIX_MENU_MUSIC_CHANCE_MENU2 5

/**
 * Picks the title screen track of a DEFAULT Menu Music roll.
 *
 * @param default_bgm BGM id kept when the roll lands on the 64 track.
 * @return The BGM id to play.
 */
static u32 gmRemixMenuMusicTitleRoll(u32 default_bgm)
{
    s32 roll = syUtilsRandIntRange(GMREMIX_MENU_MUSIC_CHANCE_ROLL);

    if (roll < GMREMIX_MENU_MUSIC_CHANCE_64)
    {
        return default_bgm;
    }
    if (roll < GMREMIX_MENU_MUSIC_CHANCE_64 + GMREMIX_MENU_MUSIC_CHANCE_MELEE)
    {
        return GMREMIX_BGM_MAIN_MELEE;
    }
    if (roll < GMREMIX_MENU_MUSIC_CHANCE_64 + GMREMIX_MENU_MUSIC_CHANCE_MELEE + GMREMIX_MENU_MUSIC_CHANCE_MENU2)
    {
        return GMREMIX_BGM_MAIN_MENU2;
    }
    return GMREMIX_BGM_MAIN_BRAWL;
}

/**
 * Returns the BGM id of a fixed menu track value (2 to 18). FILESELECT_SM64 plays its trap variant while the itsatrap
 * counter says so (BGM.asm _check_current_trap).
 *
 * @param value Menu Music value, 2 to 18.
 * @param default_bgm BGM id kept when value is not a fixed track value.
 * @return The BGM id to play.
 */
static u32 gmRemixMenuMusicTrackOf(s32 value, u32 default_bgm)
{
    if ((value < GMREMIX_MENU_MUSIC_MELEE) || (value > GMREMIX_MENU_MUSIC_SONIC_R))
    {
        return default_bgm;
    }
    if ((value == GMREMIX_MENU_MUSIC_FILESELECT_SM64) && gmRemixMenuMusicRulesTrapIsActive(sGMRemixMenuMusicTrapCount))
    {
        return GMREMIX_BGM_MAIN_TRAP_SM64;
    }
    return sMenuMusicTrackBGM[value - GMREMIX_MENU_MUSIC_MELEE];
}

/**
 * Picks the Menu Music value a RANDOM press stands for: any value 1 to 18 for RANDOM, 5 to 18 for RANDOM CLASSICS (the 64,
 * MELEE, MENU 2 and BRAWL tracks are excluded as in BGM.asm).
 *
 * @param value GMREMIX_MENU_MUSIC_RANDOM or GMREMIX_MENU_MUSIC_RANDOM_CLASSICS.
 * @return The track value, 1 to 18.
 */
static s32 gmRemixMenuMusicRandomValue(s32 value)
{
    if (value == GMREMIX_MENU_MUSIC_RANDOM)
    {
        return 1 + syUtilsRandIntRange(GMREMIX_MENU_MUSIC_SONIC_R);
    }
    return 5 + syUtilsRandIntRange(GMREMIX_MENU_MUSIC_SONIC_R - 4);
}

/**
 * Picks a menu track for RANDOM or RANDOM CLASSICS.
 *
 * @param value GMREMIX_MENU_MUSIC_RANDOM or GMREMIX_MENU_MUSIC_RANDOM_CLASSICS.
 * @param default_bgm BGM id kept when the random value is the 64 track.
 * @return The BGM id to play.
 */
static u32 gmRemixMenuMusicRandomTrack(s32 value, u32 default_bgm)
{
    return gmRemixMenuMusicTrackOf(gmRemixMenuMusicRandomValue(value), default_bgm);
}

/**
 * Picks the RANDOM ALL track: a track of the Random Music list ignoring the Random Music toggle (BGM.asm random_music_
 * with menu_randomizing_all set), or one of the hidden GameBoy songs when the list is empty (BGM.asm _random_song).
 *
 * @return The BGM id to play.
 */
static u32 gmRemixMenuMusicRandomAllTrack(void)
{
    u32 bgm = grRemixRandomMusicPickForced(GMREMIX_MENU_MUSIC_NO_RANDOM_TRACK);

    if (bgm != GMREMIX_MENU_MUSIC_NO_RANDOM_TRACK)
    {
        return bgm;
    }
    return GMREMIX_BGM_GAMEBOY_FIRST + syUtilsRandIntRange(GMREMIX_MENU_MUSIC_GAMEBOY_SONGS);
}

/**
 * Checks whether a Menu Music value is one of the RANDOM values.
 *
 * @param value Menu Music value.
 * @return TRUE for RANDOM, RANDOM CLASSICS and RANDOM ALL.
 */
static sb32 gmRemixMenuMusicIsRandom(s32 value)
{
    return (value == GMREMIX_MENU_MUSIC_RANDOM) || (value == GMREMIX_MENU_MUSIC_RANDOM_CLASSICS) ||
           (value == GMREMIX_MENU_MUSIC_RANDOM_ALL);
}

/**
 * Checks whether a BGM id is one of the menu tracks of sMenuMusicTrackBGM (values 2 to 18) or the trap variant of
 * FILESELECT_SM64.
 *
 * @param bgm BGM id to check, -1 when no track is playing.
 * @return TRUE when bgm is a listed menu track.
 */
static sb32 gmRemixMenuMusicIsListedTrack(s32 bgm)
{
    u32 i;

    if (bgm == GMREMIX_BGM_MAIN_TRAP_SM64)
    {
        return TRUE;
    }
    for (i = 0; i < sizeof(sMenuMusicTrackBGM) / sizeof(sMenuMusicTrackBGM[0]); i++)
    {
        if (bgm == (s32)sMenuMusicTrackBGM[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Checks whether a BGM id is one of the two 64 tracks (menu and character select), which BGM.asm always restarts.
 *
 * @param bgm BGM id to check.
 * @return TRUE for nSYAudioBGMModeSelect and nSYAudioBGMBattleSelect.
 */
static sb32 gmRemixMenuMusicIs64Track(s32 bgm)
{
    return (bgm == nSYAudioBGMModeSelect) || (bgm == nSYAudioBGMBattleSelect);
}

/**
 * Decides whether the track already playing keeps playing instead of being restarted (BGM.asm _check_current_track and
 * _finish). Nothing is kept when no track plays or when a 64 track plays, since BGM.asm restarts those.
 *
 * @param value Menu Music value.
 * @param current_bgm BGM id playing now, -1 when none.
 * @param default_bgm BGM id of the vanilla screen.
 * @return TRUE when the current track must keep playing.
 */
static sb32 gmRemixMenuMusicKeepsCurrent(s32 value, s32 current_bgm, u32 default_bgm)
{
    if ((current_bgm < 0) || gmRemixMenuMusicIs64Track(current_bgm))
    {
        return FALSE;
    }
    if (gmRemixMenuMusicIsRandom(value))
    {
        return TRUE;
    }
    if (value == GMREMIX_MENU_MUSIC_DEFAULT)
    {
        return gmRemixMenuMusicIsListedTrack(current_bgm);
    }
    return current_bgm == (s32)gmRemixMenuMusicTrackOf(value, default_bgm);
}

/**
 * Chooses the menu BGM id for the current Menu Music value.
 *
 * @param default_bgm BGM id of the vanilla screen.
 * @param from_title_screen TRUE on the title screen call.
 * @return The BGM id to play. The OFF value never reaches this function (gmRemixMenuMusicPlay and gmRemixMenuMusicStop
 *         handle it).
 */
static u32 gmRemixMenuMusicChoose(u32 default_bgm, sb32 from_title_screen)
{
    s32 value = gmRemixToggle(menu_music);

    if (value == GMREMIX_MENU_MUSIC_DEFAULT)
    {
        return from_title_screen ? gmRemixMenuMusicTitleRoll(default_bgm) : default_bgm;
    }
    if ((value == GMREMIX_MENU_MUSIC_RANDOM) || (value == GMREMIX_MENU_MUSIC_RANDOM_CLASSICS))
    {
        return gmRemixMenuMusicRandomTrack(value, default_bgm);
    }
    if (value == GMREMIX_MENU_MUSIC_RANDOM_ALL)
    {
        return gmRemixMenuMusicRandomAllTrack();
    }
    return gmRemixMenuMusicTrackOf(value, default_bgm);
}

/**
 * Starts the Menu Music track of a menu screen, or keeps the track already playing when the Remix rules say so.
 *
 * @param default_bgm BGM id of the vanilla screen.
 * @param from_title_screen TRUE only on the title screen call.
 */
void gmRemixMenuMusicPlay(u32 default_bgm, sb32 from_title_screen)
{
    s32 value = gmRemixToggle(menu_music);
    s32 current_bgm = syAudioGetBGMPlaying(0);

    if (value == GMREMIX_MENU_MUSIC_OFF)
    {
        syAudioStopBGMAll();
        return;
    }
    if (gmRemixMenuMusicKeepsCurrent(value, current_bgm, default_bgm))
    {
        return;
    }
    syAudioPlayBGM(0, gmRemixMenuMusicChoose(default_bgm, from_title_screen));
}

/**
 * Stops the BGM when a menu screen is left, unless a listed menu track or a RANDOM value keeps it playing (BGM.asm
 * alt_menu_music_._prevent_stop, which replaces BGM.stop_ on the CSS back paths).
 */
void gmRemixMenuMusicStop(void)
{
    s32 value = gmRemixToggle(menu_music);
    s32 current_bgm = syAudioGetBGMPlaying(0);

    if (gmRemixMenuMusicIsRandom(value) || gmRemixMenuMusicIsListedTrack(current_bgm))
    {
        return;
    }
    syAudioStopBGMAll();
}

/**
 * Resolves the Menu Music value an A press stands for: DEFAULT rolls the Smash menu tracks with the A press odds
 * (GMREMIX_AUDITION_DEFAULT_ROLL_RANGE), RANDOM and RANDOM CLASSICS pick a track value, any other value is itself.
 *
 * @param value Menu Music value, not OFF.
 * @return The value of the track to play (RANDOM ALL stays RANDOM ALL).
 */
static s32 gmRemixMenuMusicAuditionValue(s32 value)
{
    if (value == GMREMIX_MENU_MUSIC_DEFAULT)
    {
        return gmRemixToggleActionGetAuditionDefaultValue(syUtilsRandIntRange(GMREMIX_AUDITION_DEFAULT_ROLL_RANGE));
    }
    if ((value == GMREMIX_MENU_MUSIC_RANDOM) || (value == GMREMIX_MENU_MUSIC_RANDOM_CLASSICS))
    {
        return gmRemixMenuMusicRandomValue(value);
    }
    return value;
}

/**
 * Counts the press for the itsatrap easter egg (Toggles.asm play_menu_music_ _check_bgm and _sm64_trap_check): a press that
 * lands on FILESELECT_SM64 counts, a press on any other track clears the counter.
 *
 * @param value Menu Music value the press resolved to.
 */
static void gmRemixMenuMusicCountTrapPress(s32 value)
{
    if (value != GMREMIX_MENU_MUSIC_FILESELECT_SM64)
    {
        sGMRemixMenuMusicTrapCount = 0;
        return;
    }
    sGMRemixMenuMusicTrapCount = gmRemixMenuMusicRulesCountTrapPress(sGMRemixMenuMusicTrapCount);
}

/**
 * A press on Menu Music (Toggles.asm play_menu_music_): plays the chosen track now, always restarting it, or stops the
 * music for OFF. Unlike gmRemixMenuMusicPlay it never keeps the track already playing. RANDOM ALL picks from the Random
 * Music list. FILESELECT_SM64 presses are counted (itsatrap): the fourth in a row plays MAIN_TRAP_SM64 and the screens keep
 * it until a press lands on another track.
 */
void gmRemixMenuMusicAudition(void)
{
    s32 value = gmRemixToggle(menu_music);

    if (value == GMREMIX_MENU_MUSIC_OFF)
    {
        syAudioStopBGMAll();
        return;
    }
    value = gmRemixMenuMusicAuditionValue(value);
    gmRemixMenuMusicCountTrapPress(value);

    if (value == GMREMIX_MENU_MUSIC_RANDOM_ALL)
    {
        syAudioPlayBGM(0, gmRemixMenuMusicRandomAllTrack());
        return;
    }
    syAudioPlayBGM(0, gmRemixMenuMusicTrackOf(value, nSYAudioBGMModeSelect));
}

#endif
