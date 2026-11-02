/**
 * gmremixspecialzoom.c - Smash Remix SpecialZoom.asm: the Special Zoom (BETA) setting. The camera zooms in on a fighter
 * whose hit is a KO (Toggles.entry_special_zoom 1 or 2) or whose attacker is in a special move (any KO setting), and the
 * game runs in slow motion while the zoom lasts.
 *
 * ftRemixSpecialZoomFinalKnockback: ftCommonDamageInitDamageVars (RAM 0x801414E4, before is_hitstun is set)
 * ftRemixSpecialZoomCameraOffset: gmCameraUpdatePlayerZoom (RAM 0x8010C9BC)
 * ftRemixSpecialZoomMakeWallpaper: gmCameraMakeWallpaperCamera (RAM 0x8010DC10)
 * ftRemixSpecialZoomSkipsObjectsUpdate: gcRunAll (RAM 0x8000A5EC)
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <if/interface.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/taskman.h>
#include <gm/gmcamera.h>
#include <gm/gmremixstamina.h>
#include <gm/gmremixtoggle.h>
#include "ft/ftremix/ftremixhooks.h"

#define FTREMIX_SPECIAL_ZOOM_NONE 0
#define FTREMIX_SPECIAL_ZOOM_KO 1
#define FTREMIX_SPECIAL_ZOOM_SPECIAL_MOVE 2

#define FTREMIX_SPECIAL_ZOOM_DURATION 0x28
#define FTREMIX_SPECIAL_ZOOM_MAX_OFFSET 600.0F
#define FTREMIX_SPECIAL_ZOOM_CAMERA_DIST 2000.0F
#define FTREMIX_SPECIAL_ZOOM_CAMERA_PAN_SCALE 0.1F
#define FTREMIX_SPECIAL_ZOOM_CAMERA_FOV 28.0F
#define FTREMIX_SPECIAL_ZOOM_CAMERA_NORMAL 0
#define FTREMIX_SPECIAL_ZOOM_GAME_STATUS_PAUSED 2

/* Global.asm screen ids where the zoom runs (VS, Training, Remix modes and the 1P battle) */
#define FTREMIX_SPECIAL_ZOOM_SCREEN_TITLE_AND_1P 0x01
#define FTREMIX_SPECIAL_ZOOM_SCREEN_VS_BATTLE 0x16
#define FTREMIX_SPECIAL_ZOOM_SCREEN_TRAINING 0x36
#define FTREMIX_SPECIAL_ZOOM_SCREEN_REMIX_MODES 0x77

/* Sound effects played when the stage is hidden for a KO zoom (SpecialZoom.asm apply_zoom) */
#define FTREMIX_SPECIAL_ZOOM_FGM_HIDE_STAGE 152
#define FTREMIX_SPECIAL_ZOOM_FGM_HIDE_STAGE_2 187

extern alSoundEffect* func_800269C0_275C0(u16);

/* zoom_timer, zoom_type, the values saved for the restore, and zoom_offset of the camera (SpecialZoom.asm) */
static s32 sFTRemixSpecialZoomTimer;
static s32 sFTRemixSpecialZoomType;
static u8 sFTRemixSpecialZoomMagnifyDisplaySave;
static GObj *sFTRemixSpecialZoomZoomedInGObjSave;
static u8 sFTRemixSpecialZoomCameraModeSave;
static Vec2f sFTRemixSpecialZoomOffset;

/**
 * TRUE when the current screen is one where the Special Zoom runs.
 */
static sb32 ftRemixSpecialZoomIsBattleScreen(void)
{
    s32 screen = gSCManagerSceneData.scene_curr;

    return (screen == FTREMIX_SPECIAL_ZOOM_SCREEN_VS_BATTLE) || (screen == FTREMIX_SPECIAL_ZOOM_SCREEN_TRAINING)
        || (screen == FTREMIX_SPECIAL_ZOOM_SCREEN_REMIX_MODES) || (screen == FTREMIX_SPECIAL_ZOOM_SCREEN_TITLE_AND_1P);
}

/**
 * TRUE when the game is paused outside Training mode, where the zoom timer and the camera stop.
 */
static sb32 ftRemixSpecialZoomIsPausedOutsideTraining(void)
{
    if (gSCManagerSceneData.scene_curr == FTREMIX_SPECIAL_ZOOM_SCREEN_TRAINING)
    {
        return FALSE;
    }
    return gSCManagerBattleState->game_status >= FTREMIX_SPECIAL_ZOOM_GAME_STATUS_PAUSED;
}

/**
 * Offset of the zoom camera toward the attacker: half of the vector from the fighter to the attacker, or none when there
 * is no attacker (SpecialZoom.asm update_offset).
 */
