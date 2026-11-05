/**
 * gmremixtagteam.c - Remix Tag Team character queues and the fighter swap on a lost stock (TagTeam.asm start_match_setup_,
 * use_character_queues_on_load_, load_next_char_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftmanager.h>
#include <it/item.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include "fighter_registry.h"
#include "gm/gmremixmode.h"
#include "gm/gmremixstockmode.h"
#include "gm/gmremixtagteam.h"
#include "gm/gmremixtagteamicons.h"
#include "ft/ftremix/sonic/ftremixsonicclassic.h"
#include "gm/gmremixtagteamrules.h"
#include "if/ifremixtagteamhud.h"
#include "mn/mnplayers/mnremixcostumes.h"
#include "mn/mnplayers/mnremixtagteamcss.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_modes.h"
#include "remix/remix_roster_data.h"

typedef struct GMRemixTagTeamSlot
{
    s32 fkind;
    s32 costume;
    s32 shade;
    sb32 is_classic;
} GMRemixTagTeamSlot;

/* Animation heap a port's fighters share, so a swap does not allocate a new one (TagTeam.asm reuses fp->anim_heap). */
typedef struct GMRemixTagTeamFigatree
{
    void *heap;
    size_t capacity;
} GMRemixTagTeamFigatree;

typedef struct GMRemixTagTeamStolen
{
    s32 port;
    s32 slot;
} GMRemixTagTeamStolen;

static GMRemixTagTeamSlot sGMRemixTagTeamQueues[GMCOMMON_PLAYERS_MAX][GMREMIX_TAGTEAM_QUEUE_MAX];

/* Queue slot a port took from a teammate (stolen_stocks) and times a port had a stock stolen (stolen_count). */
static GMRemixTagTeamStolen sGMRemixTagTeamStolen[GMCOMMON_PLAYERS_MAX];
static s32 sGMRemixTagTeamStolenCount[GMCOMMON_PLAYERS_MAX];

static GMRemixTagTeamFigatree sGMRemixTagTeamFigatrees[GMCOMMON_PLAYERS_MAX];
static s32 sGMRemixTagTeamSwapCount;
static s32 sGMRemixTagTeamHeapAllocCount;

/* Match values of a port that the Stock Mode Last start slot replaced, put back once the first fighters exist. */
static GMRemixTagTeamSlot sGMRemixTagTeamCssPick[GMCOMMON_PLAYERS_MAX];
static sb32 sGMRemixTagTeamIsStartReplaced[GMCOMMON_PLAYERS_MAX];

sb32 gmRemixTagTeamIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_TAG_TEAM);
}

static s32 gmRemixTagTeamPickRandomFighter(void)
{
    return gmRemixTagTeamRulesRandomFighter(syUtilsRandIntRange(gmRemixTagTeamRulesRandomCount()));
}

/* Team battle: the team costume of the fighter. FFA: a Random slot gets a random legal costume (Costumes.get_random_legal_costume_), a menu slot the default one. */
static s32 gmRemixTagTeamGetCostume(s32 port, s32 fkind, sb32 is_random)
{
    SCPlayerData *data = &gSCManagerBattleState->players[port];

    if (gSCManagerBattleState->is_team_battle != FALSE)
    {
        return ftParamGetCostumeTeamID(fkind, data->team);
    }
    return (is_random != FALSE) ? mnRemixCostumeGetRandomLegal(fkind) : 0;
}

/* start_match_setup_ _teams_loop: in a team battle each earlier port with the same fighter in the slot, on the same team, adds one to the shade. */
static s32 gmRemixTagTeamGetShade(s32 port, s32 slot)
{
    s32 previous_fkinds[GMCOMMON_PLAYERS_MAX];
    s32 previous_teams[GMCOMMON_PLAYERS_MAX];
    s32 previous;

    if (gSCManagerBattleState->is_team_battle == FALSE)
    {
        return 0;
    }
    for (previous = 0; previous < port; previous++)
    {
        previous_fkinds[previous] = sGMRemixTagTeamQueues[previous][slot].fkind;
        previous_teams[previous] = gSCManagerBattleState->players[previous].team;
    }
    return gmRemixTagTeamRulesShade(sGMRemixTagTeamQueues[port][slot].fkind, gSCManagerBattleState->players[port].team,
                                    previous_fkinds, previous_teams, port);
}

