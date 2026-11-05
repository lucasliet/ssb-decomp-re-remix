/*
 * ftremixpoison.c - poison damage over time of Smash Remix (.toolchain/pkgs/smashremix-src/src/Poison.asm).
 * ftMainProcUpdateInterrupt calls ftRemixPoisonUpdate right before its heal block when no heal is pending (damage_heal == 0),
 * which is Poison.asm poison_hook; a CSS Heal therefore heals on the same frame. The Spawned Fear acid (Hazards.asm) writes the entries through ftRemixPoisonSet.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <gm/gmremixcolscripts.h>
#include <gm/gmremixpanel.h>
#include <gm/gmremixstamina.h>
#include <gm/gmremixstaminarules.h>
#include <gm/gmremixtoggle.h>
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
/* asm: css/PoisonDmg.asm apply_damage ticks when the match tic low 7 bits are zero, about every 2 s */
#define FTREMIX_POISON_PANEL_TICK_MASK 0x7F
/* asm: at game speed 1/3 or 2/3 the extra tics of the speed step can leave the low bits at 1 or 2 */
#define FTREMIX_POISON_PANEL_SLOW_TICK_LIMIT 3
/* asm: Toggles entry_game_speed values of 1/3 and 2/3 speed */
#define FTREMIX_POISON_PANEL_SPEED_THIRD 9
#define FTREMIX_POISON_PANEL_SPEED_TWO_THIRDS 11
/* asm: apply_damage caps the percent at 999 */
#define FTREMIX_POISON_PANEL_CAP 999
/* asm: apply_damage Heal value adds 2 to the healing amount (0x0818) */
#define FTREMIX_POISON_PANEL_HEAL_AMOUNT 2
/* asm: css/PoisonDmg.asm state_table values */
#define FTREMIX_POISON_PANEL_NONE 0
#define FTREMIX_POISON_PANEL_LOW 1
#define FTREMIX_POISON_PANEL_MEDIUM 2
#define FTREMIX_POISON_PANEL_HIGH 3
#define FTREMIX_POISON_PANEL_HEAL 4

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
 * Returns TRUE when a Stamina battle fighter is out of HP, so the poison skips it (asm Poison.asm _poison_continue_2 and
 * css/PoisonDmg.asm apply_damage, which skip a player whose stock is already defeated).
 */
static sb32 ftRemixPoisonIsStaminaDefeated(FTStruct *fp)
{
    return gmRemixStaminaIsActive() && gmRemixStaminaIsDefeated(fp->player);
}

/**
 * Caps a poison percent at the Stamina total HP in a Stamina battle (asm Poison.asm _continue_2, css/PoisonDmg.asm
 * _stamina). Other battles keep the percent.
 */
static s32 ftRemixPoisonStaminaCap(s32 percent)
{
    if (gmRemixStaminaIsActive() && gmRemixStaminaRulesIsOverTotal(percent, gmRemixStaminaGetTotalHP()))
    {
        return gmRemixStaminaGetTotalHP();
    }
    return percent;
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
    return !ftRemixPoisonIsStaminaDefeated(fp);
}

/**
 * Adds the poison damage to the fighter and starts the doom acid colour animation when no override is active.
 * Plays the poison sound on every damage tick.
 */
