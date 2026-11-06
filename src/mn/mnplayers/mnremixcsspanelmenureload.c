/**
 * mnremixcsspanelmenureload.c - change handlers of the Remix CSS panel menu items that act on the fighter shown on the CSS, see
 * mnremixcsspanelmenureload.h.
 *
 * css/ModelDisplay.asm:13 (onchange_handler reloads the model through 0x800E9198), css/KirbyHat.asm:76 (copy id and hat of Kirby
 * through 0x800E8EAC and 0x800E8ECC, the hat texture index is kept) and css/Visibility.asm:37 (env colour override of the port).
 * CharacterSelectDebugMenu.asm calls the handler of an item after an arrow press and for every item of the reset button; a3 is the
 * fighter GObj of the panel, 0 when none is shown.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <sys/objman.h>
#include "ft/ftremix/ftremixhooks.h"
#include "mn/mnplayers/mnremixcsspanelmenureload.h"
#include "mn/mnplayers/mnremixcsspanelmenurules.h"
#include "remix/remix_roster_data.h"

_Static_assert(MNREMIX_PANELMENU_DETAIL_HIGH == nFTPartsDetailHigh, "Model handler asks for high poly");
_Static_assert(MNREMIX_PANELMENU_DETAIL_LOW == nFTPartsDetailLow, "Model handler asks for low poly");

/**
 * Finds the fighter the CSS made for a port.
 *
 * @param port Port of the panel.
 * @return The fighter GObj, or NULL when the panel shows none.
 */
static GObj *mnRemixPanelMenuFindFighter(s32 port)
{
    GObj *fighter_gobj;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        if (ftGetStruct(fighter_gobj)->player == port)
        {
            return fighter_gobj;
        }
    }
    return NULL;
}

static sb32 mnRemixPanelMenuIsKirby(const FTStruct *fp)
{
    return ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY)) ? TRUE : FALSE;
}

/**
 * Texture index of the hat of a Kirby (the asm reads it from the special images of part 2 before the swap), 0 when the joint has none.
 */
static u16 mnRemixPanelMenuGetHatTextureID(const FTStruct *fp)
{
    const DObj *hat_joint = fp->joints[FTKIRBY_COPY_MODELPARTS_JOINT];

    if ((hat_joint == NULL) || (hat_joint->mobj == NULL))
    {
        return 0;
    }
    return hat_joint->mobj->texture_id_curr;
}

static void mnRemixPanelMenuSetHatTextureID(const FTStruct *fp, u16 texture_id)
{
    DObj *hat_joint = fp->joints[FTKIRBY_COPY_MODELPARTS_JOINT];

    if ((hat_joint == NULL) || (hat_joint->mobj == NULL))
    {
        return;
    }
    hat_joint->mobj->texture_id_curr = texture_id;
}

/**
 * Reloads the poly detail of a fighter. ftParamSetModelPartDetailAll gives the panel value and the Model Display toggle their say
 * (ftRemixModelDisplayDetail), so the detail asked for only matters for Default.
 */
static void mnRemixPanelMenuReloadModel(GObj *fighter_gobj, s32 value)
{
    ftParamSetModelPartDetailAll(fighter_gobj, (u8)mnRemixPanelMenuRulesModelDetail(value));
}

/**
 * Gives a Kirby the hat of the panel value: stores the copy id, makes the hat part the default of the hat joint and redraws the
 * parts, then puts the texture index of the hat back.
 */
static void mnRemixPanelMenuSwapKirbyHat(GObj *fighter_gobj, s32 value)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTKirbyCopy *copy_table;
    s32 copy_id;
    u16 texture_id;

    if (mnRemixPanelMenuIsKirby(fp) == FALSE)
    {
        return;
    }
    copy_id = mnRemixPanelMenuRulesCssHatFighter(value);
    fp->passive_vars.kirby.copy_id = copy_id;

    copy_table = ftRemixKirbyGetCopyTable();
    texture_id = mnRemixPanelMenuGetHatTextureID(fp);

    ftParamSetModelPartDefaultID(fighter_gobj, FTKIRBY_COPY_MODELPARTS_JOINT, copy_table[copy_id].copy_modelpart_id);
    ftParamResetModelPartAll(fighter_gobj);
    mnRemixPanelMenuSetHatTextureID(fp, texture_id);
}

void mnRemixPanelMenuApplyFighterChange(s32 item, s32 port, s32 value)
{
    GObj *fighter_gobj;
    s32 action = mnRemixPanelMenuRulesChangeAction(item);

    if (action == MNREMIX_PANELMENU_CHANGE_VISIBILITY)
    {
        ftRemixVisibilityApplyPanelValue(port, value);
        return;
    }
    if (action == MNREMIX_PANELMENU_CHANGE_NONE)
    {
        return;
    }
    fighter_gobj = mnRemixPanelMenuFindFighter(port);

    if (fighter_gobj == NULL)
    {
        return;
    }
    if (action == MNREMIX_PANELMENU_CHANGE_MODEL)
    {
        mnRemixPanelMenuReloadModel(fighter_gobj, value);
        return;
    }
    mnRemixPanelMenuSwapKirbyHat(fighter_gobj, value);
}

#endif