static void gmRemixTagTeamResetState(void)
{
    s32 port;
    s32 slot;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        sGMRemixTagTeamStolen[port].port = -1;
        sGMRemixTagTeamStolen[port].slot = 0;
        sGMRemixTagTeamStolenCount[port] = 0;
        sGMRemixTagTeamFigatrees[port].heap = NULL;
        sGMRemixTagTeamFigatrees[port].capacity = 0;
        sGMRemixTagTeamIsStartReplaced[port] = FALSE;

        for (slot = 0; slot < GMREMIX_TAGTEAM_QUEUE_MAX; slot++)
        {
            sGMRemixTagTeamQueues[port][slot].fkind = -1;
            sGMRemixTagTeamQueues[port][slot].costume = 0;
            sGMRemixTagTeamQueues[port][slot].shade = 0;
            sGMRemixTagTeamQueues[port][slot].is_classic = FALSE;
        }
    }
    sGMRemixTagTeamSwapCount = 0;
    sGMRemixTagTeamHeapAllocCount = 0;
    gmRemixTagTeamIconsReset();
}

static void gmRemixTagTeamFillMenuSlot(s32 port, s32 slot)
{
    GMRemixTagTeamSlot *queue_slot = &sGMRemixTagTeamQueues[port][slot];
    s32 css_costume;
    sb32 css_is_classic;
    s32 fkind;
    sb32 is_css_pick = mnRemixTagTeamCssGetSlot(port, slot, &fkind, &css_costume, &css_is_classic);
    sb32 is_random;

    /* The queue built on the CSS wins; a Random token of the CSS rolls like a Random slot of the port menu. */
    if ((is_css_pick == FALSE) || (fkind == REMIX_ID_RANDOM))
    {
        is_css_pick = FALSE;
        fkind = remix_modes_tagteam_slot(port, slot);
    }
    is_random = ((is_css_pick == FALSE) && (fkind < 0)) ? TRUE : FALSE;

    if (is_random != FALSE)
    {
        fkind = gmRemixTagTeamPickRandomFighter();
    }
    queue_slot->fkind = fkind;
    queue_slot->costume = ((is_css_pick != FALSE) && (gSCManagerBattleState->is_team_battle == FALSE)) ? css_costume : gmRemixTagTeamGetCostume(port, fkind, is_random);
    queue_slot->shade = gmRemixTagTeamGetShade(port, slot);
    queue_slot->is_classic = (is_css_pick != FALSE) ? css_is_classic : FALSE;
}

/* The CSS queue starts with the first pick, which is not always the last fighter of the panel (the CSS pick the match data holds). */
static void gmRemixTagTeamApplyCssFirstSlot(s32 port)
{
    SCPlayerData *data = &gSCManagerBattleState->players[port];
    GMRemixTagTeamSlot *first_slot = &sGMRemixTagTeamQueues[port][0];
    s32 fkind;
    s32 costume;
    sb32 is_classic;

    if ((mnRemixTagTeamCssGetSlot(port, 0, &fkind, &costume, &is_classic) == FALSE) || (fkind == REMIX_ID_RANDOM))
    {
        return;
    }
    first_slot->fkind = fkind;
    first_slot->costume = (gSCManagerBattleState->is_team_battle != FALSE) ? ftParamGetCostumeTeamID(fkind, data->color) : costume;
    first_slot->is_classic = is_classic;
    first_slot->shade = gmRemixTagTeamGetShade(port, 0);

    if ((first_slot->fkind == sGMRemixTagTeamCssPick[port].fkind) && (first_slot->costume == sGMRemixTagTeamCssPick[port].costume) &&
        (first_slot->is_classic == sGMRemixTagTeamCssPick[port].is_classic))
    {
        return;
    }
    sGMRemixTagTeamIsStartReplaced[port] = TRUE;
    data->fkind = first_slot->fkind;
    data->costume = first_slot->costume;
    data->shade = first_slot->shade;
    ftRemixSonicSetClassic(port, first_slot->is_classic);
}

