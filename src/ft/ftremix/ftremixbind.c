/**
 * ftremixbind.c - builds Remix fighters' special statuses from their action
 * arrays. See ftremix.h.
 */

#ifdef PORT

#include "ftremix.h"

#include <stdlib.h>
#include <string.h>
#include <port_log.h>
#include "fighter_registry.h"
#include "remix/remix_action_layout.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include <ft/ftchar/ftkirby/ftkirby.h>

extern void ftAnimEndSetWait(GObj *);
extern void ftPhysicsApplyGroundVelFriction(GObj *);
extern void ftPhysicsApplyAirVelDrift(GObj *);
extern void mpCommonSetFighterFallOnEdgeBreak(GObj *);
extern void mpCommonProcFighterWaitOrLanding(GObj *);

#define FTREMIX_PROC_SLOTS 4
#define FTREMIX_STATFLAG_AIR 0x0800

/* Special entry tables in PORT_FIGHTER_SPECIAL_* order. */
static const u32 dFTRemixSpecialTables[PORT_FIGHTER_SPECIAL_COUNT] =
{
    REMIX_SPECIAL_TABLE_GROUND_NSP,
    REMIX_SPECIAL_TABLE_GROUND_USP,
    REMIX_SPECIAL_TABLE_GROUND_DSP,
    REMIX_SPECIAL_TABLE_AIR_NSP,
    REMIX_SPECIAL_TABLE_AIR_USP,
    REMIX_SPECIAL_TABLE_AIR_DSP
};

typedef struct FTRemixBindStats
{
    s32 vanilla;
    s32 ported;
    s32 fallback;
} FTRemixBindStats;

static u32 ftRemixReadWord(u32 addr)
{
    u32 *word = remix_ram_image_resolve(addr);

    return (word != NULL) ? *word : 0;
}

static FTRemixProcFn ftRemixFindVanillaProc(u32 addr)
{
    s32 lo = 0;
    s32 hi = dFTRemixVanillaProcsNum - 1;

    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;

        if (dFTRemixVanillaProcs[mid].addr == addr)
        {
            return dFTRemixVanillaProcs[mid].proc;
        }
        if (dFTRemixVanillaProcs[mid].addr < addr)
        {
            lo = mid + 1;
        }
        else hi = mid - 1;
    }
    return NULL;
}

s32 *ftRemixFindDemoScript(u32 addr)
{
    s32 i;

    for (i = 0; i < dFTRemixDemoScriptsNum; i++)
    {
        if (dFTRemixDemoScripts[i].addr == addr)
        {
            return dFTRemixDemoScripts[i].script;
        }
    }
    return NULL;
}

FTRemixProcFn ftRemixFindProc(u32 addr)
{
    s32 i, j;

    for (i = 0; i < dFTRemixPortedProcTablesNum; i++)
    {
        const FTRemixProcTable *table = &dFTRemixPortedProcTables[i];

        for (j = 0; j < *table->procs_num; j++)
        {
            if (table->procs[j].addr == addr)
            {
                return table->procs[j].proc;
            }
        }
    }
    return NULL;
}

/* Resolves `addr` to a port routine. Returns FALSE when it is a Remix routine
 * with no port yet, so the caller picks a fallback. */
static sb32 ftRemixResolveProc(u32 addr, FTRemixProcFn *out, FTRemixBindStats *stats)
{
    FTRemixProcFn proc;

    if (addr == 0)
    {
        *out = NULL;
        return TRUE;
    }
    proc = (addr < 0x80400000u) ? ftRemixFindVanillaProc(addr) : ftRemixFindProc(addr);

    if (proc == NULL)
    {
        return FALSE;
    }
    if (addr < 0x80400000u)
    {
        stats->vanilla++;
    }
    else stats->ported++;

    *out = proc;
    return TRUE;
}

