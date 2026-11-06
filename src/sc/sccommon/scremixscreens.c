/**
 * scremixscreens.c - Other Screens of the Remix settings menu, see scremixscreens.h.
 *
 * Toggles.asm view_screen_ changes the screen id with Menu.change_screen_: Debug 0x03, View Intro 0x1C, View How to Play 0x3C. The
 * port scene ids are the SCKind values of scdef.h. View Credits (0x38) also stores the port of the player that pressed A or START as
 * the 1P port; the port menu has no such player, so port 1 is used. View Gallery makes the Gallery active with the Options screen
 * as the screen B returns to (view_gallery, previous_screen 0x39).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scene.h>
#include <mn/mncommon/mnremixgallery.h>
#include <sc/sccommon/scremixscreens.h>

extern void portSCManagerRequestScene(s32 scene);

#define SC_REMIX_SCREENS_CREDITS_PLAYER 0

/**
 * Scene that shows an Other Screen.
 *
 * @param screen SC_REMIX_SCREEN_* value.
 * @return The SCKind value, or -1 when screen is unknown.
 */
static s32 scRemixScreensGetScene(s32 screen)
{
    switch (screen)
    {
    case SC_REMIX_SCREEN_DEBUG:
        return nSCKindDebugCube;

    case SC_REMIX_SCREEN_INTRO:
        return nSCKindOpeningRoom;

    case SC_REMIX_SCREEN_HOW_TO_PLAY:
        return nSCKindExplain;

    case SC_REMIX_SCREEN_CREDITS:
        return nSCKindStaffroll;

    case SC_REMIX_SCREEN_GALLERY:
        return nSCKindCongra;

    default:
        return -1;
    }
}

void scRemixScreensOpen(s32 screen, s32 gallery_page)
{
    s32 scene = scRemixScreensGetScene(screen);

    if (scene < 0)
    {
        return;
    }
    if (screen == SC_REMIX_SCREEN_CREDITS)
    {
        gSCManagerSceneData.player = SC_REMIX_SCREENS_CREDITS_PLAYER;
    }
    if (screen == SC_REMIX_SCREEN_GALLERY)
    {
        mnRemixGalleryActivateAtPage(nSCKindOption, gallery_page);
    }
    portSCManagerRequestScene(scene);
}

#endif