static void ftRemixPoisonDamage(GObj *fighter_gobj, FTStruct *fp)
{
    fp->percent_damage = ftRemixPoisonStaminaCap(fp->percent_damage + sFTRemixPoisoned[fp->player].damage);
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
 * Returns TRUE on the match tic when the CSS Poison Dmg option ticks (asm apply_damage, the speed test).
 * At default speed the low tic bits must be zero; at 1/3 and 2/3 speed they must be below the speed step limit.
 * The extra advance of a faster speed never ticks.
 */
static sb32 ftRemixPoisonPanelTickDue(void)
{
    s32 elapsed_tics = gSCManagerBattleState->time_passed;
    s32 speed_toggle = gmRemixToggle(game_speed);
    s32 low_bits = elapsed_tics & FTREMIX_POISON_PANEL_TICK_MASK;

    if (elapsed_tics == 0)
    {
        return FALSE;
    }
    if (speed_toggle == 0)
    {
        return low_bits == 0;
    }
    if (ftRemixSpeedIsAdditionalAdvance())
    {
        return FALSE;
    }
    if ((speed_toggle == FTREMIX_POISON_PANEL_SPEED_THIRD) || (speed_toggle == FTREMIX_POISON_PANEL_SPEED_TWO_THIRDS))
    {
        return low_bits < FTREMIX_POISON_PANEL_SLOW_TICK_LIMIT;
    }
    return low_bits == 0;
}

/**
 * Returns TRUE when a Super Star or the respawn stops the CSS Poison Dmg option (asm apply_damage).
 * The spawn invincibility does not stop it, unlike the hazard poison.
 */
static sb32 ftRemixPoisonPanelIsBlocked(FTStruct *fp)
{
    if (fp->star_invincible_tics != 0)
    {
        return TRUE;
    }
    return fp->status_id < nFTCommonStatusWait;
}

/** Returns the percent added on each tick for a CSS Poison Dmg value (asm apply_damage _apply_damage). */
static s32 ftRemixPoisonPanelAmount(s32 panel_value)
{
    switch (panel_value)
    {
        case FTREMIX_POISON_PANEL_LOW:
            return 1;

        case FTREMIX_POISON_PANEL_MEDIUM:
            return 2;

        case FTREMIX_POISON_PANEL_HIGH:
            return 4;

        default:
            return 0;
    }
}

/**
 * Heals the fighter by the panel amount through the healing block of ftMainProcUpdateInterrupt, which reads damage_heal
 * (asm 0x0818) as the amount of healing left.
 */
static void ftRemixPoisonPanelHeal(FTStruct *fp)
{
    fp->damage_heal = FTREMIX_POISON_PANEL_HEAL_AMOUNT;
}

/**
 * Adds the panel damage to the percent, capped at 999, and syncs the stock record (asm apply_damage, then the tail at
 * 0x800E1850).
 */
static void ftRemixPoisonPanelDamage(GObj *fighter_gobj, FTStruct *fp, s32 amount)
{
    s32 percent = fp->percent_damage + amount;

    if (percent > FTREMIX_POISON_PANEL_CAP)
    {
        percent = FTREMIX_POISON_PANEL_CAP;
    }
    fp->percent_damage = ftRemixPoisonStaminaCap(percent);
    ftRemixPoisonSyncStock(fighter_gobj, fp);
}

/**
 * Applies the CSS Poison Dmg option for this match tic. Returns TRUE when it healed or hurt the fighter, which skips the
 * hazard poison of this frame as the asm does. The cloak check only guards the damage, not the heal (asm apply_damage
 * order).
 */
static sb32 ftRemixPoisonPanelTryApply(GObj *fighter_gobj, FTStruct *fp)
{
    s32 panel_value;

    if (!ftRemixPoisonPanelTickDue() || ftRemixPoisonPanelIsBlocked(fp) || ftRemixPoisonIsStaminaDefeated(fp))
    {
        return FALSE;
    }
    panel_value = gmRemixPanel(POISON_DAMAGE, fp->player);
    if (panel_value == FTREMIX_POISON_PANEL_NONE)
    {
        return FALSE;
    }
    if (panel_value == FTREMIX_POISON_PANEL_HEAL)
    {
        ftRemixPoisonPanelHeal(fp);
        return TRUE;
    }
    if (itRemixCloakingDeviceIsCloaked(fp->player))
    {
        return FALSE;
    }
    ftRemixPoisonPanelDamage(fighter_gobj, fp, ftRemixPoisonPanelAmount(panel_value));
    return TRUE;
}

/**
 * Per-frame poison of one fighter: the CSS Poison Dmg option first (asm apply_damage), then the hazard poison
 * (asm Poison.poison_hook, the `_poison_check` path).
 * Called from ftMainProcUpdateInterrupt when the fighter is not healing.
 */
void ftRemixPoisonUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixPoisonPanelTryApply(fighter_gobj, fp))
    {
        return;
    }
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