/* Routine of slot `slot` (update, interrupt, physics, map) in `desc`. */
static FTRemixProcFn *ftRemixGetProcSlot(FTStatusDesc *desc, s32 slot)
{
    switch (slot)
    {
    case 0: return &desc->proc_update;
    case 1: return &desc->proc_interrupt;
    case 2: return &desc->proc_physics;
    default: return &desc->proc_map;
    }
}

/* Generic routine for a new action whose Remix routine is not ported yet. */
static FTRemixProcFn ftRemixGetGenericProc(s32 slot, sb32 is_air)
{
    switch (slot)
    {
    case 0: return ftAnimEndSetWait;
    case 1: return NULL;
    case 2: return (is_air) ? ftPhysicsApplyAirVelDrift : ftPhysicsApplyGroundVelFriction;
    default: return (is_air) ? mpCommonProcFighterWaitOrLanding : mpCommonSetFighterFallOnEdgeBreak;
    }
}

static void ftRemixBindAction(FTStatusDesc *desc, u32 entry, const FTStatusDesc *parent_desc, s32 remix_id,
                              s32 action, FTRemixBindStats *stats)
{
    u32 word0 = ftRemixReadWord(entry);
    s32 slot;

    desc->mflags.motion_id = (s16)(word0 >> 16) >> 6;
    desc->mflags.attack_id = (word0 >> 16) & 0x3F;
    desc->sflags.halfword = word0 & 0xFFFF;

    for (slot = 0; slot < FTREMIX_PROC_SLOTS; slot++)
    {
        u32 addr = ftRemixReadWord(entry + 4 + (slot * 4));
        FTRemixProcFn *proc = ftRemixGetProcSlot(desc, slot);

        if (ftRemixResolveProc(addr, proc, stats) != FALSE)
        {
            continue;
        }
        stats->fallback++;
        *proc = (parent_desc != NULL) ? *ftRemixGetProcSlot((FTStatusDesc *)parent_desc, slot)
                                      : ftRemixGetGenericProc(slot, (desc->sflags.halfword & FTREMIX_STATFLAG_AIR) != 0);
        port_log("SSB64: ftRemix: id 0x%02X action 0x%X slot %d: Remix routine 0x%08X not ported, using %s\n",
                 remix_id, nFTCommonStatusSpecialStart + action, slot, addr,
                 (parent_desc != NULL) ? "parent's" : "generic");
    }
}

static void ftRemixBindKirbyCopyHandlers(struct FighterDescriptor *descriptor, s32 remix_id, FTRemixBindStats *stats)
{
    static const u32 tables[2] = { REMIX_SPECIAL_TABLE_KIRBY_GROUND_NSP, REMIX_SPECIAL_TABLE_KIRBY_AIR_NSP };
    s32 i;

    for (i = 0; i < 2; i++)
    {
        u32 addr = ftRemixReadWord(tables[i] + (remix_id * 4));
        FTRemixProcFn proc;

        if ((addr != 0) && (ftRemixResolveProc(addr, &proc, stats) != FALSE))
        {
            descriptor->kirby_copy_handler[i] = proc;
            continue;
        }
        stats->fallback++;
        descriptor->kirby_copy_handler[i] = NULL;
        port_log("SSB64: ftRemix: id 0x%02X Kirby copy entry %d: Remix routine 0x%08X not ported\n", remix_id, i, addr);
    }
}

static void ftRemixBindHandlers(struct FighterDescriptor *descriptor, s32 remix_id, s32 parent_fkind,
                                FTRemixBindStats *stats)
{
    const FighterDescriptor *parent = port_fighter_descriptor(parent_fkind);
    s32 i;

    for (i = 0; i < PORT_FIGHTER_SPECIAL_COUNT; i++)
    {
        u32 addr = ftRemixReadWord(dFTRemixSpecialTables[i] + (remix_id * 4));
        FTRemixProcFn proc;

        if ((addr != 0) && (ftRemixResolveProc(addr, &proc, stats) != FALSE))
        {
            descriptor->special_handler[i] = proc;
            continue;
        }
        stats->fallback++;
        descriptor->special_handler[i] = (parent != NULL) ? parent->special_handler[i] : NULL;
        port_log("SSB64: ftRemix: id 0x%02X special entry %d: Remix routine 0x%08X not ported, using parent's\n",
                 remix_id, i, addr);
    }
    ftRemixBindKirbyCopyHandlers(descriptor, remix_id, stats);
}

