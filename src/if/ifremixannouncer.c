/**
 * ifremixannouncer.c - Smash Remix Pokemon Stadium announcer (src/PokemonAnnouncer.asm): call tables, timers, the four pick
 * modes (NAME, KO, GENERAL, STANDARD) and the per-frame countdown. Phase 2 hooks call only ifremixannouncer.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixannouncer.h"
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

/* Field order and size match the asm add_announcement macro (16 bytes per entry). */
typedef struct IFRemixAnnouncement
{
    u16 id;
    u16 fgm_id;
    u32 duration;
    u32 timer;
    u16 used;
    u16 flag;
} IFRemixAnnouncement;

typedef struct IFRemixAnnouncerTable
{
    IFRemixAnnouncement *entries;
    s32 count;
} IFRemixAnnouncerTable;

#define IFREMIX_ANNOUNCEMENT(announcer_id, fgm, time, special) { announcer_id, fgm, time, 0, 0, special }
#define IFREMIX_ANNOUNCER_TABLE(calls) { calls, ARRAY_COUNT(calls) }

static IFRemixAnnouncement sPokemonCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x0, 0x45C, 0x384, 0x0),   /* ONIX */
    IFREMIX_ANNOUNCEMENT(0x1, 0x45D, 0x384, 0x0),   /* SNORLAX */
    IFREMIX_ANNOUNCEMENT(0x2, 0x45E, 0x384, 0x0),   /* GOLDEEN */
    IFREMIX_ANNOUNCEMENT(0x3, 0x45F, 0x384, 0x0),   /* MEOWTH */
    IFREMIX_ANNOUNCEMENT(0x4, 0x460, 0x384, 0x0),   /* CHARIZARD */
    IFREMIX_ANNOUNCEMENT(0x5, 0x461, 0x384, 0x0),   /* BEEDRILL */
    IFREMIX_ANNOUNCEMENT(0x6, 0x462, 0x384, 0x0),   /* BLASTOISE */
    IFREMIX_ANNOUNCEMENT(0x7, 0x463, 0x384, 0x0),   /* CHANSEY */
    IFREMIX_ANNOUNCEMENT(0x8, 0x464, 0x384, 0x0),   /* STARMIE */
    IFREMIX_ANNOUNCEMENT(0x9, 0x465, 0x384, 0x0),   /* HITMONLEE */
    IFREMIX_ANNOUNCEMENT(0xA, 0x466, 0x384, 0x0),   /* KOFFING */
    IFREMIX_ANNOUNCEMENT(0xB, 0x467, 0x384, 0x0),   /* CLEFAIRY */
    IFREMIX_ANNOUNCEMENT(0xC, 0x468, 0x384, 0x0),   /* MEW */
};

static IFRemixAnnouncement sQuickKOCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0xD, 0x46D, 0x000, 0x0),   /* Taken down on the word go */
    IFREMIX_ANNOUNCEMENT(0xE, 0x46E, 0x000, 0x0),   /* That was quick, down already */
};

static IFRemixAnnouncement sRegularKOCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0xF, 0x469, 0x4B0, 0x0),   /* Oh, Is it down and out? */
    IFREMIX_ANNOUNCEMENT(0x10, 0x46A, 0x4B0, 0x0),  /* Waa, Going Down */
    IFREMIX_ANNOUNCEMENT(0x11, 0x46B, 0x4B0, 0x0),  /* Oh, It's down */
    IFREMIX_ANNOUNCEMENT(0x12, 0x46C, 0x4B0, 0x0),  /* And it's down */
};

static IFRemixAnnouncement sLongKOCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x13, 0x476, 0x258, 0x0),  /* It's finally taken down */
    IFREMIX_ANNOUNCEMENT(0x14, 0x477, 0x258, 0x0),  /* It goes down after a good fight */
};

