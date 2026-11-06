/**
 * ftremixvisibility.c - Smash Remix Visibility option (css/Visibility.asm, CharEnvColor.asm initialize_override_table_
 * and update_player_indicator_alpha_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixenvcolor.h"
#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmremixpanel.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>

#define FTREMIX_VISIBILITY_VALUE_CLOAKED 1
#define FTREMIX_VISIBILITY_VALUE_NONE 2
#define FTREMIX_VISIBILITY_VALUE_DARK 3
#define FTREMIX_VISIBILITY_OPAQUE 0xFF

/**
 * Maps a Visibility panel value to an env colour state: 1 Cloaked (override 0xFFFFFF10), 2 None, 3 Dark,
 * anything else Default (CharEnvColor.asm initialize_override_table_).
 */
static FTRemixEnvColorState ftRemixVisibilityStateOfValue(s32 value)
{
    switch (value)
    {
    case FTREMIX_VISIBILITY_VALUE_CLOAKED:
        return nFTRemixEnvColorStateCloaked;

    case FTREMIX_VISIBILITY_VALUE_NONE:
        return nFTRemixEnvColorStateNone;

    case FTREMIX_VISIBILITY_VALUE_DARK:
        return nFTRemixEnvColorStateDark;

    default:
        return nFTRemixEnvColorStateNormal;
    }
}

/** TRUE on the How to Play and Auto-Demo screens, where the asm clears the override of the fighter. */
static sb32 ftRemixVisibilityIsDemoScene(void)
{
    u8 scene = gSCManagerSceneData.scene_curr;

    return (scene == nSCKindExplain) || (scene == nSCKindAutoDemo);
}

/**
 * Sets the env colour state of the fighter's port from the Visibility panel, then applies that state as the override.
 * Called from ftManagerInitFighter (asm initialize_override_table_). Moveset overrides are not touched.
 */
void ftRemixVisibilityInitFighter(FTStruct *fp)
{
    FTRemixEnvColorState state = nFTRemixEnvColorStateNormal;

    if ((fp->player < 0) || (fp->player >= FTREMIX_ENVCOLOR_PORTS))
    {
        return;
    }
    if (!ftRemixVisibilityIsDemoScene())
    {
        state = ftRemixVisibilityStateOfValue(gmRemixPanel(VISIBILITY, fp->player));
    }
    ftRemixEnvColorSetState(fp->player, state);
    ftRemixEnvColorRestoreState(fp->player);
}

/**
 * Applies a Visibility panel value to the env colour override of a port right away (css/Visibility.asm onchange_handler),
 * so the fighter shown on the CSS changes while the panel menu is open.
 *
 * @param port Port whose option changed.
 * @param value New Visibility panel value.
 */
void ftRemixVisibilityApplyPanelValue(s32 port, s32 value)
{
    if ((port < 0) || (port >= FTREMIX_ENVCOLOR_PORTS))
    {
        return;
    }
    ftRemixEnvColorSetState(port, ftRemixVisibilityStateOfValue(value));
    ftRemixEnvColorRestoreState(port);
}

/**
 * Sets the alpha of the player tag sprite to the alpha of the env colour override of the port, or opaque when the
 * port has none. Called from ifCommonPlayerTagProcDisplay before the tag is drawn (asm update_player_indicator_alpha_).
 */
void ftRemixVisibilityPlayerTagSetAlpha(SObj *sobj, s32 player)
{
    u32 rgba = ftRemixEnvColorGetOverride(player);

    sobj->sprite.alpha = (rgba != 0) ? (u8)(rgba & 0xFF) : FTREMIX_VISIBILITY_OPAQUE;
}

#endif