s32 ftRemixKirbyVanillaMotionCount(void)
{
    return nFTKirbyMotionCopyYoshiSpecialAirNRelease + 1;
}

s32 ftRemixBindKirbyActions(struct FighterDescriptor *descriptor)
{
    FTRemixBindStats stats = { 0, 0, 0 };
    s32 count = kRemixActionCounts[REMIX_ID_KIRBY];
    u32 array = ftRemixReadWord(REMIX_ACTION_ARRAY_TABLE + (REMIX_ID_KIRBY * 4));
    s32 parent_action_count = nFTKirbyStatusCopyYoshiSpecialAirNRelease - nFTCommonStatusSpecialStart + 1;
    FTStatusDesc *parent_descs = port_fighter_special_descs(nFTKindKirby);
    FTStatusDesc *descs;
    s32 i;

    if ((count == 0) || (array == 0) || (parent_descs == NULL))
    {
        port_log("SSB64: ftRemix: Kirby has no action array; keeping the vanilla specials\n");
        return 0;
    }
    descs = malloc(count * sizeof(FTStatusDesc));

    if (descs == NULL)
    {
        port_log("SSB64: ftRemix: Kirby: cannot allocate %d status descs\n", count);
        return 0;
    }
    memset(descs, 0, count * sizeof(FTStatusDesc));
    for (i = 0; i < count; i++)
    {
        const FTStatusDesc *parent_desc = (i < parent_action_count) ? &parent_descs[i] : NULL;

        ftRemixBindAction(&descs[i], array + (i * REMIX_ACTION_ENTRY_SIZE), parent_desc, REMIX_ID_KIRBY, i, &stats);
    }
    descriptor->special_descs = descs;
    descriptor->special_descs_count = count;

    port_log("SSB64: ftRemix: Kirby bound %d actions: %d vanilla, %d ported, %d fallback routines\n",
             count, stats.vanilla, stats.ported, stats.fallback);
    return stats.fallback;
}

s32 ftRemixBindSpecials(s32 remix_id, s32 parent_fkind, s32 parent_action_count, struct FighterDescriptor *descriptor)
{
    FTRemixBindStats stats = { 0, 0, 0 };
    s32 count = kRemixActionCounts[remix_id];
    u32 array = ftRemixReadWord(REMIX_ACTION_ARRAY_TABLE + (remix_id * 4));
    FTStatusDesc *parent_descs = port_fighter_special_descs(parent_fkind);
    FTStatusDesc *descs;
    s32 i;

    if ((count == 0) || (array == 0))
    {
        port_log("SSB64: ftRemix: id 0x%02X has no action array; keeping the parent's specials\n", remix_id);
        return 0;
    }
    descs = malloc(count * sizeof(FTStatusDesc));

    if (descs == NULL)
    {
        port_log("SSB64: ftRemix: id 0x%02X: cannot allocate %d status descs\n", remix_id, count);
        return 0;
    }
    memset(descs, 0, count * sizeof(FTStatusDesc));
    for (i = 0; i < count; i++)
    {
        const FTStatusDesc *parent_desc = ((parent_descs != NULL) && (i < parent_action_count)) ? &parent_descs[i] : NULL;

        ftRemixBindAction(&descs[i], array + (i * REMIX_ACTION_ENTRY_SIZE), parent_desc, remix_id, i, &stats);
    }
    descriptor->special_descs = descs;
    descriptor->special_descs_count = count;

    ftRemixBindHandlers(descriptor, remix_id, parent_fkind, &stats);

    port_log("SSB64: ftRemix: id 0x%02X bound %d actions: %d vanilla, %d ported, %d fallback routines\n",
             remix_id, count, stats.vanilla, stats.ported, stats.fallback);
    return stats.fallback;
}

#endif