static IFRemixAnnouncement sGeneralCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x15, 0x499, 0x3840, 0x0), /* What a furious battle */
    IFREMIX_ANNOUNCEMENT(0x16, 0x49A, 0x3840, 0x0), /* Neither one is conceding an inch */
    IFREMIX_ANNOUNCEMENT(0x17, 0x49B, 0x3840, 0x0), /* The Intense battle continues */
    IFREMIX_ANNOUNCEMENT(0x18, 0x49C, 0x3840, 0x0), /* The tense battle continues */
    IFREMIX_ANNOUNCEMENT(0x19, 0x49D, 0x3840, 0x0), /* This battle is still up in the air */
    IFREMIX_ANNOUNCEMENT(0x1A, 0x49E, 0x3840, 0x0), /* This battle is raging back and forth */
    IFREMIX_ANNOUNCEMENT(0x1A, 0x4BE, 0x3840, 0x0), /* This battle is raging back and forth */
    IFREMIX_ANNOUNCEMENT(0x1A, 0x4BF, 0x3840, 0x0), /* This battle is raging back and forth */
};

static IFRemixAnnouncement sCloseMatchCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x1B, 0x4A0, 0x000, 0x0),  /* The battle is coming right down to the wire */
};

static IFRemixAnnouncement sSleepCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x1C, 0x47C, 0x708, 0x0),  /* Oh no, it fell asleep */
    IFREMIX_ANNOUNCEMENT(0x1D, 0x47D, 0x708, 0x0),  /* It's asleep on the job */
    IFREMIX_ANNOUNCEMENT(0x1F, 0x47F, 0x708, 0x0),  /* Falling alseep is going to make this a one sided fight */
};

static IFRemixAnnouncement sComboCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x20, 0x480, 0x384, 0x1),  /* The attack is still continuing */
};

static IFRemixAnnouncement sKnockbackHighCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x21, 0x482, 0x384, 0x0),  /* Ooh, that's strong */
    IFREMIX_ANNOUNCEMENT(0x22, 0x483, 0x384, 0x0),  /* Powerful Strike */
    IFREMIX_ANNOUNCEMENT(0x23, 0x484, 0x384, 0x0),  /* Pow, that's an effective hit */
    IFREMIX_ANNOUNCEMENT(0x24, 0x485, 0x384, 0x0),  /* Harsh Blow */
    IFREMIX_ANNOUNCEMENT(0x25, 0x486, 0x384, 0x0),  /* That was brutal */
    IFREMIX_ANNOUNCEMENT(0x26, 0x487, 0x384, 0x0),  /* Super Effective */
    IFREMIX_ANNOUNCEMENT(0x27, 0x488, 0x384, 0x0),  /* Severe Hit */
    IFREMIX_ANNOUNCEMENT(0x28, 0x489, 0x384, 0x0),  /* Savage Hit */
    IFREMIX_ANNOUNCEMENT(0x29, 0x48A, 0x384, 0x0),  /* Woaaaahhh */
    IFREMIX_ANNOUNCEMENT(0x2A, 0x48B, 0x384, 0x0),  /* Woooah that was overpowering */
    IFREMIX_ANNOUNCEMENT(0x2B, 0x48C, 0x384, 0x0),  /* Devastating */
    IFREMIX_ANNOUNCEMENT(0x2C, 0x48D, 0x384, 0x0),  /* Ground zero */
};

static IFRemixAnnouncement sKnockbackMidCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x2D, 0x48E, 0x384, 0x0),  /* Good Hit */
    IFREMIX_ANNOUNCEMENT(0x2E, 0x48F, 0x384, 0x0),  /* Boom */
    IFREMIX_ANNOUNCEMENT(0x2F, 0x490, 0x384, 0x0),  /* That's a good hit */
    IFREMIX_ANNOUNCEMENT(0x30, 0x491, 0x384, 0x0),  /* Have that */
    IFREMIX_ANNOUNCEMENT(0x31, 0x492, 0x384, 0x0),  /* Aaaah */
    IFREMIX_ANNOUNCEMENT(0x32, 0x493, 0x384, 0x0),  /* Wow */
};

