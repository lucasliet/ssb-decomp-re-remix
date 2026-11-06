/**
 * ifremixannouncertables.c - call tables of the Pokemon Stadium announcer (PokemonAnnouncer.asm, "CALL STRUCTS"), see
 * ifremixannouncerrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixannouncerrules.h"

#define IFREMIX_ANNOUNCER_LENGTH(calls) ((int)(sizeof(calls) / sizeof((calls)[0])))

/* Field order and size match the asm add_announcement macro (16 bytes per entry). */
#define IFREMIX_ANNOUNCEMENT(announcer_id, fgm, time, special) { announcer_id, fgm, time, 0, 0, special }

/* A table whose draw covers all its entries. */
#define IFREMIX_ANNOUNCER_TABLE(calls) { calls, IFREMIX_ANNOUNCER_LENGTH(calls), IFREMIX_ANNOUNCER_LENGTH(calls), 0 }

/* A table that owns the first "owned" entries of its array and draws over the first "picks" entries, the asm a1 of announcer_main_. */
#define IFREMIX_ANNOUNCER_TABLE_PICKS(calls, owned, picks) { calls, owned, picks, 0 }

/* A table of a call that plays at most once per match (clang_announcement_ tests the used byte first). */
#define IFREMIX_ANNOUNCER_TABLE_ONCE(calls) { calls, IFREMIX_ANNOUNCER_LENGTH(calls), IFREMIX_ANNOUNCER_LENGTH(calls), 1 }

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

/* Memory order of the asm: the 6 mid calls follow the 12 high calls, and the high pick draws over all 18. */
static IFRemixAnnouncement sKnockbackCalls[] =
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

/* The asm draws over 1 entry (hit_announcement_ _fire passes a1 = 1), so the second burn call never plays. */
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

/* The opening calls follow the Gym Leader Castle call in the asm, which draws over 2 (_gym of announcer_constant_). */
static IFRemixAnnouncement sGymOpeningCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x4E, 0x4B6, 0x8708, 0x1), /* We bring you this battle live from gym leader castle */
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

/* The two burrow again calls follow the burrow call in the asm, and every burrow draws over all 3 (burrow_announcement_). */
static IFRemixAnnouncement sBurrowCalls[] =
{
    IFREMIX_ANNOUNCEMENT(0x50, 0x5D1, 0x708, 0x1),  /* It Burrowed Underground */
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

/* Order follows IFRemixAnnouncerCall. */
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
    [nIFRemixAnnouncerCallKnockbackHigh] = IFREMIX_ANNOUNCER_TABLE_PICKS(sKnockbackCalls, 12, 18),
    [nIFRemixAnnouncerCallKnockbackMid] = IFREMIX_ANNOUNCER_TABLE_PICKS(sKnockbackCalls + 12, 6, 6),
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
    [nIFRemixAnnouncerCallBurn] = IFREMIX_ANNOUNCER_TABLE_PICKS(sBurnCalls, 2, 1),
    [nIFRemixAnnouncerCallSeismicToss] = IFREMIX_ANNOUNCER_TABLE(sSeismicTossCall),
    [nIFRemixAnnouncerCallRecovery] = IFREMIX_ANNOUNCER_TABLE(sRecoveryCall),
    [nIFRemixAnnouncerCallClang] = IFREMIX_ANNOUNCER_TABLE_ONCE(sClangCall),
    [nIFRemixAnnouncerCallStun] = IFREMIX_ANNOUNCER_TABLE(sStunCalls),
    [nIFRemixAnnouncerCallTeamKO] = IFREMIX_ANNOUNCER_TABLE(sTeamKOCalls),
    [nIFRemixAnnouncerCallSlash] = IFREMIX_ANNOUNCER_TABLE(sSlashCall),
    [nIFRemixAnnouncerCallFlamethrower] = IFREMIX_ANNOUNCER_TABLE(sFlamethrowerCall),
    [nIFRemixAnnouncerCallGymLeader] = IFREMIX_ANNOUNCER_TABLE_PICKS(sGymOpeningCalls, 1, 2),
    [nIFRemixAnnouncerCallOpening] = IFREMIX_ANNOUNCER_TABLE_PICKS(sGymOpeningCalls + 1, 3, 3),
    [nIFRemixAnnouncerCallSing] = IFREMIX_ANNOUNCER_TABLE(sSingCall),
    [nIFRemixAnnouncerCallAttack] = IFREMIX_ANNOUNCER_TABLE(sAttackCalls),
    [nIFRemixAnnouncerCallDamageEarly] = IFREMIX_ANNOUNCER_TABLE(sDamageEarlyCall),
    [nIFRemixAnnouncerCallBurrow] = IFREMIX_ANNOUNCER_TABLE_PICKS(sBurrowCalls, 1, 3),
    [nIFRemixAnnouncerCallBurrowAgain] = IFREMIX_ANNOUNCER_TABLE_PICKS(sBurrowCalls + 1, 2, 2),
    [nIFRemixAnnouncerCallDig] = IFREMIX_ANNOUNCER_TABLE(sDigCall),
    [nIFRemixAnnouncerCallBodySlam] = IFREMIX_ANNOUNCER_TABLE(sBodySlamCall),
    [nIFRemixAnnouncerCallTaunt] = IFREMIX_ANNOUNCER_TABLE(sTauntCall),
};

IFRemixAnnouncerTable *ifRemixAnnouncerTablesGet(IFRemixAnnouncerCall call)
{
    return &sIFRemixAnnouncerTables[call];
}

/** Resets the timer and the used flag of the entries a table owns, the asm refresh loop of announcer_setup_. */
static void ifRemixAnnouncerTablesResetOne(IFRemixAnnouncerTable *table)
{
    int index;

    for (index = 0; index < table->count; index++)
    {
        table->entries[index].timer = 0;
        table->entries[index].used = 0;
    }
}

void ifRemixAnnouncerTablesReset(void)
{
    int call;

    for (call = 0; call < nIFRemixAnnouncerCallCount; call++)
    {
        ifRemixAnnouncerTablesResetOne(&sIFRemixAnnouncerTables[call]);
    }
}

/** Counts every non-zero timer of the entries a table owns down by one frame. */
static void ifRemixAnnouncerTablesCountDownOne(IFRemixAnnouncerTable *table)
{
    int index;

    for (index = 0; index < table->count; index++)
    {
        if (table->entries[index].timer != 0)
        {
            table->entries[index].timer--;
        }
    }
}

void ifRemixAnnouncerTablesCountDown(void)
{
    int call;

    for (call = 0; call < nIFRemixAnnouncerCallCount; call++)
    {
        ifRemixAnnouncerTablesCountDownOne(&sIFRemixAnnouncerTables[call]);
    }
}

#endif