static void ftRemixSpecialZoomUpdateOffset(FTStruct *fp, s32 damage_player_num)
{
    GObj *attacker_gobj = ftParamGetPlayerNumGObj(damage_player_num);
    Vec3f *fighter_pos = fp->coll_data.p_translate;
    Vec3f *attacker_pos;

    if (attacker_gobj == NULL)
    {
        sFTRemixSpecialZoomOffset.x = 0.0F;
        sFTRemixSpecialZoomOffset.y = 0.0F;
        return;
    }
    attacker_pos = ftGetStruct(attacker_gobj)->coll_data.p_translate;
    sFTRemixSpecialZoomOffset.x = (attacker_pos->x - fighter_pos->x) * 0.5F;
    sFTRemixSpecialZoomOffset.y = (attacker_pos->y - fighter_pos->y) * 0.5F;
}

/**
 * Points the camera at the fighter and zooms it in, and saves the camera and magnifying glass states to restore them
 * (SpecialZoom.asm set_camera_zoom_in_mode).
 */
static void ftRemixSpecialZoomCameraZoomIn(GObj *fighter_gobj)
{
    sFTRemixSpecialZoomZoomedInGObjSave = gGMCameraStruct.pzoom_fighter_gobj;
    sFTRemixSpecialZoomCameraModeSave = gGMCameraStruct.status_curr;

    gmCameraSetStatusPlayerZoom(fighter_gobj, 0.0F, 0.0F, FTREMIX_SPECIAL_ZOOM_CAMERA_DIST, FTREMIX_SPECIAL_ZOOM_CAMERA_PAN_SCALE, FTREMIX_SPECIAL_ZOOM_CAMERA_FOV);
    gIFCommonPlayerInterface.magnify_mode = 0;
}

/** TRUE while a Special Zoom is on (Zoom.zoom_type is not zero). */
sb32 ftRemixSpecialZoomIsActive(void)
{
    return (sFTRemixSpecialZoomType != FTREMIX_SPECIAL_ZOOM_NONE) ? TRUE : FALSE;
}

/**
 * Ends the zoom: restores the camera and the magnifying glass, shows the stage again and destroys the background
 * (SpecialZoom.asm end_zoom). Does nothing when no zoom is active.
 */
static void ftRemixSpecialZoomEnd(void)
{
    if (sFTRemixSpecialZoomType == FTREMIX_SPECIAL_ZOOM_NONE)
    {
        return;
    }
    gmCameraSetStatus(sFTRemixSpecialZoomCameraModeSave);
    gGMCameraStruct.pzoom_fighter_gobj = sFTRemixSpecialZoomZoomedInGObjSave;
    gIFCommonPlayerInterface.magnify_mode = sFTRemixSpecialZoomMagnifyDisplaySave;

    /* REMIX-TODO(Render): end_zoom shows the stage groups 1 and 2 again (temp_toggle_group_display_, Render.GROUP_TABLE);
     * the group lists have no port yet. */
    ftRemixSpecialZoomWallpaperHide();

    sFTRemixSpecialZoomTimer = 0;
    sFTRemixSpecialZoomType = FTREMIX_SPECIAL_ZOOM_NONE;
}

/**
 * Starts a zoom of the given kind on the fighter (SpecialZoom.asm apply_zoom). A special move zoom does not replace a KO
 * zoom that is running. The camera only zooms in for a KO, or a special move zoom in a 2-player match.
 */
static void ftRemixSpecialZoomApply(GObj *fighter_gobj, s32 zoom_flag)
{
    if (sFTRemixSpecialZoomTimer > 0)
    {
        if ((sFTRemixSpecialZoomType == FTREMIX_SPECIAL_ZOOM_KO) && (zoom_flag == FTREMIX_SPECIAL_ZOOM_SPECIAL_MOVE))
        {
            return;
        }
        ftRemixSpecialZoomEnd();
    }
    sFTRemixSpecialZoomType = zoom_flag;
    sFTRemixSpecialZoomMagnifyDisplaySave = gIFCommonPlayerInterface.magnify_mode;

    if ((zoom_flag != FTREMIX_SPECIAL_ZOOM_SPECIAL_MOVE) || (ftRemixSpecialZoomCountPlayersInMatch() == 2))
    {
        ftRemixSpecialZoomCameraZoomIn(fighter_gobj);
    }
    sFTRemixSpecialZoomTimer = FTREMIX_SPECIAL_ZOOM_DURATION;

    if (zoom_flag == FTREMIX_SPECIAL_ZOOM_KO)
    {
        /* REMIX-TODO(Render): apply_zoom hides the stage groups 1 and 2 (temp_toggle_group_display_); the group lists have no
         * port yet. */
        func_800269C0_275C0(FTREMIX_SPECIAL_ZOOM_FGM_HIDE_STAGE);
        func_800269C0_275C0(FTREMIX_SPECIAL_ZOOM_FGM_HIDE_STAGE_2);
    }
    if (!gmRemixToggle(flash_guard))
    {
        ftRemixSpecialZoomWallpaperShow();
    }
}