static IFRemixAnnouncement sDamageCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x33, 0x495, 0x384, 0x0),  /* Kapow Major Damage */
    IFREMIX_ANNOUNCEMENT(0x34, 0x496, 0x384, 0x0),  /* Heavy Damage */
    IFREMIX_ANNOUNCEMENT(0x35, 0x497, 0x384, 0x0),  /* Major Damage */
    IFREMIX_ANNOUNCEMENT(0x36, 0x498, 0x384, 0x0),  /* That one hurt */
};

static IFRemixAnnouncement sThunderCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x37, 0x4A2, 0x708, 0x1),  /* Here it comes, Thunder */
};

static IFRemixAnnouncement sQuickCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x38, 0x4AB, 0x708, 0x1),  /* There, quick attack */
};

static IFRemixAnnouncement sChargeCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x39, 0x4A6, 0x708, 0x1),  /* It's building energy for the next attack */
};

static IFRemixAnnouncement sDisableCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x3A, 0x4A4, 0x708, 0x1),  /* That's Disable */
};

static IFRemixAnnouncement sTeleportCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x3B, 0x4A3, 0x708, 0x1),  /* It used teleport */
};

static IFRemixAnnouncement sFirePunchCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x3C, 0x4A5, 0x708, 0x1),  /* Oh, fire punch! */
};

static IFRemixAnnouncement sRestCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x3D, 0x4AC, 0x708, 0x1),  /* It's catching some rest */
    IFREMIX_ANNOUNCEMENT(0x3E, 0x4AD, 0x708, 0x1),  /* It's taking a quick rest */
};

static IFRemixAnnouncement sPoundCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x3F, 0x4AA, 0x708, 0x1),  /* What a pound */
};

static IFRemixAnnouncement sToughCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x40, 0x4A7, 0x708, 0x1),  /* It hung tough */
};

static IFRemixAnnouncement sCounterCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x41, 0x4A8, 0x4B0, 0x1),  /* Awesome Counter */
};

static IFRemixAnnouncement sLittleDamageCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x42, 0x4A9, 0x708, 0x1),  /* There's a little damage */
};

static IFRemixAnnouncement sBurnCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x43, 0x478, 0x1C20, 0x1), /* It Suffered a Burn */
    IFREMIX_ANNOUNCEMENT(0x44, 0x49F, 0x1C20, 0x1), /* Its got a Nasty Burn, can it tough it out? */
};

static IFRemixAnnouncement sSeismicTossCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x45, 0x4AF, 0x708, 0x1),  /* Seismic Toss */
};

static IFRemixAnnouncement sRecoveryCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x46, 0x4B1, 0x708, 0x1),  /* It's restoring HP */
};

static IFRemixAnnouncement sClangCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x47, 0x4B2, 0x708, 0x1),  /* It's a Head on Collision, power versus power */
};

static IFRemixAnnouncement sStunCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x48, 0x4B3, 0x708, 0x1),  /* Being unable to move hurts */
    IFREMIX_ANNOUNCEMENT(0x49, 0x4B4, 0x708, 0x1),  /* It's paralyzed on the spot */
};

static IFRemixAnnouncement sTeamKOCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x4A, 0x4B9, 0x8708, 0x1), /* Will it erase the humiliation of its fallen comrade? */
    IFREMIX_ANNOUNCEMENT(0x4B, 0x4BA, 0x8708, 0x1), /* What will the otherside do */
};

static IFRemixAnnouncement sSlashCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x4C, 0x4B8, 0x708, 0x1),  /* What a slash */
};

static IFRemixAnnouncement sFlamethrowerCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x4D, 0x4B5, 0x708, 0x1),  /* Flamethrower! */
};

static IFRemixAnnouncement sGymLeaderCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x4E, 0x4B6, 0x8708, 0x1), /* We bring you this battle live from gym leader castle */
};

static IFRemixAnnouncement sOpeningCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x4F, 0x4B7, 0x8708, 0x1), /* Let's get started */
    IFREMIX_ANNOUNCEMENT(0x4F, 0x4C0, 0x8708, 0x1), /* We're ready to roll */
    IFREMIX_ANNOUNCEMENT(0x4F, 0x4C1, 0x8708, 0x1), /* What kind of battle can we expect to see */
};

