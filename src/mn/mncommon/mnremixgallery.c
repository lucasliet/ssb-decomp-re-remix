#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mn/mncommon/mncongra.h>
#include <sc/scene.h>
#include <sys/controller.h>
#include <sys/taskman.h>
#include <mn/mncommon/mnremixgallery.h>
#include <mn/mncommon/mnremixgalleryrules.h>
#include "remix/remix_roster_data.h"

#define MN_REMIX_GALLERY_STICK_THRESHOLD 40

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
    sMNRemixGalleryIndex = mnRemixGalleryRulesStepIndex(sMNRemixGalleryIndex, step);
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
    return REMIX_ID_TO_PORT_FKIND(mnRemixGalleryRulesGetCharacterId(sMNRemixGalleryIndex));
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

#endif
