/**
 * gmremixkothglow.c - overlay of the fighters that score on the hill of Remix King of the Hill (KingOfTheHill.asm track_score_
 * _apply_gfx_routines, GFXRoutine.asm KOTH_RED, KOTH_BLUE, KOTH_GREEN, KOTH_YELLOW and KOTH_GRAY).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <gm/gmremixcolscripts.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixkoth.h"
#include "gm/gmremixkothglow.h"
#include "gm/gmremixkothrules.h"
#include "gm/gmremixkothstate.h"
#include "remix/remix_symbols.h"

/* GFXRoutine.asm add_gfx_routine order: KOTH_RED 0x80, KOTH_BLUE 0x81, KOTH_GREEN 0x82, KOTH_YELLOW 0x83 (BALL_FIRE is 0x84).
 * KOTH_GRAY is not in that table: the asm writes its address straight to the fighter, so it is registered on first use. */
static const s32 dGMRemixKothGlowColAnimIDs[] = { 0x80, 0x81, 0x82, 0x83 };

static s32 sGMRemixKothGlowGrayID;

static s32 gmRemixKothGlowGetColAnimID(s32 tag)
{
    if (tag != GMREMIX_KOTHRULES_TAG_GRAY)
    {
        return dGMRemixKothGlowColAnimIDs[tag];
    }
    if (sGMRemixKothGlowGrayID == 0)
    {
        sGMRemixKothGlowGrayID = gmRemixColScriptsRegisterRoutine(REMIX_SYM_GFXRoutine__KOTH_GRAY);
    }
    return sGMRemixKothGlowGrayID;
}

static sb32 gmRemixKothGlowIsCpu(FTStruct *fp)
{
    return ((gSCManagerBattleState->is_team_battle == FALSE) && (gSCManagerBattleState->players[fp->player].pkind == nFTPlayerKindCom)) ? TRUE : FALSE;
}

void gmRemixKothGlowApply(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 tag;
    s32 colanim_id;

    if (gmRemixColScriptsGetOverride(fp->player) != 0)
    {
        return;
    }
    tag = gmRemixKothRulesTag(gmRemixKothGetKey(fp), (gSCManagerBattleState->is_team_battle != FALSE) ? 1 : 0,
                              (gmRemixKothGlowIsCpu(fp) != FALSE) ? 1 : 0);
    colanim_id = gmRemixKothGlowGetColAnimID(tag);

    if (colanim_id == 0)
    {
        return;
    }
    gmRemixColScriptsSetOverride(fp->player, colanim_id);
    ftParamCheckSetFighterColAnimID(fighter_gobj, colanim_id, 0);
}

#endif