static IFRemixAnnouncement sSingCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x1E, 0x47E, 0x708, 0x0),  /* It's singing */
};

static IFRemixAnnouncement sAttackCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x33, 0x4BB, 0x384, 0x0),  /* Kapow Major Damage */
    IFREMIX_ANNOUNCEMENT(0x34, 0x4BC, 0x384, 0x0),  /* Heavy Damage */
    IFREMIX_ANNOUNCEMENT(0x35, 0x4BD, 0x384, 0x0),  /* Major Damage */
};

static IFRemixAnnouncement sDamageEarlyCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x4F, 0x494, 0x8708, 0x1), /* Major Blow from the Word Go */
};

static IFRemixAnnouncement sBurrowCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x50, 0x5D1, 0x708, 0x1),  /* It Burrowed Underground */
};

static IFRemixAnnouncement sBurrowAgainCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x51, 0x5D3, 0x708, 0x1),  /* The Underground attack continues */
    IFREMIX_ANNOUNCEMENT(0x51, 0x5D2, 0x708, 0x1),  /* It Burrowed Underground Again */
};

static IFRemixAnnouncement sDigCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x51, 0x5D0, 0x708, 0x1),  /* Dig This */
};

static IFRemixAnnouncement sBodySlamCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x51, 0x5CF, 0x708, 0x1),  /* Body Slam */
};

static IFRemixAnnouncement sTauntCall[] =
{
    IFREMIX_ANNOUNCEMENT(0x52, 0x608, 0x708, 0x1),  /* It's Getting Pumped Up */
};