/* use_character_queues_on_load_: Stock Mode Last with a stored count below the stock setting starts at the queue slot of the skipped stocks. */
static void gmRemixTagTeamApplyStartSlot(s32 port)
{
    SCPlayerData *data = &gSCManagerBattleState->players[port];
    s32 previous_count = gmRemixStockModeGetLastCount(port);
    s32 start_slot;

    if ((gmRemixStockModeIsLast(port) == FALSE) || (previous_count < 0))
    {
        return;
    }
    start_slot = gmRemixTagTeamRulesSkippedStocks(TRUE, gSCManagerBattleState->stocks, previous_count);

    if (start_slot >= GMREMIX_TAGTEAM_QUEUE_MAX)
    {
        start_slot = GMREMIX_TAGTEAM_QUEUE_MAX - 1;
    }
    if (start_slot == 0)
    {
        return;
    }
    sGMRemixTagTeamIsStartReplaced[port] = TRUE;
    data->fkind = sGMRemixTagTeamQueues[port][start_slot].fkind;
    data->costume = sGMRemixTagTeamQueues[port][start_slot].costume;
    data->shade = sGMRemixTagTeamQueues[port][start_slot].shade;
    ftRemixSonicSetClassic(port, sGMRemixTagTeamQueues[port][start_slot].is_classic);
}

void gmRemixTagTeamBuildQueues(void)
{
    s32 port;
    s32 slot;

    if (gmRemixTagTeamIsActive() == FALSE)
    {
        return;
    }
    gmRemixTagTeamResetState();

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        SCPlayerData *data = &gSCManagerBattleState->players[port];

        if (data->pkind == nFTPlayerKindNot)
        {
            continue;
        }
        sGMRemixTagTeamQueues[port][0].fkind = data->fkind;
        sGMRemixTagTeamQueues[port][0].costume = data->costume;
        sGMRemixTagTeamQueues[port][0].shade = data->shade;
        sGMRemixTagTeamQueues[port][0].is_classic = (data->fkind == REMIX_ID_SONIC) ? ftRemixSonicIsClassic(port) : FALSE;
        sGMRemixTagTeamCssPick[port] = sGMRemixTagTeamQueues[port][0];

        gmRemixTagTeamApplyCssFirstSlot(port);

        for (slot = 1; slot < GMREMIX_TAGTEAM_QUEUE_MAX; slot++)
        {
            gmRemixTagTeamFillMenuSlot(port, slot);
        }
        gmRemixTagTeamApplyStartSlot(port);
    }
}

/* The icons of the slots past the stocks set are never shown, so their characters are not read. */
static void gmRemixTagTeamResolveIcons(s32 port)
{
    s32 slot;

    for (slot = 0; (slot < GMREMIX_TAGTEAM_QUEUE_MAX) && (slot <= gSCManagerBattleState->stocks); slot++)
    {
        gmRemixTagTeamIconsSet(port, slot, sGMRemixTagTeamQueues[port][slot].fkind, sGMRemixTagTeamQueues[port][slot].costume, sGMRemixTagTeamQueues[port][slot].is_classic);
    }
}

static void gmRemixTagTeamRestoreCssPick(s32 port)
{
    SCPlayerData *data = &gSCManagerBattleState->players[port];

    if (sGMRemixTagTeamIsStartReplaced[port] == FALSE)
    {
        return;
    }
    data->fkind = sGMRemixTagTeamCssPick[port].fkind;
    data->costume = sGMRemixTagTeamCssPick[port].costume;
    data->shade = sGMRemixTagTeamCssPick[port].shade;
    sGMRemixTagTeamIsStartReplaced[port] = FALSE;
}

static void gmRemixTagTeamRecordFigatree(s32 port)
{
    FTStruct *fp = ftGetStruct(gSCManagerBattleState->players[port].fighter_gobj);
    FTData *ft_data = port_fighter_data(fp->fkind);

    sGMRemixTagTeamFigatrees[port].heap = fp->figatree_heap;
    sGMRemixTagTeamFigatrees[port].capacity = (ft_data != NULL) ? ft_data->file_anim_size : 0;
}

