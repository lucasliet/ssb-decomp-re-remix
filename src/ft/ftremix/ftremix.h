#ifndef _FTREMIX_H_
#define _FTREMIX_H_

/*
 * Smash Remix action binding (PORT only).
 *
 * Remix gives every new fighter an action array (statuses 0xDC + i) whose
 * entries name status routines by N64 address, plus per-id special-move entry
 * tables. ftRemixBindSpecials turns those into the synth's FTStatusDesc table
 * and special entry handlers: vanilla addresses map to decomp functions
 * (ftremixvanillaprocs.c, generated), Remix addresses map to the C ports
 * listed in ftremixported.c, and anything unported falls back to the parent's
 * routine for the same slot.
 */

#include <ft/fighter.h>
#include "remix/remix_roster_data.h" /* REMIX_ID_*: a Remix id is also its port fkind */

typedef void (*FTRemixProcFn)(GObj *);

typedef struct FTRemixProc
{
    u32 addr;           /* N64 RAM address of the routine */
    FTRemixProcFn proc; /* Port implementation */
} FTRemixProc;

typedef struct FTRemixProcTable
{
    const FTRemixProc *procs;
    const s32 *procs_num;
} FTRemixProcTable;

typedef struct FTRemixScript
{
    u32 addr;    /* N64 RAM address of a vanilla overlay motion script */
    s32 *script; /* Decomp array holding it */
} FTRemixScript;

extern const FTRemixScript dFTRemixDemoScripts[];
extern const s32 dFTRemixDemoScriptsNum;

extern const FTRemixProc dFTRemixVanillaProcs[];
extern const s32 dFTRemixVanillaProcsNum;

extern const FTRemixProcTable dFTRemixPortedProcTables[];
extern const s32 dFTRemixPortedProcTablesNum;

struct FighterDescriptor;

/**
 * Builds `descriptor`'s special status table and special entry handlers from
 * Remix id `remix_id`'s action array. `parent_fkind` supplies fallbacks.
 * Returns the number of routine slots that fell back to the parent.
 */
s32 ftRemixBindSpecials(s32 remix_id, s32 parent_fkind, s32 parent_action_count, struct FighterDescriptor *descriptor);

/**
 * Grows Kirby's vanilla specials with the copy abilities' actions of Remix's Kirby action array.
 * Returns the number of routine slots that did not resolve to a port routine.
 */
s32 ftRemixBindKirbyActions(struct FighterDescriptor *descriptor);

struct FTData;

/**
 * The FTData whose special files `fp`'s moves read: Remix addresses a character's files through global pointers
 * (Character.X_file_n_ptr), so Kirby wearing a copied power reads the copied fighter's files, not Kirby's.
 */
struct FTData *ftRemixPowerData(struct FTStruct *fp);

/** Port implementation for Remix routine `addr`, or NULL when it has none. */
FTRemixProcFn ftRemixFindProc(u32 addr);

/** Decomp array for vanilla overlay motion script `addr`, or NULL. */
s32 *ftRemixFindDemoScript(u32 addr);

#endif