/* Order follows IFRemixAnnouncerCall; the table count is the asm pick count of the call. */
static IFRemixAnnouncerTable sIFRemixAnnouncerTables[nIFRemixAnnouncerCallCount] =
{
    [nIFRemixAnnouncerCallPokemon] = IFREMIX_ANNOUNCER_TABLE(sPokemonCalls),
    [nIFRemixAnnouncerCallQuickKO] = IFREMIX_ANNOUNCER_TABLE(sQuickKOCalls),
    [nIFRemixAnnouncerCallRegularKO] = IFREMIX_ANNOUNCER_TABLE(sRegularKOCalls),
    [nIFRemixAnnouncerCallLongKO] = IFREMIX_ANNOUNCER_TABLE(sLongKOCalls),
    [nIFRemixAnnouncerCallGeneral] = IFREMIX_ANNOUNCER_TABLE(sGeneralCalls),
    [nIFRemixAnnouncerCallCloseMatch] = IFREMIX_ANNOUNCER_TABLE(sCloseMatchCall),
    [nIFRemixAnnouncerCallSleep] = IFREMIX_ANNOUNCER_TABLE(sSleepCalls),
    [nIFRemixAnnouncerCallCombo] = IFREMIX_ANNOUNCER_TABLE(sComboCalls),
    [nIFRemixAnnouncerCallKnockbackHigh] = IFREMIX_ANNOUNCER_TABLE(sKnockbackHighCalls),
    [nIFRemixAnnouncerCallKnockbackMid] = IFREMIX_ANNOUNCER_TABLE(sKnockbackMidCalls),
    [nIFRemixAnnouncerCallDamage] = IFREMIX_ANNOUNCER_TABLE(sDamageCalls),
    [nIFRemixAnnouncerCallThunder] = IFREMIX_ANNOUNCER_TABLE(sThunderCall),
    [nIFRemixAnnouncerCallQuick] = IFREMIX_ANNOUNCER_TABLE(sQuickCall),
    [nIFRemixAnnouncerCallCharge] = IFREMIX_ANNOUNCER_TABLE(sChargeCall),
    [nIFRemixAnnouncerCallDisable] = IFREMIX_ANNOUNCER_TABLE(sDisableCall),
    [nIFRemixAnnouncerCallTeleport] = IFREMIX_ANNOUNCER_TABLE(sTeleportCall),
    [nIFRemixAnnouncerCallFirePunch] = IFREMIX_ANNOUNCER_TABLE(sFirePunchCall),
    [nIFRemixAnnouncerCallRest] = IFREMIX_ANNOUNCER_TABLE(sRestCalls),
    [nIFRemixAnnouncerCallPound] = IFREMIX_ANNOUNCER_TABLE(sPoundCall),
    [nIFRemixAnnouncerCallTough] = IFREMIX_ANNOUNCER_TABLE(sToughCall),
    [nIFRemixAnnouncerCallCounter] = IFREMIX_ANNOUNCER_TABLE(sCounterCall),
    [nIFRemixAnnouncerCallLittleDamage] = IFREMIX_ANNOUNCER_TABLE(sLittleDamageCall),
    [nIFRemixAnnouncerCallBurn] = IFREMIX_ANNOUNCER_TABLE(sBurnCalls),
    [nIFRemixAnnouncerCallSeismicToss] = IFREMIX_ANNOUNCER_TABLE(sSeismicTossCall),
    [nIFRemixAnnouncerCallRecovery] = IFREMIX_ANNOUNCER_TABLE(sRecoveryCall),
    [nIFRemixAnnouncerCallClang] = IFREMIX_ANNOUNCER_TABLE(sClangCall),
    [nIFRemixAnnouncerCallStun] = IFREMIX_ANNOUNCER_TABLE(sStunCalls),
    [nIFRemixAnnouncerCallTeamKO] = IFREMIX_ANNOUNCER_TABLE(sTeamKOCalls),
    [nIFRemixAnnouncerCallSlash] = IFREMIX_ANNOUNCER_TABLE(sSlashCall),
    [nIFRemixAnnouncerCallFlamethrower] = IFREMIX_ANNOUNCER_TABLE(sFlamethrowerCall),
    [nIFRemixAnnouncerCallGymLeader] = IFREMIX_ANNOUNCER_TABLE(sGymLeaderCall),
    [nIFRemixAnnouncerCallOpening] = IFREMIX_ANNOUNCER_TABLE(sOpeningCalls),
    [nIFRemixAnnouncerCallSing] = IFREMIX_ANNOUNCER_TABLE(sSingCall),
    [nIFRemixAnnouncerCallAttack] = IFREMIX_ANNOUNCER_TABLE(sAttackCalls),
    [nIFRemixAnnouncerCallDamageEarly] = IFREMIX_ANNOUNCER_TABLE(sDamageEarlyCall),
    [nIFRemixAnnouncerCallBurrow] = IFREMIX_ANNOUNCER_TABLE(sBurrowCall),
    [nIFRemixAnnouncerCallBurrowAgain] = IFREMIX_ANNOUNCER_TABLE(sBurrowAgainCalls),
    [nIFRemixAnnouncerCallDig] = IFREMIX_ANNOUNCER_TABLE(sDigCall),
    [nIFRemixAnnouncerCallBodySlam] = IFREMIX_ANNOUNCER_TABLE(sBodySlamCall),
    [nIFRemixAnnouncerCallTaunt] = IFREMIX_ANNOUNCER_TABLE(sTauntCall),
};

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

/** Returns a random entry of the table, the asm Global.get_random_int_ over the table length. */
static IFRemixAnnouncement *ifRemixAnnouncerPickRandom(IFRemixAnnouncerTable *table)
{
    return &table->entries[syUtilsRandIntRange(table->count)];
}

/** Random entries until one has no timer left, one attempt per entry. Returns NULL when every attempt is still cooling down. */
static IFRemixAnnouncement *ifRemixAnnouncerPickFree(IFRemixAnnouncerTable *table)
{
    s32 attempt;

    for (attempt = 0; attempt < table->count; attempt++)
    {
        IFRemixAnnouncement *entry = ifRemixAnnouncerPickRandom(table);

        if (entry->timer == 0)
        {
            return entry;
        }
    }
    return NULL;
}

/**
 * STANDARD pick (asm announcer_main_ _standard). After one attempt per entry, the last random entry is used unless its
 * flag is set, in which case the call stays silent.
 */
