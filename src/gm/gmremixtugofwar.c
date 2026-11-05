/**
 * gmremixtugofwar.c - state and credit rules of the Remix Tug of War mode (TugOfWar.asm setup_, update_last_hit_by_,
 * credit_stock_to_koer_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <port_log.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixstockmode.h"
#include "gm/gmremixtugofwar.h"
#include "gm/gmremixtugofwarrules.h"
#include "if/ifremixbattleplace.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <ft/ftcommon/ftcommonfunctions.h>

/* Port deviation: the asm guards the stock with `slti 0xFF` on a signed byte, which never limits, so 127 wraps to -128 there. */
#define GMREMIX_TUGOFWAR_STOCK_MAX 127

static s32 sGMRemixTugOfWarLastHit[GMCOMMON_PLAYERS_MAX];
static s32 sGMRemixTugOfWarTeam[GMCOMMON_PLAYERS_MAX];
static sb32 sGMRemixTugOfWarIs1v1 = FALSE;

static void gmRemixTugOfWarResetPorts(void)
{
    s32 i;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        sGMRemixTugOfWarLastHit[i] = GMREMIX_TUGOFWAR_NO_PORT;
        sGMRemixTugOfWarTeam[i] = GMREMIX_TUGOFWAR_NO_PORT;
    }
}

/* TugOfWar.asm setup_: every fighter starts without a shield breaker (vanilla leaves shield_player uninitialized) and its team is recorded per port. */
static void gmRemixTugOfWarInitFighters(void)
{
    GObj *fighter_gobj;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        fp->shield_player = GMREMIX_TUGOFWAR_NO_PORT;
        sGMRemixTugOfWarTeam[fp->player] = fp->team;
    }
}

/* Fills ports with the port of each fighter and returns the fighter count. The 1v1 test needs the count, not the order. */
static s32 gmRemixTugOfWarCollectFighterPorts(s32 ports[GMCOMMON_PLAYERS_MAX])
{
    GObj *fighter_gobj;
    s32 fighters_num = 0;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        if (fighters_num < GMCOMMON_PLAYERS_MAX)
        {
            ports[fighters_num] = ftGetStruct(fighter_gobj)->player;
        }
        fighters_num++;
    }
    return fighters_num;
}

static void gmRemixTugOfWarSetup1v1(s32 port_first, s32 port_second)
{
    sGMRemixTugOfWarLastHit[port_first] = port_second;
    sGMRemixTugOfWarLastHit[port_second] = port_first;
}

/* TRUE when the hooks record hits: Tug of War and not 1v1 (TugOfWar.asm checks is_1v1 in both hooks). */
static sb32 gmRemixTugOfWarRecordsHits(void)
{
    if (gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR) == FALSE)
    {
        return FALSE;
    }
    return (sGMRemixTugOfWarIs1v1 == FALSE) ? TRUE : FALSE;
}

static void gmRemixTugOfWarSetLastHit(s32 victim_port, s32 attacker_port)
{
    if (gmRemixTugOfWarRulesIsValidPort(victim_port) == FALSE)
    {
        return;
    }
    if (gmRemixTugOfWarRulesIsValidPort(attacker_port) == FALSE)
    {
        return;
    }
    sGMRemixTugOfWarLastHit[victim_port] = attacker_port;
}

void gmRemixTugOfWarSetup(void)
{
    s32 ports[GMCOMMON_PLAYERS_MAX];
    s32 fighters_num;

    if (gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR) == FALSE)
    {
        return;
    }
    gmRemixTugOfWarResetPorts();
    gmRemixTugOfWarInitFighters();
    fighters_num = gmRemixTugOfWarCollectFighterPorts(ports);
    sGMRemixTugOfWarIs1v1 = (fighters_num == 2) ? TRUE : FALSE;
    if (sGMRemixTugOfWarIs1v1 == FALSE)
    {
        return;
    }
    gmRemixTugOfWarSetup1v1(ports[0], ports[1]);
}

