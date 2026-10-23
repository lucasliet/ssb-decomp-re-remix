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

#define FTREMIX_VISIBILITY_VALUE_NONE 2
#define FTREMIX_VISIBILITY_VALUE_DARK 3
#define FTREMIX_VISIBILITY_OPAQUE 0xFF

/**
 * Maps a Visibility panel value to an env colour state. Cloaked (1) is not implemented in Remix,
 * so it keeps the Default state.
 */
static FTRemixEnvColorState ftRemixVisibilityStateOfValue(s32 value)
{
    switch (value)
    {
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
 * Sets the alpha of the player tag sprite to the alpha of the env colour override of the port, or opaque when the
 * port has none. Called from ifCommonPlayerTagProcDisplay before the tag is drawn (asm update_player_indicator_alpha_).
 */
void ftRemixVisibilityPlayerTagSetAlpha(SObj *sobj, s32 player)
{
    u32 rgba = ftRemixEnvColorGetOverride(player);

    sobj->sprite.alpha = (rgba != 0) ? (u8)(rgba & 0xFF) : FTREMIX_VISIBILITY_OPAQUE;
}

#endif
