/**
 * gmremixtagteam.c - Remix Tag Team character queues and the fighter swap on a lost stock (TagTeam.asm start_match_setup_,
 * use_character_queues_on_load_, load_next_char_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <it/item.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixtagteam.h"
#include "gm/gmremixtagteamrules.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_modes.h"

typedef struct GMRemixTagTeamSlot
{
    s32 fkind;
    s32 costume;
} GMRemixTagTeamSlot;

static GMRemixTagTeamSlot sGMRemixTagTeamQueues[GMCOMMON_PLAYERS_MAX][GMREMIX_TAGTEAM_QUEUE_MAX];

sb32 gmRemixTagTeamIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_TAG_TEAM);
}

static s32 gmRemixTagTeamPickRandomFighter(void)
{
    return gmRemixTagTeamRulesRandomFighter(syUtilsRandIntRange(gmRemixTagTeamRulesRandomCount()));
}

static s32 gmRemixTagTeamGetCostume(s32 port, s32 fkind)
{
    SCPlayerData *data = &gSCManagerBattleState->players[port];

    if (gSCManagerBattleState->is_team_battle != FALSE)
    {
        return ftParamGetCostumeTeamID(fkind, data->color);
    }
    return 0;
}

void gmRemixTagTeamSetup(void)
{
    s32 port;
    s32 slot;

    if (gmRemixTagTeamIsActive() == FALSE)
    {
        return;
    }
    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        SCPlayerData *data = &gSCManagerBattleState->players[port];

        if (data->pkind == nFTPlayerKindNot)
        {
            continue;
        }
        sGMRemixTagTeamQueues[port][0].fkind = data->fkind;
        sGMRemixTagTeamQueues[port][0].costume = data->costume;

        for (slot = 1; slot < GMREMIX_TAGTEAM_QUEUE_MAX; slot++)
        {
            s32 fkind = remix_modes_tagteam_slot(port, slot);

            if (fkind < 0)
            {
                fkind = gmRemixTagTeamPickRandomFighter();
            }
            sGMRemixTagTeamQueues[port][slot].fkind = fkind;
            sGMRemixTagTeamQueues[port][slot].costume = gmRemixTagTeamGetCostume(port, fkind);
        }
        port_log("tagteam: port %d queue %d %d %d %d %d %d\n", (int)port, (int)sGMRemixTagTeamQueues[port][0].fkind,
                 (int)sGMRemixTagTeamQueues[port][1].fkind, (int)sGMRemixTagTeamQueues[port][2].fkind,
                 (int)sGMRemixTagTeamQueues[port][3].fkind, (int)sGMRemixTagTeamQueues[port][4].fkind,
                 (int)sGMRemixTagTeamQueues[port][5].fkind);
    }
}

/* The slot of the port's current stock: the stocks set minus the stocks left (the asm adds the stolen and LAST offsets). */
static s32 gmRemixTagTeamGetSlot(FTStruct *fp)
{
    return gmRemixTagTeamRulesGetSlot(gSCManagerBattleState->stocks, fp->stock_count, GMREMIX_TAGTEAM_QUEUE_MAX);
}

static void gmRemixTagTeamMoveOwnership(GObj *old_gobj, GObj *new_gobj)
{
    GObj *gobj;

    for (gobj = gGCCommonLinks[nGCCommonLinkIDItem]; gobj != NULL; gobj = gobj->link_next)
    {
        ITStruct *ip = itGetStruct(gobj);

        if (ip->owner_gobj == old_gobj)
        {
            ip->owner_gobj = new_gobj;
        }
    }
    for (gobj = gGCCommonLinks[nGCCommonLinkIDWeapon]; gobj != NULL; gobj = gobj->link_next)
    {
        WPStruct *wp = wpGetStruct(gobj);

        if (wp->owner_gobj == old_gobj)
        {
            wp->owner_gobj = new_gobj;
        }
    }
}

static GObj* gmRemixTagTeamMakeFighter(FTStruct *old_fp, GMRemixTagTeamSlot *slot)
{
    SCPlayerData *data = &gSCManagerBattleState->players[old_fp->player];
    FTDesc desc = dFTManagerDefaultFighterDesc;

    ftManagerSetupFilesAllKind(slot->fkind);

    desc.fkind = slot->fkind;
    desc.lr = old_fp->lr;
    desc.team = old_fp->team;
    desc.player = old_fp->player;
    desc.detail = ((gSCManagerBattleState->pl_count + gSCManagerBattleState->cp_count) < 3) ? nFTPartsDetailHigh : nFTPartsDetailLow;
    desc.costume = slot->costume;
    desc.shade = data->shade;
    desc.handicap = data->handicap;
    desc.level = data->level;
    desc.stock_count = old_fp->stock_count;
    desc.damage = 0;
    desc.is_skip_entry = TRUE;
    desc.is_skip_shadow_setup = TRUE;
    desc.pkind = data->pkind;
    desc.controller = &gSYControllerDevices[old_fp->player];
    desc.figatree_heap = ftManagerAllocFigatreeHeapKind(slot->fkind);

    return ftManagerMakeFighter(&desc);
}

GObj* gmRemixTagTeamLoadNextFighter(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GMRemixTagTeamSlot *slot;
    GObj *new_gobj;

    if (gmRemixTagTeamIsActive() == FALSE)
    {
        return fighter_gobj;
    }
    slot = &sGMRemixTagTeamQueues[fp->player][gmRemixTagTeamGetSlot(fp)];

    if (gmRemixTagTeamRulesNeedsSwap(slot->fkind, fp->fkind) == 0)
    {
        return fighter_gobj;
    }
    port_log("tagteam: port %d swaps fkind %d for %d\n", (int)fp->player, (int)fp->fkind, (int)slot->fkind);

    new_gobj = gmRemixTagTeamMakeFighter(fp, slot);
    gmRemixTagTeamMoveOwnership(fighter_gobj, new_gobj);
    gSCManagerBattleState->players[fp->player].fighter_gobj = new_gobj;
    gSCManagerBattleState->players[fp->player].stock_damage_all = 0;
    ftManagerDestroyFighter(fighter_gobj);
    ftParamUnlockPlayerControl(new_gobj);

    return new_gobj;
}

#endif
