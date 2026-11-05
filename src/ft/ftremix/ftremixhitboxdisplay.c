/*
 * ftremixhitboxdisplay.c - Special Model Display and Model Display of Smash Remix (Hitbox.asm, Costumes.asm).
 *
 * Special Model Display (Toggles.asm entry_special_model: 0 off, 1 hitbox, 2 hitbox+model, 3 ECB) sets the display mode of
 * every fighter and every item each frame (hitbox_mode_), of every projectile when it is drawn (projectile_), and tells
 * the draw code of fighters, items and projectiles when to draw the hitboxes over the model (hitbox+model). Model Display
 * (Toggles.asm entry_model_display: 0 default, 1 high poly, 2 low poly) forces the poly detail of a fighter on load and
 * on every ftParamSetModelPartDetailAll call, and is ignored on HTP, Demo and the intro scenes, as in Costumes.asm.
 * Hitbox.asm has no such screen check. The display-list builders are in ftremixhitbox.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <it/item.h>
#include <sc/scene.h>
#include <sc/scmanager.h>
#include <sys/develop.h>
#include <sys/objman.h>
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixhitbox.h"
#include <gm/gmremixpanel.h>
#include "gm/gmremixtoggle.h"

#define FTREMIX_MODEL_DISPLAY_DEFAULT 0                 /* Toggles.asm entry_model_display and CSS Model: default */
#define FTREMIX_HITBOX_DISPLAY_HITBOX 1                /* Toggles.asm entry_special_model: hitbox */
#define FTREMIX_HITBOX_DISPLAY_HITBOX_PLUS_MODEL 2      /* Toggles.asm entry_special_model: hitbox+model */
#define FTREMIX_HITBOX_DISPLAY_ECB 3                    /* Toggles.asm entry_special_model: ECB */
#define FTREMIX_MODEL_DISPLAY_HIGH 1                    /* Toggles.asm entry_model_display: always high poly */
#define FTREMIX_MODEL_DISPLAY_LOW 2                     /* Toggles.asm entry_model_display: always low poly */

/*
 * Costumes.asm force_hi_lo_poly_ skips HTP, Demo and the intro scenes (screen ids N64_LOGO up to INTRO_19); Hitbox.asm
 * hitbox_mode_ has no such check. The port's scene enum uses the same ids as the Remix screen ids (see grmetallic, nSCKind1PTrainingMode = 0x36).
 */
static sb32 ftRemixHitboxDisplayIsIgnoredScene(void)
{
    s32 scene = gSCManagerSceneData.scene_curr;

    if ((scene == nSCKindExplain) || (scene == nSCKindAutoDemo))
    {
        return TRUE;
    }
    return (scene >= nSCKindStartup) && (scene <= nSCKindOpeningNewcomers);
}

/**
 * Model Display: returns the poly detail a fighter must use. The port's CSS panel value wins when it is not default,
 * otherwise the universal toggle applies (Costumes.asm force_hi_lo_poly_ checks the toggle first and the per-port value
 * second, so the per-port value overrides it).
 *
 * @param detail The detail the vanilla code would use (nFTPartsDetailHigh or nFTPartsDetailLow).
 * @param player The port of the fighter (0 to 3).
 * @return The detail to use.
 */
u8 ftRemixModelDisplayDetail(u8 detail, s32 player)
{
    s32 choice = gmRemixPanel(MODEL_DISPLAY, player);

    if (choice == FTREMIX_MODEL_DISPLAY_DEFAULT)
    {
        choice = gmRemixToggle(model_display);
    }
    if (ftRemixHitboxDisplayIsIgnoredScene())
    {
        return detail;
    }
    if (choice == FTREMIX_MODEL_DISPLAY_HIGH)
    {
        return nFTPartsDetailHigh;
    }
    if (choice == FTREMIX_MODEL_DISPLAY_LOW)
    {
        return nFTPartsDetailLow;
    }
    return detail;
}

/**
 * Special Model Display: returns the toggle value as the display mode (nDBDisplayMode*) that Hitbox.asm projectile_ and the
 * item loop of hitbox_mode_ write: 1 hitbox, 2 hitbox+model (outlined hitboxes), 3 ECB, otherwise 0 (model).
 *
 * @return The display mode of projectiles and items.
 */
