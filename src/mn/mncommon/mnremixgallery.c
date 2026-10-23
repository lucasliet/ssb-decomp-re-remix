#include <ft/fighter.h>
#include <mn/mncommon/mncongra.h>
#include <sc/scene.h>
#include <sys/controller.h>
#include <sys/taskman.h>
#include <mn/mncommon/mnremixgallery.h>

#define MN_REMIX_GALLERY_STICK_THRESHOLD 40

/* Gallery.asm id_table order (Character.id) mapped to the fighter kinds that
 * index the vanilla Congratulations picture table. The remaining 28 Remix
 * gallery entries need the synth fighter kinds: REMIX-TODO(gallery-remix-fighters). */
static const s32 sMNRemixGalleryFighterKinds[] =
{
    nFTKindMario,
    nFTKindDonkey,
    nFTKindLink,
    nFTKindSamus,
    nFTKindYoshi,
    nFTKindKirby,
    nFTKindFox,
    nFTKindPikachu,
    nFTKindLuigi,
    nFTKindCaptain,
    nFTKindNess,
    nFTKindPurin,
};

static sb32 sMNRemixGalleryIsActive;
static s32 sMNRemixGalleryIndex;
static s32 sMNRemixGalleryPreviousScene;
static s32 sMNRemixGalleryStickDirection;

/* Keeps the Congratulations scene id on every page change so the picture reloads. */
static void mnRemixGalleryReloadScene(void)
{
    gSCManagerSceneData.scene_prev = nSCKindCongra;
    gSCManagerSceneData.scene_curr = nSCKindCongra;
    syTaskmanSetLoadScene();
}

static void mnRemixGalleryLeave(void)
{
    sMNRemixGalleryIsActive = FALSE;
    gSCManagerSceneData.scene_prev = nSCKindCongra;
    gSCManagerSceneData.scene_curr = sMNRemixGalleryPreviousScene;
    syTaskmanSetLoadScene();
}

static void mnRemixGalleryStep(s32 step)
{
    s32 count = ARRAY_COUNT(sMNRemixGalleryFighterKinds);

    sMNRemixGalleryIndex = (sMNRemixGalleryIndex + step + count) % count;
    mnRemixGalleryReloadScene();
}

/* Stick direction of player 1: 1 = right, -1 = left, 0 = inside the dead zone. */
static s32 mnRemixGalleryReadStickDirection(void)
{
    s32 stick_x = gSYControllerDevices[0].stick_range.x;

    if (stick_x >= MN_REMIX_GALLERY_STICK_THRESHOLD)
    {
        return 1;
    }
    if (stick_x <= -MN_REMIX_GALLERY_STICK_THRESHOLD)
    {
        return -1;
    }
    return 0;
}

/* Returns a page step for this frame. The stick steps once per push, not once per frame. */
static s32 mnRemixGalleryReadStep(void)
{
    s32 stick_direction = mnRemixGalleryReadStickDirection();
    s32 step = 0;

    if (mnCongraGetPlayerTapButtons(R_TRIG | R_JPAD) != FALSE)
    {
        step = 1;
    }
    else if (mnCongraGetPlayerTapButtons(L_TRIG | L_JPAD) != FALSE)
    {
        step = -1;
    }
    else if ((stick_direction != 0) && (stick_direction != sMNRemixGalleryStickDirection))
    {
        step = stick_direction;
    }
    sMNRemixGalleryStickDirection = stick_direction;
    return step;
}

void mnRemixGalleryActivate(s32 previous_scene)
{
    sMNRemixGalleryIsActive = TRUE;
    sMNRemixGalleryIndex = 0;
    sMNRemixGalleryPreviousScene = previous_scene;
    sMNRemixGalleryStickDirection = 0;
}

sb32 mnRemixGalleryIsActive(void)
{
    return sMNRemixGalleryIsActive;
}

s32 mnRemixGalleryGetFighterKind(void)
{
    return sMNRemixGalleryFighterKinds[sMNRemixGalleryIndex];
}

void mnRemixGalleryUpdateInput(void)
{
    s32 step;

    if (mnCongraGetPlayerTapButtons(B_BUTTON) != FALSE)
    {
        mnRemixGalleryLeave();
        return;
    }
    step = mnRemixGalleryReadStep();
    if (step != 0)
    {
        mnRemixGalleryStep(step);
    }
}