void gmRemixTagTeamSetup(void)
{
    s32 port;

    if (gmRemixTagTeamIsActive() == FALSE)
    {
        return;
    }
    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        if (gSCManagerBattleState->players[port].pkind == nFTPlayerKindNot)
        {
            continue;
        }
        gmRemixTagTeamRestoreCssPick(port);
        gmRemixTagTeamRecordFigatree(port);
        port_log("tagteam: port %d queue %d %d %d %d %d %d initial stocks %d\n", (int)port, (int)sGMRemixTagTeamQueues[port][0].fkind,
                 (int)sGMRemixTagTeamQueues[port][1].fkind, (int)sGMRemixTagTeamQueues[port][2].fkind,
                 (int)sGMRemixTagTeamQueues[port][3].fkind, (int)sGMRemixTagTeamQueues[port][4].fkind,
                 (int)sGMRemixTagTeamQueues[port][5].fkind, (int)gmRemixStockModeGetInitialCount(port));
        gmRemixTagTeamResolveIcons(port);
    }
}

/* The slot of the port's current stock (load_next_char_): the initial stock count minus the stocks left and the stolen ones, plus the Stock Mode Last skip. */
static s32 gmRemixTagTeamGetSlotOfPort(s32 port, s32 stocks_left)
{
    s32 initial_count = gmRemixStockModeGetInitialCount(port);
    s32 skipped_stocks;

    if (initial_count < 0)
    {
        initial_count = gSCManagerBattleState->stocks;
    }
    skipped_stocks = gmRemixTagTeamRulesSkippedStocks(gmRemixStockModeIsLast(port), gSCManagerBattleState->stocks, initial_count);

    return gmRemixTagTeamRulesQueueIndex(initial_count, stocks_left, sGMRemixTagTeamStolenCount[port], skipped_stocks,
                                         GMREMIX_TAGTEAM_QUEUE_MAX);
}

/* load_next_char_: a port that took a stock from a teammate plays the character of that stock. */
static GMRemixTagTeamSlot* gmRemixTagTeamGetQueueSlot(FTStruct *fp)
{
    GMRemixTagTeamStolen *stolen = &sGMRemixTagTeamStolen[fp->player];

    if (stolen->port >= 0)
    {
        return &sGMRemixTagTeamQueues[stolen->port][stolen->slot];
    }
    return &sGMRemixTagTeamQueues[fp->player][gmRemixTagTeamGetSlotOfPort(fp->player, fp->stock_count)];
}

sb32 gmRemixTagTeamApplyStockIcon(SObj *sobj, s32 port, s32 position, s32 icon_count)
{
    GMRemixTagTeamStolen *stolen = &sGMRemixTagTeamStolen[port];
    s32 base_slot;

    if (gmRemixTagTeamIsActive() == FALSE)
    {
        return FALSE;
    }
    if ((stolen->port >= 0) && (position == (icon_count - 1)))
    {
        return gmRemixTagTeamIconsApply(sobj, stolen->port, stolen->slot);
    }
    base_slot = gmRemixTagTeamGetSlotOfPort(port, gSCManagerBattleState->players[port].stock_count);

    return gmRemixTagTeamIconsApply(sobj, port, gmRemixTagTeamRulesIconSlot(base_slot, icon_count, position, GMREMIX_TAGTEAM_QUEUE_MAX));
}