s32 ftRemixHitboxDisplayObjectMode(void)
{
    switch (gmRemixToggle(special_model))
    {
        case FTREMIX_HITBOX_DISPLAY_HITBOX:
            return nDBDisplayModeHitCollisionFill;
        case FTREMIX_HITBOX_DISPLAY_HITBOX_PLUS_MODEL:
            return nDBDisplayModeHitAttackOutline;
        case FTREMIX_HITBOX_DISPLAY_ECB:
            return nDBDisplayModeMapCollision;
        default:
            return nDBDisplayModeMaster;
    }
}

/**
 * Hitbox+model: the model is drawn and the hitboxes are drawn over it. Hitbox.asm render_hitbox_plus_model_,
 * render_hitbox_plus_projectile_, render_hitbox_plus_item_ and make_hitboxes_transparent_ all test the toggle for 2.
 *
 * @return TRUE when the toggle is hitbox+model.
 */
sb32 ftRemixHitboxDisplayIsOverlay(void)
{
    return gmRemixToggle(special_model) == FTREMIX_HITBOX_DISPLAY_HITBOX_PLUS_MODEL;
}

/**
 * Hitbox.asm hitbox_mode_ item loop: writes the display state (ITStruct 0x0374) of every item object. The loop starts at
 * FIRST_ITEM_PTR (RAM 0x80046700), the first object of the item link, and follows link_next.
 */
static void ftRemixHitboxDisplayApplyToItems(void)
{
    s32 mode = ftRemixHitboxDisplayObjectMode();
    GObj *item_gobj;

    for (item_gobj = gGCCommonLinks[nGCCommonLinkIDItem]; item_gobj != NULL; item_gobj = item_gobj->link_next)
    {
        ITStruct *ip = itGetStruct(item_gobj);

        if (ip != NULL)
        {
            ip->display_mode = mode;
        }
    }
}

/**
 * Special Model Display: sets the display mode of a fighter from the toggle, and of every item. The mode maps to the port
 * display modes of ftDisplayMainProcDisplay. Hitbox.asm hitbox_mode_ writes the state every frame, 0 included, so a toggle
 * of 0 returns the fighter to the model; the HitboxView enhancement applies after this call.
 *
 * @param fp The fighter struct whose display mode is set.
 */
void ftRemixHitboxDisplayApply(FTStruct *fp)
{
    switch (gmRemixToggle(special_model))
    {
        case FTREMIX_HITBOX_DISPLAY_HITBOX:
            fp->display_mode = nDBDisplayModeHitCollisionFill;
            break;
        case FTREMIX_HITBOX_DISPLAY_HITBOX_PLUS_MODEL:
            /* hitbox_mode_ keeps the fighter in state 0 so the model shows; the hitbox pass is added by
             * render_hitbox_plus_model_ (RAM 0x800F2FD0), see ftRemixHitboxDisplayIsOverlay */
            fp->display_mode = nDBDisplayModeMaster;
            break;
        case FTREMIX_HITBOX_DISPLAY_ECB:
            fp->display_mode = nDBDisplayModeMapCollision;
            break;
        default:
            fp->display_mode = nDBDisplayModeMaster;
            break;
    }
    ftRemixHitboxDisplayApplyToItems();
}

/**
 * Hitbox.asm fix_hitbox_position_ (RAM 0x800F3310): when hitbox or hitbox+model is on, the hitbox of a fighter drawn on a
 * screen whose fighter camera is not the main camera skips the mooring (the RotRpyR matrix kind) of the vanilla code and
 * is drawn as on the main camera. The screens are the CSS screens (PlayersVS to 1PBonus2Players), the VS results, the 1P
 * intro, the opening room and running scenes, and the ending room. The asm also lists the 1P screen id (TITLE_AND_1P) when
 * the first loaded file is 0x4F, the Continue image: the port keeps the Continue screen as its own scene.
 *
 * @return TRUE when the hitbox is drawn without the mooring.
 */
sb32 ftRemixHitboxDisplayFixesPosition(void)
{
    s32 toggle = gmRemixToggle(special_model);
    s32 scene = gSCManagerSceneData.scene_curr;

    if ((toggle != FTREMIX_HITBOX_DISPLAY_HITBOX) && (toggle != FTREMIX_HITBOX_DISPLAY_HITBOX_PLUS_MODEL))
    {
        return FALSE;
    }
    if ((scene >= nSCKindPlayersVS) && (scene <= nSCKind1PBonus2Players))
    {
        return TRUE;
    }
    switch (scene)
    {
        case nSCKind1PIntro:
        case nSCKindOpeningRoom:
        case nSCKindOpeningRun:
        case nSCKindVSResults:
        case nSCKindEnding:
        case nSCKind1PContinue:
            return TRUE;
        default:
            return FALSE;
    }
}

#endif
