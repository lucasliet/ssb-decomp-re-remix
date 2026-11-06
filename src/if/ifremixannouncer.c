/**
 * ifremixannouncer.c - Smash Remix Pokemon Stadium announcer (src/PokemonAnnouncer.asm): call tables, timers, the four pick
 * modes (NAME, KO, GENERAL, STANDARD) and the per-frame countdown. Phase 2 hooks call only ifremixannouncer.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixannouncer.h"
#include "if/ifremixannouncerrules.h"
#include "gm/gmremixtoggle.h"
#include <gr/grremix/grremixids.h>
#include <sc/scene.h>
#include <sc/scmanager.h>
#include <sys/audio.h>
#include <sys/objdef.h>
#include <sys/objman.h>
#include <sys/objtypes.h>
#include <sys/utils.h>

#define IFREMIX_ANNOUNCER_MODE_ALL 1
#define IFREMIX_ANNOUNCER_MODE_OFF 2
#define IFREMIX_ANNOUNCER_STOCK_RULE 0x2
#define IFREMIX_ANNOUNCER_CLOSE_MATCH_STOCKS 2
#define IFREMIX_ANNOUNCER_GLOBAL_TIMER 0x78
#define IFREMIX_ANNOUNCER_QUICK_KO_TIME 0x4B0
#define IFREMIX_ANNOUNCER_LONG_KO_PERCENT 150
#define IFREMIX_ANNOUNCER_OPENING_TIME 0x1E
#define IFREMIX_ANNOUNCER_TIME_UP_REMAIN 0xE09
#define IFREMIX_ANNOUNCER_TIME_UP_FGM 0x4A1
#define IFREMIX_ANNOUNCER_GENERAL_MIN_TIME 0x960
#define IFREMIX_ANNOUNCER_GENERAL_ROLL_RANGE 0x960
#define IFREMIX_ANNOUNCER_GENERAL_ROLL_HIT 0x50
#define IFREMIX_ANNOUNCER_GENERAL_MAX_TIME 0x12C0
#define IFREMIX_ANNOUNCER_NAME_ITEM_BASE 0x20
#define IFREMIX_ANNOUNCER_THERE_GOES_FGM 0x45B
#define IFREMIX_ANNOUNCER_GAME_SET_DELAY 0x80

static u32 sIFRemixAnnouncerGlobalTimer;
static u32 sIFRemixAnnouncerGeneralTimer;
static sb32 sIFRemixAnnouncerSleepRequested;

/** TRUE for the three stages where the Pokemon Stadium announcer speaks in the Pokemon-stages mode. */
static sb32 ifRemixAnnouncerIsPokemonStage(s32 gkind)
{
    return (gkind == nGRRemixStage_POKEMON_STADIUM) || (gkind == nGRRemixStage_GYM_LEADER_CASTLE) || (gkind == nGRRemixStage_POKEMON_STADIUM_2);
}

/** TRUE on the Gym Leader Castle, which has its own opening call. */
static sb32 ifRemixAnnouncerIsGymStage(void)
{
    return gSCManagerBattleState->gkind == nGRRemixStage_GYM_LEADER_CASTLE;
}

sb32 ifRemixAnnouncerIsActive(void)
{
    s32 mode = gmRemixToggle(announcer_mode);

    if (mode == IFREMIX_ANNOUNCER_MODE_OFF)
    {
        return FALSE;
    }
    if (mode == IFREMIX_ANNOUNCER_MODE_ALL)
    {
        return TRUE;
    }
    return ifRemixAnnouncerIsPokemonStage(gSCManagerBattleState->gkind);
}

/** TRUE when the stock rule is on and every port is down to its last stock, the asm close match condition. */
static sb32 ifRemixAnnouncerIsCloseMatch(void)
{
    s32 port;

    if (gSCManagerSceneData.scene_curr != nSCKindVSBattle)
    {
        return FALSE;
    }
    if ((gSCManagerBattleState->game_rules & IFREMIX_ANNOUNCER_STOCK_RULE) == 0)
    {
        return FALSE;
    }
    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        if (gSCManagerBattleState->players[port].stock_count >= IFREMIX_ANNOUNCER_CLOSE_MATCH_STOCKS)
        {
            return FALSE;
        }
    }
    return TRUE;
}

/** Random draw of the announcer, the asm Global.get_random_int_. */
static int ifRemixAnnouncerDraw(int range)
{
    return syUtilsRandIntRange(range);
}

/** Returns a random entry of the table, the asm Global.get_random_int_ over the entries the call draws from. */
static IFRemixAnnouncement *ifRemixAnnouncerPickRandom(IFRemixAnnouncerTable *table)
{
    return &table->entries[ifRemixAnnouncerDraw(table->pick_count)];
}

/** Random entries until one has no timer left, one attempt per entry. Returns NULL when every attempt is still cooling down. */
static IFRemixAnnouncement *ifRemixAnnouncerPickFree(IFRemixAnnouncerTable *table)
{
    s32 attempt;

    for (attempt = 0; attempt < table->pick_count; attempt++)
    {
        IFRemixAnnouncement *entry = ifRemixAnnouncerPickRandom(table);

        if (entry->timer == 0)
        {
            return entry;
        }
    }
    return NULL;
}

