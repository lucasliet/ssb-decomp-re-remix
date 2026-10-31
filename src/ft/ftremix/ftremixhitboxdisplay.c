/*
 * ftremixhitboxdisplay.c - Special Model Display and Model Display of Smash Remix (Hitbox.asm, Costumes.asm).
 *
 * Special Model Display (Toggles.asm entry_special_model: 0 off, 1 hitbox, 2 hitbox+model, 3 ECB) sets the display mode of
 * every fighter each frame. Model Display (Toggles.asm entry_model_display: 0 default, 1 high poly, 2 low poly) forces the
 * poly detail of a fighter on load and on every ftParamSetModelPartDetailAll call. Both are ignored on HTP, Demo and the
 * intro scenes, as in the asm. Items, projectiles and the Advanced Hurtbox Display are not ported here (see the notes).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scene.h>
#include <sc/scmanager.h>
#include <sys/develop.h>
#include "ft/ftremix/ftremixhooks.h"
#include <gm/gmremixpanel.h>
#include "gm/gmremixtoggle.h"

#define FTREMIX_MODEL_DISPLAY_DEFAULT 0                 /* Toggles.asm entry_model_display and CSS Model: default */
#define FTREMIX_HITBOX_DISPLAY_HITBOX 1                /* Toggles.asm entry_special_model: hitbox */
#define FTREMIX_HITBOX_DISPLAY_HITBOX_PLUS_MODEL 2      /* Toggles.asm entry_special_model: hitbox+model */
#define FTREMIX_HITBOX_DISPLAY_ECB 3                    /* Toggles.asm entry_special_model: ECB */
#define FTREMIX_MODEL_DISPLAY_HIGH 1                    /* Toggles.asm entry_model_display: always high poly */
#define FTREMIX_MODEL_DISPLAY_LOW 2                     /* Toggles.asm entry_model_display: always low poly */

/*
 * Costumes.asm force_hi_lo_poly_ and Hitbox.asm hitbox_mode_ skip HTP, Demo and the intro scenes (screen ids N64_LOGO up to
 * INTRO_19). The port's scene enum uses the same ids as the Remix screen ids (see grmetallic, nSCKind1PTrainingMode = 0x36).
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
 * Special Model Display: sets the display mode of a fighter from the toggle. The mode maps to the port display modes of
 * ftDisplayMainProcDisplay. A toggle of 0 leaves the mode as it is, so vanilla and the HitboxView enhancement are unchanged.
 * Hitbox.asm hitbox_mode_.
 *
 * @param fp The fighter struct whose display mode is set.
 */
void ftRemixHitboxDisplayApply(FTStruct *fp)
{
    s32 toggle = gmRemixToggle(special_model);

    if (ftRemixHitboxDisplayIsIgnoredScene())
    {
        return;
    }
    switch (toggle)
    {
        case FTREMIX_HITBOX_DISPLAY_HITBOX:
            fp->display_mode = nDBDisplayModeHitCollisionFill;
            break;
        case FTREMIX_HITBOX_DISPLAY_HITBOX_PLUS_MODEL:
            /* DEVIATION(render_hitbox_plus_model_ not ported): Hitbox.asm hitbox_mode_ (0x800F2BFC) keeps the fighter in
             * state 0, so the model stays visible, and render_hitbox_plus_model_ (Hitbox.asm 0x800F2FD0) draws the hitbox
             * over it. Until that overlay hook exists, the fighter shows only its model. */
            fp->display_mode = nDBDisplayModeMaster;
            break;
        case FTREMIX_HITBOX_DISPLAY_ECB:
            fp->display_mode = nDBDisplayModeMapCollision;
            break;
        default:
            break;
    }
}

#endif
