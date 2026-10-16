/*
 * ftremixpoison.c - poison damage over time of Smash Remix (.toolchain/pkgs/smashremix-src/src/Poison.asm).
 * ftMainProcUpdateInterrupt calls ftRemixPoisonUpdate where the heal block is skipped (damage_heal == 0), which is
 * Poison.asm poison_hook. The Spawned Fear acid (Hazards.asm) writes the entries through ftRemixPoisonSet.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <gm/gmremixcolscripts.h>
#include <it/itremix/itremixcloakingdevice.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "ft/ftremix/ftremixhooks.h"

extern void *func_800269C0_275C0(u16 id);

/* asm: Poison.asm DAMAGE_SFX */
#define FTREMIX_POISON_DAMAGE_SFX 0x1C
/* asm: Poison.asm OVERLAY = GFXRoutine.DOOM_ACID (Remix colour animation id) */
#define FTREMIX_POISON_DOOM_ACID_COLANIM 0x79
/* asm: Poison.asm damage every 32 frames, the match timer low bits must be zero */
#define FTREMIX_POISON_TICK_MASK 0x1F
/* asm: Poison.asm poisoned_players has four port entries */
#define FTREMIX_POISON_PORTS 4
/* asm: Poison.asm 0x800E18A0, the heal tail resets colanim 9 once no heal is left */
#define FTREMIX_POISON_BURN_COLANIM_RESET 0x9
/* REMIX-TODO(toggles): css/PoisonDmg.asm apply_damage (CSS "Poison Damage" option, state_table per port, Remix default
 * 0 = Off) is not ported. With that default the asm returns at once, so skipping it changes nothing. */
#define FTREMIX_POISON_CSS_DAMAGE_DEFAULT FALSE

typedef struct FTRemixPoisonEntry
{
    s32 duration; /* asm: frames left, 0 = not poisoned */
    s32 damage;   /* asm: percent added every 32 frames */
    s32 type;     /* asm: 0 = doom acid, the CSS types 1..4 are not ported */
} FTRemixPoisonEntry;

static FTRemixPoisonEntry sFTRemixPoisoned[FTREMIX_POISON_PORTS];
static u16 sFTRemixPoisonTimer;

/**
 * Sets the poison of a port (asm: Poison.poisoned_players entry written by the acid hazard).
 * Ports outside 0..3 are ignored.
 */
void ftRemixPoisonSet(s32 port, s32 duration, s32 damage, s32 type)
{
    FTRemixPoisonEntry *entry;

    if ((port < 0) || (port >= FTREMIX_POISON_PORTS))
    {
        return;
    }
    entry = &sFTRemixPoisoned[port];
    entry->duration = duration;
    entry->damage = damage;
    entry->type = type;
}

/** Advances the 16-bit poison timer once per frame (asm: the halfword at 0x801313F8, written by the acid hazard). */
void ftRemixPoisonAdvanceTimer(void)
{
    sFTRemixPoisonTimer++;
}

/** Clears the poison durations between matches (asm: Poison.clear_poison_, called by Item.asm:1577, not wired yet). */
void ftRemixPoisonClear(void)
{
    s32 port;

    for (port = 0; port < FTREMIX_POISON_PORTS; port++)
    {
        sFTRemixPoisoned[port].duration = 0;
    }
}

/**
 * Counts the poison down by one frame and clears the colour animation when it ends.
 * Returns TRUE when the damage check of this frame must run.
 */
static sb32 ftRemixPoisonCountDown(FTStruct *fp)
{
    FTRemixPoisonEntry *entry = &sFTRemixPoisoned[fp->player];

    if ((entry->duration == 0) || itRemixCloakingDeviceIsCloaked(fp->player))
    {
        return FALSE;
    }
    entry->duration--;
    if (entry->duration != 0)
    {
        return TRUE;
    }
    if (gmRemixColScriptsGetOverride(fp->player) == FTREMIX_POISON_DOOM_ACID_COLANIM)
    {
        gmRemixColScriptsSetOverride(fp->player, 0);
        return FALSE;
    }
    return TRUE;
}

/**
 * Checks whether the poison hurts this fighter on this frame: once every 32 frames, not while invincible or respawning.
 * A fighter below the Idle action is respawning, which ends the poison.
 */
static sb32 ftRemixPoisonCanDamage(FTStruct *fp)
{
    if ((sFTRemixPoisonTimer & FTREMIX_POISON_TICK_MASK) != 0)
    {
        return FALSE;
    }
    if ((fp->invincible_tics != 0) || (fp->star_invincible_tics != 0))
    {
        return FALSE;
    }
    if (fp->status_id < nFTCommonStatusWait)
    {
        sFTRemixPoisoned[fp->player].duration = 0;
        gmRemixColScriptsSetOverride(fp->player, 0);
        return FALSE;
    }
    return TRUE;
}

/**
 * Adds the poison damage to the fighter and starts the doom acid colour animation when no override is active.
 * Plays the poison sound on every damage tick.
 */
static void ftRemixPoisonDamage(GObj *fighter_gobj, FTStruct *fp)
{
    /* REMIX-TODO(Stamina): asm _continue_2 clamps the percent at the stamina total in stamina mode and skips defeated stocks; no port yet. */
    fp->percent_damage += sFTRemixPoisoned[fp->player].damage;
    if (gmRemixColScriptsGetOverride(fp->player) == 0)
    {
        gmRemixColScriptsSetOverride(fp->player, FTREMIX_POISON_DOOM_ACID_COLANIM);
        ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_POISON_DOOM_ACID_COLANIM, 0);
    }
    func_800269C0_275C0(FTREMIX_POISON_DAMAGE_SFX);
}

/**
 * Copies the damage into the stock record and resets the colour animation 9 once the fighter is no longer healing.
 * The tail of the heal block, which the asm poison path also runs (Poison.asm `j 0x800E1850`).
 */
static void ftRemixPoisonSyncStock(GObj *fighter_gobj, FTStruct *fp)
{
    gSCManagerBattleState->players[fp->player].stock_damage_all = fp->percent_damage;
    if ((fp->damage_heal == 0) && (fp->colanim.colanim_id == FTREMIX_POISON_BURN_COLANIM_RESET))
    {
        ftParamResetStatUpdateColAnim(fighter_gobj);
    }
}

/**
 * Per-frame poison of one fighter (asm: Poison.poison_hook, the `_poison_check` path).
 * Called from ftMainProcUpdateInterrupt when the fighter is not healing.
 */
void ftRemixPoisonUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!ftRemixPoisonCountDown(fp))
    {
        return;
    }
    if (!ftRemixPoisonCanDamage(fp))
    {
        return;
    }
    ftRemixPoisonDamage(fighter_gobj, fp);
    ftRemixPoisonSyncStock(fighter_gobj, fp);
}

#endif
