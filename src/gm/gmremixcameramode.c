/**
 * gmremixcameramode.c - Camera Mode setting (Camera.asm entry_camera_mode) for the battle camera: the BONUS, FIXED and
 * SCENE routines and frozen views, and the checks of the pause and Cinematic Entry that depend on the mode.
 * See ft/ftremix/ftremixhooks.h for the vanilla call sites.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmcamera.h"
#include "gm/gmremixtoggle.h"
#include "gr/grdef.h"
#include "gr/grremix/grremixids.h"
#include "mp/map.h"
#include "sc/scene.h"
#include "sc/scmanager.h"
#include "sys/objtypes.h"

#define GM_REMIX_CAMERA_FOV_DEFAULT 38.0F

enum
{
    nGMRemixCameraModeNormal,
    nGMRemixCameraModeBonus,
    nGMRemixCameraModeFixed,
    nGMRemixCameraModeScene
};

typedef struct GMRemixCameraParams
{
    Vec3f eye;
    Vec3f at;
    f32 zoom;
} GMRemixCameraParams;

typedef struct GMRemixCameraStageParams
{
    s32 stage;
    const GMRemixCameraParams *params;
} GMRemixCameraStageParams;

extern void (*dGMCameraFuncList[])(GObj *camera_gobj);

/* Camera.asm frozen_camera_ and fixed_camera_always_ (frozen views, the stages that always use them) */
static const GMRemixCameraParams sGMRemixCameraFrozenWorld1 = { { 0.0F, -100.0F, 6600.0F }, { 0.0F, -100.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFrozenFlatZone = { { 0.0F, -100.0F, 6600.0F }, { 0.0F, -100.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFrozenFlatZone2 = { { 0.0F, 0.0F, 8000.0F }, { 0.0F, 0.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraGBLand = { { 0.0F, 1250.0F, 7000.0F }, { 0.0F, 1250.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static GMRemixCameraParams sGMRemixCameraHRC = { { -21312.0F, 1536.0F, 6112.0F }, { -21312.0F, -512.0F, -3584.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraPokeFloats = { { 0.0F, 0.0F, 7000.0F }, { 0.0F, 0.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };

/* Camera.asm _set_fixed_camera_values, the fixed view of each stage (the default is fixed_default) */
static const GMRemixCameraParams sGMRemixCameraFixedDefault = { { 0.0F, 1850.0F, 10000.0F }, { 0.0F, 942.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedIncreaseY = { { 0.0F, 2560.0F, 10000.0F }, { 0.0F, 1328.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedIncreaseYMore = { { 0.0F, 2560.0F, 10000.0F }, { 0.0F, 1968.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedIncreaseYAndZ = { { 240.0F, 1200.0F, 12032.0F }, { 240.0F, 1608.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedSectorZDLO = { { 0.0F, 3328.0F, 10000.0F }, { 0.0F, 2656.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedShowdown = { { 0.0F, 2560.0F, 15000.0F }, { 0.0F, 1968.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedSectorZ = { { 960.0F, 2168.0F, 11008.0F }, { 960.0F, 692.0F, 0.0F }, 60.0F };
static const GMRemixCameraParams sGMRemixCameraFixedSectorZSR = { { 6656.0F, 1250.0F, 7000.0F }, { 6656.0F, 1250.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedYoshisIsland = { { 960.0F, 1250.0F, 11008.0F }, { 1664.0F, 1250.0F, 0.0F }, 48.0F };
static const GMRemixCameraParams sGMRemixCameraFixedHyruleCastle = { { 0.0F, 2560.0F, 11264.0F }, { 0.0F, 2048.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedHyruleTemple = { { 0.0F, 960.0F, 16144.0F }, { 0.0F, 128.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedFrostyVillage = { { -416.0F, 1328.0F, 11024.0F }, { -480.0F, 1968.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedN64 = { { -512.0F, 4096.0F, 10000.0F }, { -512.0F, 1024.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedSaffronCity = { { 0.0F, 1200.0F, 11024.0F }, { -96.0F, 492.0F, -96.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedMushroomKingdomSR = { { 0.0F, 1850.0F, 11024.0F }, { 0.0F, 942.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedSmashketball = { { 0.0F, 960.0F, 11392.0F }, { 0.0F, 235.0F, -11392.0F }, 32.0F };
static const GMRemixCameraParams sGMRemixCameraFixedPeach2 = { { 240.0F, 1000.0F, 12032.0F }, { 240.0F, 1408.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedGreatBay = { { 2304.0F, 1850.0F, 10000.0F }, { 2048.0F, 942.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedRTTF = { { 0.0F, 3632.0F, 21760.0F }, { 0.0F, 0.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedRemixRTTF = { { 0.0F, 4400.0F, 23120.0F }, { 0.0F, 400.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };
static const GMRemixCameraParams sGMRemixCameraFixedRest = { { 0.0F, 1850.0F, 5632.0F }, { 0.0F, 942.0F, 0.0F }, GM_REMIX_CAMERA_FOV_DEFAULT };

/* Stages of the fixed table (Camera.asm _set_fixed_camera_values, after the frozen and bonus checks); the first match wins
 * and the duplicated SECTOR_Z_DL and SECTOR_Z_O checks of the asm are unreachable, so they are left out */
static const GMRemixCameraStageParams sGMRemixCameraFixedStages[] =
{
    { nGRRemixStage_RACE_TO_THE_FINISH, &sGMRemixCameraFixedRTTF },
    { nGRRemixStage_REMIX_RTTF, &sGMRemixCameraFixedRemixRTTF },
    { nGRRemixStage_REST, &sGMRemixCameraFixedRest },
    { nGRRemixStage_PLANET_ZEBES, &sGMRemixCameraFixedIncreaseY },
    { nGRRemixStage_COOLCOOL, &sGMRemixCameraFixedIncreaseY },
    { nGRRemixStage_COOLCOOL_REMIX, &sGMRemixCameraFixedIncreaseY },
    { nGRRemixStage_PEACHS_CASTLE, &sGMRemixCameraFixedIncreaseY },
    { nGRRemixStage_DRAGONKING, &sGMRemixCameraFixedIncreaseY },
    { nGRRemixStage_NPC, &sGMRemixCameraFixedIncreaseY },
    { nGRRemixStage_SECTOR_Z_DL, &sGMRemixCameraFixedSectorZDLO },
    { nGRRemixStage_SECTOR_Z_O, &sGMRemixCameraFixedSectorZDLO },
    { nGRRemixStage_GANONS_TOWER, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_VENOM, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_HCASTLE_REMIX, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_ONETT, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_KITCHEN, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_TOH, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_MUTE, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_BLUE, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_WINDY, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_BOWSERS_KEEP, &sGMRemixCameraFixedIncreaseYMore },
    { nGRRemixStage_OSOHE, &sGMRemixCameraFixedIncreaseYAndZ },
    { nGRRemixStage_RITH_ESSA, &sGMRemixCameraFixedIncreaseYAndZ },
    { nGRRemixStage_SHOWDOWN, &sGMRemixCameraFixedShowdown },
    { nGRRemixStage_FROSTY, &sGMRemixCameraFixedFrostyVillage },
    { nGRRemixStage_HTEMPLE, &sGMRemixCameraFixedHyruleTemple },
    { nGRRemixStage_HYRULE_CASTLE, &sGMRemixCameraFixedHyruleCastle },
    { nGRRemixStage_YOSHIS_ISLAND, &sGMRemixCameraFixedYoshisIsland },
    { nGRRemixStage_SECTOR_Z_REMIX, &sGMRemixCameraFixedSectorZSR },
    { nGRRemixStage_SECTOR_Z, &sGMRemixCameraFixedSectorZ },
    { nGRRemixStage_N64, &sGMRemixCameraFixedN64 },
    { nGRRemixStage_SAFFRON_CITY, &sGMRemixCameraFixedSaffronCity },
    { nGRRemixStage_MK_REMIX, &sGMRemixCameraFixedMushroomKingdomSR },
    { nGRRemixStage_SMASHKETBALL, &sGMRemixCameraFixedSmashketball },
    { nGRRemixStage_PEACH2, &sGMRemixCameraFixedPeach2 },
    { nGRRemixStage_GREAT_BAY, &sGMRemixCameraFixedGreatBay },
};

/** Reads the Camera Mode toggle: NORMAL, BONUS, FIXED or SCENE. */
static s32 gmRemixCameraModeToggle(void)
{
    return gmRemixToggle(camera_mode);
}

/** TRUE when the hazard mode bit 0 is clear, which turns on the flat zone frozen views (Camera.asm frozen_camera_). */
static sb32 gmRemixCameraFlatZonesFrozen(void)
{
    return (gmRemixToggle(hazard_mode) & 0x0001) == 0;
}

/**
 * Frozen view of a stage for the camera updates (Camera.asm frozen_camera_). NULL when the camera moves normally.
 */
static const GMRemixCameraParams *gmRemixCameraFreezeParams(s32 stage)
{
    switch (stage)
    {
    case nGRRemixStage_WORLD1:
        return &sGMRemixCameraFrozenWorld1;

    case nGRRemixStage_HRC:
        return &sGMRemixCameraHRC;

    case nGRRemixStage_POKEFLOATS:
        return &sGMRemixCameraPokeFloats;

    default:
        break;
    }
    if (gmRemixCameraFlatZonesFrozen() == FALSE)
    {
        return NULL;
    }
    switch (stage)
    {
    case nGRRemixStage_GB_LAND:
        return &sGMRemixCameraGBLand;

    case nGRRemixStage_FLAT_ZONE_2:
        return &sGMRemixCameraFrozenFlatZone2;

    case nGRRemixStage_FLAT_ZONE:
        return &sGMRemixCameraFrozenFlatZone;

    default:
        return NULL;
    }
}

/** Frozen view of the Fixed mode stages (Camera.asm _set_fixed_camera_values). The hazard mode does not apply here. */
static const GMRemixCameraParams *gmRemixCameraFixedFrozenParams(s32 stage)
{
    switch (stage)
    {
    case nGRRemixStage_WORLD1:
        return &sGMRemixCameraFrozenWorld1;

    case nGRRemixStage_GB_LAND:
        return &sGMRemixCameraGBLand;

    case nGRRemixStage_FLAT_ZONE_2:
        return &sGMRemixCameraFrozenFlatZone2;

    case nGRRemixStage_FLAT_ZONE:
        return &sGMRemixCameraFrozenFlatZone;

    case nGRRemixStage_POKEFLOATS:
        return &sGMRemixCameraPokeFloats;

    default:
        return NULL;
    }
}

/** Fixed view of a stage that has no frozen view (Camera.asm _set_fixed_camera_values, default case). */
static const GMRemixCameraParams *gmRemixCameraFixedStageParams(s32 stage)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sGMRemixCameraFixedStages); i++)
    {
        if (sGMRemixCameraFixedStages[i].stage == stage)
        {
            return sGMRemixCameraFixedStages[i].params;
        }
    }
    return &sGMRemixCameraFixedDefault;
}

/** Copies the eye and focal point of a view to the camera object. */
static void gmRemixCameraWriteView(CObj *cobj, const GMRemixCameraParams *params)
{
    cobj->vec.eye = params->eye;
    cobj->vec.at = params->at;
}

/** Writes a fixed view, including its fov (the zoom field of the Camera.asm table). */
static void gmRemixCameraWriteFixedView(CObj *cobj, const GMRemixCameraParams *params)
{
    gmRemixCameraWriteView(cobj, params);
    cobj->projection.persp.fovy = params->zoom;
}

/** Bonus stage pause view, read from the ground data (Camera.asm _set_fixed_bonus_cam_values). */
static void gmRemixCameraWriteBonusPauseView(CObj *cobj)
{
    cobj->vec.at.x = gMPCollisionGroundData->zoom_start.x;
    cobj->vec.at.y = gMPCollisionGroundData->zoom_start.y;
    cobj->vec.at.z = gMPCollisionGroundData->zoom_start.z;
    cobj->vec.eye.x = gMPCollisionGroundData->zoom_end.x;
    cobj->vec.eye.y = gMPCollisionGroundData->zoom_end.y;
    cobj->vec.eye.z = gMPCollisionGroundData->zoom_end.z;
}

/** TRUE on the Bonus stage screen, where Fixed mode uses the pause view (Camera.asm current_screen 0x35). */
static sb32 gmRemixCameraIsBonusScreen(void)
{
    return gSCManagerSceneData.scene_curr == nSCKind1PBonusStage;
}

/**
 * Fixed mode view of the battle camera (Camera.asm _fixed_cam, after the HRC check): the frozen stages first, then the
 * bonus stage pause view, then the stage table.
 */
static void gmRemixCameraSetupFixedView(CObj *cobj, s32 stage)
{
    const GMRemixCameraParams *frozen_params = gmRemixCameraFixedFrozenParams(stage);

    if (frozen_params != NULL)
    {
        gmRemixCameraWriteFixedView(cobj, frozen_params);
        return;
    }
    if (gmRemixCameraIsBonusScreen())
    {
        gmRemixCameraWriteBonusPauseView(cobj);
        return;
    }
    gmRemixCameraWriteFixedView(cobj, gmRemixCameraFixedStageParams(stage));
}

/**
 * Bonus mode routines (Camera.asm _bonus_cam): the default and Planet Zebes routines become the Mushroom Kingdom one.
 */
static void gmRemixCameraUseInishieRoutines(void)
{
    dGMCameraFuncList[nGMCameraStatusDefault] = dGMCameraFuncList[nGMCameraStatusInishie];
    dGMCameraFuncList[nGMCameraStatusZebes] = dGMCameraFuncList[nGMCameraStatusInishie];
}

/**
 * Scene mode routines (Camera.asm _scene_cam): the default, Mushroom Kingdom and Planet Zebes routines become the anim
 * routine.
 */
static void gmRemixCameraUseAnimRoutines(void)
{
    dGMCameraFuncList[nGMCameraStatusDefault] = dGMCameraFuncList[nGMCameraStatusAnim];
    dGMCameraFuncList[nGMCameraStatusInishie] = dGMCameraFuncList[nGMCameraStatusAnim];
    dGMCameraFuncList[nGMCameraStatusZebes] = dGMCameraFuncList[nGMCameraStatusAnim];
}

/** TRUE when the Camera Mode toggle is Fixed. */
sb32 gmRemixCameraIsFixed(void)
{
    return gmRemixCameraModeToggle() == nGMRemixCameraModeFixed;
}

/** TRUE when the Camera Mode toggle is Scene. */
sb32 gmRemixCameraIsScene(void)
{
    return gmRemixCameraModeToggle() == nGMRemixCameraModeScene;
}

/**
 * Sets the battle camera view and routines of a default-branch camera (Camera.asm set_initial_coordinates_). Runs before
 * the Remix status of the stage is chosen, because the routine table it edits is read there.
 */
void gmRemixCameraSetupDefault(CObj *cobj)
{
    s32 mode = gmRemixCameraModeToggle();
    s32 stage = gSCManagerBattleState->gkind;

    if (mode == nGMRemixCameraModeNormal)
    {
        return;
    }
    if (mode == nGMRemixCameraModeBonus)
    {
        gmRemixCameraUseInishieRoutines();
        return;
    }
    if (stage == nGRRemixStage_HRC)
    {
        return;
    }
    if (mode == nGMRemixCameraModeScene)
    {
        gmRemixCameraUseAnimRoutines();
        return;
    }
    gmRemixCameraSetupFixedView(cobj, stage);
}

/**
 * Makes the Fixed mode battle camera run the anim routine (Camera.asm get_custom_camera_, which replaces the routine read
 * of gmCameraRunFuncCamera). Status changes are blocked in Fixed mode (gmRemixCameraSetStatusIsFrozen), so the routine
 * stays the anim one. Zebes, Mushroom Kingdom and HRC never get it (their setup does not run in Camera.asm).
 */
void gmRemixCameraApplyFixedRun(s32 gkind)
{
    if (gmRemixCameraIsFixed() == FALSE)
    {
        return;
    }
    if ((gkind == nGRKindZebes) || (gkind == nGRKindInishie) || (gkind == nGRRemixStage_HRC))
    {
        return;
    }
    gGMCameraStruct.func_camera = gmCameraAnimFuncCamera;
}

/**
 * TRUE when a status change must be ignored (Camera.asm fixed_camera_always_): Fixed mode, or a stage with a frozen view.
 */
sb32 gmRemixCameraSetStatusIsFrozen(void)
{
    if (gmRemixCameraIsFixed())
    {
        return TRUE;
    }
    return gmRemixCameraFreezeParams(gSCManagerBattleState->gkind) != NULL;
}

/** X of the Home-Run Contest camera (Camera.hrc eye X), which HRC.asm main_ moves to follow the Sandbag. */
f32 gmRemixCameraGetHrcX(void)
{
    return sGMRemixCameraHRC.eye.x;
}

/** Moves the Home-Run Contest camera along X, eye and focal point together (HRC.asm main_ writes Camera.hrc + 0x0 and + 0xC). */
void gmRemixCameraSetHrcX(f32 x)
{
    sGMRemixCameraHRC.eye.x = x;
    sGMRemixCameraHRC.at.x = x;
}

/**
 * Writes the frozen view of the stage over the camera position update (Camera.asm frozen_camera_). Returns TRUE when the
 * update was replaced, and the caller skips its vector add.
 */
sb32 gmRemixCameraFreezeUpdate(CObj *cobj)
{
    const GMRemixCameraParams *params = gmRemixCameraFreezeParams(gSCManagerBattleState->gkind);

    if (params == NULL)
    {
        return FALSE;
    }
    gmRemixCameraWriteView(cobj, params);
    return TRUE;
}

/**
 * Cinematic Entry focus of the countdown (Camera.asm cinematic_entry_): Fixed and Scene modes never play it, Always
 * returns the cinematic focus (2), Never returns 0, and Default keeps the random focus.
 */
s32 gmRemixCameraCinematicFocus(s32 random_focus)
{
    s32 mode = gmRemixCameraModeToggle();
    s32 setting = gmRemixToggle(cinematic_entry);

    if ((mode == nGMRemixCameraModeFixed) || (mode == nGMRemixCameraModeScene))
    {
        return 0;
    }
    if (setting == 0)
    {
        return random_focus;
    }
    if (setting == 1)
    {
        return 2;
    }
    return 0;
}

/** TRUE when the Bonus pause zoom runs (Camera.asm prevent_fixed_camera_pause_bonus_: not in Fixed mode). */
sb32 gmRemixCameraPauseBonusZoomAllowed(void)
{
    return gmRemixCameraIsFixed() == FALSE;
}

/**
 * TRUE when the analog pause control stays on (Camera.asm pause_disable_control_flag_). Fixed mode, the flat zones (when
 * hazard bit 0 is clear), WORLD1 and HRC turn it off.
 */
sb32 gmRemixCameraPauseControlEnabled(void)
{
    s32 stage = gSCManagerBattleState->gkind;

    if (gmRemixCameraIsFixed())
    {
        return FALSE;
    }
    if (gmRemixCameraFlatZonesFrozen())
    {
        if ((stage == nGRRemixStage_FLAT_ZONE_2) || (stage == nGRRemixStage_FLAT_ZONE) || (stage == nGRRemixStage_GB_LAND))
        {
            return FALSE;
        }
    }
    return (stage != nGRRemixStage_WORLD1) && (stage != nGRRemixStage_HRC);
}

#endif