sb32 gmRemixTagTeamStealStock(s32 thief, s32 stolen_port, SObj *sobj)
{
    s32 slot;

    if (gmRemixTagTeamIsActive() == FALSE)
    {
        return FALSE;
    }
    slot = gmRemixTagTeamRulesGetSlot(gSCManagerBattleState->stocks, sGMRemixTagTeamStolenCount[stolen_port],
                                      GMREMIX_TAGTEAM_QUEUE_MAX);
    sGMRemixTagTeamStolen[thief].port = stolen_port;
    sGMRemixTagTeamStolen[thief].slot = slot;
    sGMRemixTagTeamStolenCount[stolen_port]++;
    port_log("tagteam: port %d steals queue slot %d of port %d\n", (int)thief, (int)slot, (int)stolen_port);

    return gmRemixTagTeamIconsApply(sobj, stolen_port, slot);
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

/* Animation heap for the fighter a port swaps to: the port's heap while it fits (load_next_char_ passes fp->anim_heap on), else one of the biggest animation size (use_largest_anim_heap_). */
static void* gmRemixTagTeamGetFigatreeHeap(s32 port, s32 fkind)
{
    GMRemixTagTeamFigatree *figatree = &sGMRemixTagTeamFigatrees[port];
    FTData *ft_data = port_fighter_data(fkind);
    size_t needed_size = (ft_data != NULL) ? ft_data->file_anim_size : gFTManagerFigatreeHeapSize;

    if ((figatree->heap != NULL) && (figatree->capacity >= needed_size))
    {
        return figatree->heap;
    }
    figatree->heap = syTaskmanMalloc(gFTManagerFigatreeHeapSize, 0x10);
    figatree->capacity = gFTManagerFigatreeHeapSize;
    sGMRemixTagTeamHeapAllocCount++;

    return figatree->heap;
}

static void gmRemixTagTeamFillFighterDesc(FTDesc *desc, FTStruct *old_fp, GMRemixTagTeamSlot *slot)
{
    SCPlayerData *data = &gSCManagerBattleState->players[old_fp->player];

    *desc = dFTManagerDefaultFighterDesc;

    desc->fkind = slot->fkind;
    desc->lr = old_fp->lr;
    desc->team = old_fp->team;
    desc->player = old_fp->player;
    desc->detail = ((gSCManagerBattleState->pl_count + gSCManagerBattleState->cp_count) < 3) ? nFTPartsDetailHigh : nFTPartsDetailLow;
    desc->costume = slot->costume;
    desc->shade = slot->shade;
    desc->handicap = data->handicap;
    desc->level = data->level;
    desc->stock_count = old_fp->stock_count;
    desc->damage = 0;
    desc->is_skip_entry = TRUE;
    desc->is_skip_shadow_setup = TRUE;
    desc->pkind = data->pkind;
    desc->controller = &gSYControllerDevices[old_fp->player];
    desc->figatree_heap = gmRemixTagTeamGetFigatreeHeap(old_fp->player, slot->fkind);
}

GObj* gmRemixTagTeamLoadNextFighter(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GMRemixTagTeamSlot *slot;
    FTDesc desc;
    GObj *new_gobj;
    s32 port = fp->player;
    s32 old_fkind = fp->fkind;

    if (gmRemixTagTeamIsActive() == FALSE)
    {
        return fighter_gobj;
    }
    slot = gmRemixTagTeamGetQueueSlot(fp);

    if (gmRemixTagTeamRulesNeedsSwap(slot->fkind, slot->costume, slot->shade, slot->is_classic, fp->fkind, fp->costume, fp->shade,
                                     (fp->fkind == REMIX_ID_SONIC) ? ftRemixSonicIsClassic(port) : FALSE) == 0)
    {
        return fighter_gobj;
    }
    ftManagerSetupFilesAllKind(slot->fkind);
    gmRemixTagTeamFillFighterDesc(&desc, fp, slot);

    /* Destroy first, as the asm does: the new fighter then takes the freed fighter struct, parts and animation heap. */
    ftManagerDestroyFighter(fighter_gobj);
    ftRemixSonicSetClassic(port, slot->is_classic);
    new_gobj = ftManagerMakeFighter(&desc);

    gmRemixTagTeamMoveOwnership(fighter_gobj, new_gobj);
    gSCManagerBattleState->players[port].fighter_gobj = new_gobj;
    gSCManagerBattleState->players[port].stock_damage_all = 0;
    ftParamUnlockPlayerControl(new_gobj);
    ifRemixTagTeamRefreshHud(port, new_gobj);

    sGMRemixTagTeamSwapCount++;
    port_log("tagteam: port %d swap %d: fkind %d -> %d costume %d shade %d, anim heap %p cap %u, heap allocations %d\n",
             (int)port, (int)sGMRemixTagTeamSwapCount, (int)old_fkind, (int)slot->fkind, (int)slot->costume, (int)slot->shade,
             sGMRemixTagTeamFigatrees[port].heap, (unsigned)sGMRemixTagTeamFigatrees[port].capacity, (int)sGMRemixTagTeamHeapAllocCount);

    return new_gobj;
}

#endif