void gmRemixTugOfWarRecordDamage(FTStruct *victim, FTStruct *attacker)
{
    if (gmRemixTugOfWarRecordsHits() == FALSE)
    {
        return;
    }
    if (attacker->team == victim->team)
    {
        return;
    }
    gmRemixTugOfWarSetLastHit(victim->player, attacker->player);
}

void gmRemixTugOfWarRecordShieldBreak(FTStruct *victim)
{
    s32 attacker_port = victim->shield_player;

    if (gmRemixTugOfWarRecordsHits() == FALSE)
    {
        return;
    }
    if (gmRemixTugOfWarRulesIsValidPort(attacker_port) == FALSE)
    {
        return;
    }
    if (sGMRemixTugOfWarTeam[attacker_port] == victim->team)
    {
        return;
    }
    gmRemixTugOfWarSetLastHit(victim->player, attacker_port);
}

s32 gmRemixTugOfWarGetLastHit(s32 victim_port)
{
    if (gmRemixTugOfWarRulesIsValidPort(victim_port) == FALSE)
    {
        return GMREMIX_TUGOFWAR_NO_PORT;
    }
    return sGMRemixTugOfWarLastHit[victim_port];
}

/* TugOfWar.asm update_sIFCommonBattlePlace_: a revived fighter undoes the place step of its elimination, unless a teammate
 * still had stocks (in teams the team had not been eliminated). */
static void gmRemixTugOfWarFixPlace(FTStruct *revived)
{
    s32 i;

    if (gSCManagerBattleState->is_team_battle != FALSE)
    {
        for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
        {
            SCPlayerData *player_data = &gSCManagerBattleState->players[i];

            if ((i == revived->player) || (player_data->pkind == nFTPlayerKindNot))
            {
                continue;
            }
            if ((player_data->team == revived->team) && (player_data->stock_count >= 0))
            {
                return;
            }
        }
    }
    ifRemixBattlePlaceRestoreRevived();
}

static void gmRemixTugOfWarGiveStock(s32 port)
{
    SCPlayerData *player_data = &gSCManagerBattleState->players[port];
    FTStruct *fp;
    s32 stock_before;

    if (player_data->fighter_gobj == NULL)
    {
        return;
    }
    fp = ftGetStruct(player_data->fighter_gobj);
    stock_before = fp->stock_count;

    if (stock_before < GMREMIX_TUGOFWAR_STOCK_MAX)
    {
        fp->stock_count++;
    }
    player_data->stock_count = fp->stock_count;
    gmRemixStockModeSetLastCount(port, player_data->stock_count);
    port_log("tugofwar: stock credit to port %d, stock_count %d -> %d\n", (int)port, (int)stock_before, (int)fp->stock_count);

    if (gmRemixTugOfWarShouldRevive(stock_before) == FALSE)
    {
        return;
    }
    ftCommonRebirthDownSetStatus(player_data->fighter_gobj);
    gmRemixTugOfWarFixPlace(fp);
}

void gmRemixTugOfWarCreditKO(FTStruct *victim)
{
    s32 koer_port = GMREMIX_TUGOFWAR_NO_PORT;
    s32 koer_team = GMREMIX_TUGOFWAR_NO_PORT;
    s32 credit_port;

    if (gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR) == FALSE)
    {
        return;
    }
    victim->shield_player = GMREMIX_TUGOFWAR_NO_PORT;

    if (gmRemixTugOfWarRulesIsValidPort(victim->damage_player) != FALSE)
    {
        koer_port = victim->damage_player;
        koer_team = sGMRemixTugOfWarTeam[koer_port];
    }
    credit_port = gmRemixTugOfWarPickCreditPort(victim->team, koer_team, koer_port, gmRemixTugOfWarGetLastHit(victim->player));

    if (sGMRemixTugOfWarIs1v1 == FALSE)
    {
        sGMRemixTugOfWarLastHit[victim->player] = GMREMIX_TUGOFWAR_NO_PORT;
    }
    if ((gmRemixTugOfWarRulesIsValidPort(credit_port) != FALSE) && (credit_port != victim->player))
    {
        gmRemixTugOfWarGiveStock(credit_port);
    }
}

#endif