/**
 * Runs the zoom's camera step on a slow-motion frame: the camera keeps moving unless a camera mode or the time twister
 * blocks it, and the timer counts down (and ends the zoom at zero). Nothing runs while the game is paused.
 */
static void ftRemixSpecialZoomSlowMoCameraStep(void)
{
    sb32 is_camera_blocked;

    if (ftRemixSpecialZoomIsPausedOutsideTraining())
    {
        return;
    }
    is_camera_blocked = (gmRemixToggle(camera_mode) != FTREMIX_SPECIAL_ZOOM_CAMERA_NORMAL)
        || (gGMCameraStruct.func_camera == gmCameraMapZoomFuncCamera);

    if (!is_camera_blocked)
    {
        gmCameraRunFuncCamera(gGMCameraGObj);
    }
    sFTRemixSpecialZoomTimer--;

    if (sFTRemixSpecialZoomTimer == 0)
    {
        ftRemixSpecialZoomEnd();
    }
}

/**
 * Makes the wallpaper for the Special Zoom when the stage camera is made (gmCameraMakeWallpaperCamera); resets the zoom
 * state of the previous match (create_zoom_camera).
 */
void ftRemixSpecialZoomMakeWallpaper(void)
{
    if (!gmRemixToggle(special_zoom))
    {
        return;
    }
    sFTRemixSpecialZoomTimer = 0;
    sFTRemixSpecialZoomType = FTREMIX_SPECIAL_ZOOM_NONE;

    ftRemixSpecialZoomWallpaperLoad();
    ftRemixSpecialZoomWallpaperCreateCamera();
}

/**
 * Moves the zoom focus of the stage camera by the zoom offset while the zoom runs (zoomed_in_camera_offset). An offset of
 * the max offset or more is ignored.
 */
void ftRemixSpecialZoomCameraOffset(Vec3f *pos)
{
    if (!gmRemixToggle(special_zoom) || (sFTRemixSpecialZoomTimer <= 0))
    {
        return;
    }
    if ((ABSF(sFTRemixSpecialZoomOffset.x) >= FTREMIX_SPECIAL_ZOOM_MAX_OFFSET) || (ABSF(sFTRemixSpecialZoomOffset.y) >= FTREMIX_SPECIAL_ZOOM_MAX_OFFSET))
    {
        return;
    }
    pos->x += sFTRemixSpecialZoomOffset.x;
    pos->y += sFTRemixSpecialZoomOffset.y;
}

/**
 * Runs the slow motion of the zoom in place of gcRunAll (objects_update): one object update every 16 game updates, the
 * camera step on the others. Returns TRUE when gcRunAll must skip the object update of this game update.
 */
sb32 ftRemixSpecialZoomSkipsObjectsUpdate(void)
{
    if (!gmRemixToggle(special_zoom))
    {
        return FALSE;
    }
    if (!ftRemixSpecialZoomIsBattleScreen())
    {
        sFTRemixSpecialZoomTimer = 0;
        return FALSE;
    }
    if ((sFTRemixSpecialZoomTimer <= 0) || ((dSYTaskmanUpdateCount & 0xF) == 0))
    {
        return FALSE;
    }
    ftRemixSpecialZoomSlowMoCameraStep();

    if ((dSYTaskmanUpdateCount & 0x1) != 0)
    {
        ftRemixSpecialZoomWallpaperPlayAnim();
    }
    return TRUE;
}

/**
 * Decides the Special Zoom of a fighter's hit, after the knockback is set (SpecialZoom.asm final_knockback_was_set). A
 * KO zoom needs the KO setting (any KO, or a KO while the decisive stock is the last one), the special move zoom is checked
 * after it. Stamina battles get no zoom at all (SpecialZoom.asm check_gamemode_2 jumps to the original function).
 */
void ftRemixSpecialZoomFinalKnockback(FTStruct *fp, s32 damage_player_num)
{
    s32 toggle = gmRemixToggle(special_zoom);
    sb32 is_decisive_stock;
    sb32 is_ko_zoom_allowed;

    if ((toggle == 0) || !ftRemixSpecialZoomIsBattleScreen())
    {
        return;
    }
    if (gmRemixStaminaIsActive())
    {
        return;
    }
    is_decisive_stock = ftRemixSpecialZoomIsDecisiveStock(fp);
    ftRemixSpecialZoomUpdateOffset(fp, damage_player_num);

    is_ko_zoom_allowed = (toggle == 2) || is_decisive_stock;
    if (is_ko_zoom_allowed && ftRemixSpecialZoomCheckKO(fp))
    {
        ftRemixSpecialZoomApply(fp->fighter_gobj, FTREMIX_SPECIAL_ZOOM_KO);
        return;
    }
    if (ftRemixSpecialZoomIsAttackerSpecialMove(ftParamGetPlayerNumGObj(damage_player_num)))
    {
        ftRemixSpecialZoomApply(fp->fighter_gobj, FTREMIX_SPECIAL_ZOOM_SPECIAL_MOVE);
    }
}

#endif
