/**
 * gmremixmusictitle.c - Music Title at Match Start toggle of the Remix settings (BGM.asm show_music_title_).
 *
 * The asm draws a translucent rectangle with the track line ("note title note", scale 1.0) and the game line (scale 0.75)
 * in Kabel when the GO lights are made, and destroys them when the GO! text is made. The rectangle and both lines go through
 * the port overlay (remix_hud_text), put every frame by a process of the scene while the banner is shown.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmusictitle.h"
#include "gm/gmremixtoggle.h"
#include <sys/audio.h>
#include <sys/objman.h>
#include "remix/remix_hud_text.h"
#include "remix/remix_music_title.h"

/* BGM.asm: rectangle (1, 156, 320 x 28, 0x34343494), track line at (160, 158) and game line at (160, 172), centred. */
#define GMREMIX_MUSIC_TITLE_RECT_SLOT 2
#define GMREMIX_MUSIC_TITLE_TRACK_SLOT 71
#define GMREMIX_MUSIC_TITLE_GAME_SLOT 72
#define GMREMIX_MUSIC_TITLE_RECT_X 1.0F
#define GMREMIX_MUSIC_TITLE_RECT_Y 156.0F
#define GMREMIX_MUSIC_TITLE_RECT_WIDTH 320.0F
#define GMREMIX_MUSIC_TITLE_RECT_HEIGHT 28.0F
#define GMREMIX_MUSIC_TITLE_RECT_COLOR 0x34343494
#define GMREMIX_MUSIC_TITLE_CENTER_X 160.0F
#define GMREMIX_MUSIC_TITLE_TRACK_Y 158.0F
#define GMREMIX_MUSIC_TITLE_GAME_Y 172.0F
#define GMREMIX_MUSIC_TITLE_GAME_SCALE 0.75F
#define GMREMIX_MUSIC_TITLE_COLOR 0xFFFFFFFF
#define GMREMIX_MUSIC_TITLE_PROC_PRIORITY 3

static sb32 sGMRemixMusicTitleIsShown;
static const char *sGMRemixMusicTitleGame;
static const char *sGMRemixMusicTitleTrack;

static void gmRemixMusicTitleProcUpdate(GObj *gobj)
{
    if (sGMRemixMusicTitleIsShown == FALSE)
    {
        return;
    }
    remix_hud_rect_put(GMREMIX_MUSIC_TITLE_RECT_SLOT, GMREMIX_MUSIC_TITLE_RECT_X, GMREMIX_MUSIC_TITLE_RECT_Y,
                       GMREMIX_MUSIC_TITLE_RECT_WIDTH, GMREMIX_MUSIC_TITLE_RECT_HEIGHT, GMREMIX_MUSIC_TITLE_RECT_COLOR);
    /* The glyph 0x80 of the Kabel font is the musical note the asm puts on both sides of the track name. */
    remix_hud_text_putf(GMREMIX_MUSIC_TITLE_TRACK_SLOT, GMREMIX_MUSIC_TITLE_CENTER_X, GMREMIX_MUSIC_TITLE_TRACK_Y,
                        GMREMIX_MUSIC_TITLE_COLOR, 1.0F, REMIX_HUD_ALIGN_CENTER, "\x80 %s \x80", sGMRemixMusicTitleTrack);
    remix_hud_text_put(GMREMIX_MUSIC_TITLE_GAME_SLOT, sGMRemixMusicTitleGame, GMREMIX_MUSIC_TITLE_CENTER_X,
                       GMREMIX_MUSIC_TITLE_GAME_Y, GMREMIX_MUSIC_TITLE_COLOR, GMREMIX_MUSIC_TITLE_GAME_SCALE,
                       REMIX_HUD_ALIGN_CENTER);
}

void gmRemixMusicTitleShowAtCountdown(void)
{
    GObj *gobj;

    sGMRemixMusicTitleIsShown = FALSE;

    if (!gmRemixToggle(show_music_title) || !gmRemixToggle(play_music))
    {
        return;
    }
    if (remix_music_title_get(syAudioGetBGMPlaying(0), &sGMRemixMusicTitleTrack, &sGMRemixMusicTitleGame) == 0)
    {
        return;
    }
    sGMRemixMusicTitleIsShown = TRUE;

    gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, gmRemixMusicTitleProcUpdate, nGCProcessKindFunc, GMREMIX_MUSIC_TITLE_PROC_PRIORITY);
}

void gmRemixMusicTitleHideAtGo(void)
{
    sGMRemixMusicTitleIsShown = FALSE;
}

#endif