/** NAME pick: the Pokemon clip of the item kind, silent while its timer runs or when the item is not a Pokemon. */
static IFRemixAnnouncement *ifRemixAnnouncerPickName(s32 item_kind)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallPokemon);
    s32 index = item_kind - IFREMIX_ANNOUNCER_NAME_ITEM_BASE;

    if ((index < 0) || (index >= table->count))
    {
        return NULL;
    }
    if (table->entries[index].timer != 0)
    {
        return NULL;
    }
    return &table->entries[index];
}

/**
 * Regular KO pick. The asm retries a cooling entry, but its retry branch falls through to the next draw, so the last
 * of its four draws is always used. A single uniform draw keeps that behaviour.
 */
static IFRemixAnnouncement *ifRemixAnnouncerPickRegularKO(void)
{
    return ifRemixAnnouncerPickRandom(ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallRegularKO));
}

/** Quick KO pick, falls back to a regular KO when the quick clip was already used in the match. */
static IFRemixAnnouncement *ifRemixAnnouncerPickQuickKO(void)
{
    IFRemixAnnouncement *entry = ifRemixAnnouncerPickRandom(ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallQuickKO));

    if (entry->used != 0)
    {
        return ifRemixAnnouncerPickRegularKO();
    }
    return entry;
}

/** Long KO pick, falls back to a regular KO while its timer runs. */
static IFRemixAnnouncement *ifRemixAnnouncerPickLongKO(void)
{
    IFRemixAnnouncement *entry = ifRemixAnnouncerPickRandom(ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallLongKO));

    if (entry->timer != 0)
    {
        return ifRemixAnnouncerPickRegularKO();
    }
    return entry;
}

/** KO pick: quick under 20 seconds of match time, long above 150 percent damage, regular otherwise. */
static IFRemixAnnouncement *ifRemixAnnouncerPickKO(FTStruct *victim_fp)
{
    if (gSCManagerBattleState->time_passed < IFREMIX_ANNOUNCER_QUICK_KO_TIME)
    {
        return ifRemixAnnouncerPickQuickKO();
    }
    if (victim_fp->percent_damage > IFREMIX_ANNOUNCER_LONG_KO_PERCENT)
    {
        return ifRemixAnnouncerPickLongKO();
    }
    return ifRemixAnnouncerPickRegularKO();
}

/** GENERAL pick: the close match clip once per match in a close stock game, else a free general clip. */
static IFRemixAnnouncement *ifRemixAnnouncerPickGeneral(void)
{
    IFRemixAnnouncement *close_entry = &ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallCloseMatch)->entries[0];

    if (ifRemixAnnouncerIsCloseMatch() && (close_entry->used == 0))
    {
        return close_entry;
    }
    return ifRemixAnnouncerPickFree(ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallGeneral));
}

/** Starts the timers of a played clip and marks it used, the asm success path of announcer_main_. */
static sb32 ifRemixAnnouncerPlayEntry(IFRemixAnnouncement *entry)
{
    if (entry == NULL)
    {
        return FALSE;
    }
    entry->timer = entry->duration;
    entry->used = TRUE;
    sIFRemixAnnouncerGlobalTimer = IFREMIX_ANNOUNCER_GLOBAL_TIMER;
    syAudioPlayFGM(entry->fgm_id);
    return TRUE;
}

/** TRUE when no clip played in the last few frames, so the announcer does not talk over itself. */
static sb32 ifRemixAnnouncerIsGlobalReady(void)
{
    return sIFRemixAnnouncerGlobalTimer == 0;
}

static sb32 ifRemixAnnouncerPlayGeneral(void)
{
    if (!ifRemixAnnouncerIsGlobalReady())
    {
        return FALSE;
    }
    return ifRemixAnnouncerPlayEntry(ifRemixAnnouncerPickGeneral());
}

sb32 ifRemixAnnouncerPlayCall(IFRemixAnnouncerCall call)
{
    IFRemixAnnouncerTable *table;

    if (!ifRemixAnnouncerIsGlobalReady())
    {
        return FALSE;
    }
    table = ifRemixAnnouncerTablesGet(call);
    if (ifRemixAnnouncerRulesIsSpent(table))
    {
        return FALSE;
    }
    return ifRemixAnnouncerPlayEntry(ifRemixAnnouncerRulesPickStandard(table, ifRemixAnnouncerDraw));
}

sb32 ifRemixAnnouncerPlayName(s32 item_kind)
{
    if (!ifRemixAnnouncerIsGlobalReady())
    {
        return FALSE;
    }
    return ifRemixAnnouncerPlayEntry(ifRemixAnnouncerPickName(item_kind));
}

sb32 ifRemixAnnouncerPlayKO(FTStruct *victim_fp)
{
    if (!ifRemixAnnouncerIsGlobalReady())
    {
        return FALSE;
    }
    return ifRemixAnnouncerPlayEntry(ifRemixAnnouncerPickKO(victim_fp));
}