static IFRemixAnnouncement *ifRemixAnnouncerPickStandard(IFRemixAnnouncerTable *table)
{
    IFRemixAnnouncement *entry = NULL;
    s32 attempt;

    for (attempt = 0; attempt < table->count; attempt++)
    {
        entry = ifRemixAnnouncerPickRandom(table);
        if (entry->timer == 0)
        {
            return entry;
        }
    }
    if (entry->flag != 0)
    {
        return NULL;
    }
    return entry;
}

/** NAME pick: the Pokemon clip of the item kind, silent while its timer runs or when the item is not a Pokemon. */
static IFRemixAnnouncement *ifRemixAnnouncerPickName(s32 item_kind)
{
    IFRemixAnnouncerTable *table = &sIFRemixAnnouncerTables[nIFRemixAnnouncerCallPokemon];
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
    return ifRemixAnnouncerPickRandom(&sIFRemixAnnouncerTables[nIFRemixAnnouncerCallRegularKO]);
}

/** Quick KO pick, falls back to a regular KO when the quick clip was already used in the match. */
static IFRemixAnnouncement *ifRemixAnnouncerPickQuickKO(void)
{
    IFRemixAnnouncement *entry = ifRemixAnnouncerPickRandom(&sIFRemixAnnouncerTables[nIFRemixAnnouncerCallQuickKO]);

    if (entry->used != 0)
    {
        return ifRemixAnnouncerPickRegularKO();
    }
    return entry;
}

/** Long KO pick, falls back to a regular KO while its timer runs. */
static IFRemixAnnouncement *ifRemixAnnouncerPickLongKO(void)
{
    IFRemixAnnouncement *entry = ifRemixAnnouncerPickRandom(&sIFRemixAnnouncerTables[nIFRemixAnnouncerCallLongKO]);

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
    IFRemixAnnouncement *close_entry = &sIFRemixAnnouncerTables[nIFRemixAnnouncerCallCloseMatch].entries[0];

    if (ifRemixAnnouncerIsCloseMatch() && (close_entry->used == 0))
    {
        return close_entry;
    }
    return ifRemixAnnouncerPickFree(&sIFRemixAnnouncerTables[nIFRemixAnnouncerCallGeneral]);
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
    if (!ifRemixAnnouncerIsGlobalReady())
    {
        return FALSE;
    }
    return ifRemixAnnouncerPlayEntry(ifRemixAnnouncerPickStandard(&sIFRemixAnnouncerTables[call]));
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

/** Resets the timers and used flags of every entry, the asm refresh loop of announcer_setup_. */
static void ifRemixAnnouncerResetTable(IFRemixAnnouncerTable *table)
{
    s32 index;

    for (index = 0; index < table->count; index++)
    {
        table->entries[index].timer = 0;
        table->entries[index].used = FALSE;
    }
}

static void ifRemixAnnouncerResetAllTables(void)
{
    s32 call;

    for (call = 0; call < nIFRemixAnnouncerCallCount; call++)
    {
        ifRemixAnnouncerResetTable(&sIFRemixAnnouncerTables[call]);
    }
}

/** Counts every non-zero entry timer of one table down by one frame. */
static void ifRemixAnnouncerCountTable(IFRemixAnnouncerTable *table)
{
    s32 index;

    for (index = 0; index < table->count; index++)
    {
        if (table->entries[index].timer != 0)
        {
            table->entries[index].timer--;
        }
    }
}

static void ifRemixAnnouncerCountAllTables(void)
{
    s32 call;

    for (call = 0; call < nIFRemixAnnouncerCallCount; call++)
    {
        ifRemixAnnouncerCountTable(&sIFRemixAnnouncerTables[call]);
    }
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
    ifRemixAnnouncerCountGlobalTimer();
    if (sIFRemixAnnouncerSleepRequested)
    {
        ifRemixAnnouncerRunSleep();
        return;
    }
    ifRemixAnnouncerCountAllTables();
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
    ifRemixAnnouncerResetAllTables();
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