void ifRemixAnnouncerRequestSleep(void)
{
    sIFRemixAnnouncerSleepRequested = TRUE;
}

static void ifRemixAnnouncerCountGlobalTimer(void)
{
    if (sIFRemixAnnouncerGlobalTimer != 0)
    {
        sIFRemixAnnouncerGlobalTimer--;
    }
}

/** TRUE when the general timer has reached the chance window: a 1 in 2400 roll or the forced time. */
static sb32 ifRemixAnnouncerIsGeneralDue(u32 timer)
{
    s32 roll = syUtilsRandIntRange(IFREMIX_ANNOUNCER_GENERAL_ROLL_RANGE);

    return (roll == IFREMIX_ANNOUNCER_GENERAL_ROLL_HIT) || (timer == IFREMIX_ANNOUNCER_GENERAL_MAX_TIME);
}

/** The general timer counts every frame, and a general clip is tried only after the minimum time. */
static void ifRemixAnnouncerRunGeneral(void)
{
    u32 timer = sIFRemixAnnouncerGeneralTimer;

    sIFRemixAnnouncerGeneralTimer++;
    if (timer < IFREMIX_ANNOUNCER_GENERAL_MIN_TIME)
    {
        return;
    }
    if (!ifRemixAnnouncerIsGeneralDue(timer))
    {
        return;
    }
    if (ifRemixAnnouncerPlayGeneral())
    {
        sIFRemixAnnouncerGeneralTimer = 0;
    }
}

/** The opening clip plays once, 30 frames into the match. The Gym Leader Castle has its own clip. */
static void ifRemixAnnouncerRunOpening(void)
{
    if (ifRemixAnnouncerIsGymStage())
    {
        ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallGymLeader);
        return;
    }
    ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallOpening);
}

/** A pending sleep clip replaces the rest of the frame, as in the asm; it stays pending until it plays. */
static void ifRemixAnnouncerRunSleep(void)
{
    if (ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallSleep))
    {
        sIFRemixAnnouncerSleepRequested = FALSE;
    }
}

/** Per-frame process of the announcer (asm announcer_constant_). Registered by ifRemixAnnouncerSetup. */
static void ifRemixAnnouncerRun(GObj *run_gobj)
{
    u32 global_timer_before = sIFRemixAnnouncerGlobalTimer;

    ifRemixAnnouncerCountGlobalTimer();
    if (ifRemixAnnouncerRulesChecksSleep(global_timer_before, sIFRemixAnnouncerSleepRequested))
    {
        ifRemixAnnouncerRunSleep();
        return;
    }
    ifRemixAnnouncerTablesCountDown();
    if (gSCManagerBattleState->time_remain == IFREMIX_ANNOUNCER_TIME_UP_REMAIN)
    {
        syAudioPlayFGM(IFREMIX_ANNOUNCER_TIME_UP_FGM);
        return;
    }
    if (gSCManagerBattleState->time_passed == IFREMIX_ANNOUNCER_OPENING_TIME)
    {
        ifRemixAnnouncerRunOpening();
        return;
    }
    ifRemixAnnouncerRunGeneral();
}

void ifRemixAnnouncerSetup(void)
{
    GObj *announcer_gobj;

    if (!ifRemixAnnouncerIsActive())
    {
        return;
    }
    sIFRemixAnnouncerGlobalTimer = 0;
    sIFRemixAnnouncerGeneralTimer = 0;
    sIFRemixAnnouncerSleepRequested = FALSE;
    ifRemixAnnouncerTablesReset();
    announcer_gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(announcer_gobj, ifRemixAnnouncerRun, nGCProcessKindFunc, 0);
}

/** The vanilla game set clip is replaced only while the announcer is active (asm there_goes_). */
u16 ifRemixAnnouncerGameSetFgm(u16 vanilla_fgm)
{
    if (!ifRemixAnnouncerIsActive())
    {
        return vanilla_fgm;
    }
    return IFREMIX_ANNOUNCER_THERE_GOES_FGM;
}

#define IFREMIX_ANNOUNCER_VANILLA_TIME_UP_FGM 0x020F
#define IFREMIX_ANNOUNCER_TIME_UP_FGM 0x04AE

/** The vanilla TIME UP clip is replaced only while the announcer is active (asm time_up_announcement_). */
u16 ifRemixAnnouncerTimeUpFgm(u16 vanilla_fgm)
{
    if ((vanilla_fgm != IFREMIX_ANNOUNCER_VANILLA_TIME_UP_FGM) || !ifRemixAnnouncerIsActive())
    {
        return vanilla_fgm;
    }
    return IFREMIX_ANNOUNCER_TIME_UP_FGM;
}

/** The longer delay only applies while the announcer is active, so the clip finishes before the transition (asm delay_). */
s32 ifRemixAnnouncerGameSetDelay(s32 vanilla_delay)
{
    if (!ifRemixAnnouncerIsActive())
    {
        return vanilla_delay;
    }
    return IFREMIX_ANNOUNCER_GAME_SET_DELAY;
}

#endif
