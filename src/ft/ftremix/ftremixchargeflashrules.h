#ifndef _FTREMIXCHARGEFLASHRULES_H_
#define _FTREMIXCHARGEFLASHRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the Character.gfx_routine_end table (Character.asm:2179, read by get_gfx_routine_end_ at 0x800E9948 in
 * ftParamResetStatUpdateColAnim): the fully charged glow that is started again when another color animation ends. Only
 * int, so the headless unit tests (tests/remix) can run it.
 */

/* colanim_id of a fighter that takes the vanilla Samus / Donkey Kong glow (nGMColAnimFighterCommonSpecialNCharge) */
#define FTREMIX_CHARGE_FLASH_COMMON (-1)

/** One row of the table: the charge word to read, the level that counts as fully charged and the glow to start. */
typedef struct FTRemixChargeFlash
{
    int n64_offset;     /* N64 FTStruct offset of the charge word (passive_vars) */
    int full_level;
    int colanim_id;     /* GFXRoutine id, or FTREMIX_CHARGE_FLASH_COMMON */

} FTRemixChargeFlash;

/**
 * Charge glow of a Remix fighter id.
 *
 * @param fkind Remix fighter id (also the port fkind).
 * @param flash Receives the row when the fighter has one.
 * @return 1 when the fighter's effective gfx_routine_end entry restarts a charge glow, else 0 (DISABLED, no-op parent).
 */
extern int ftRemixChargeFlashFind(int fkind, FTRemixChargeFlash *flash);

#endif

#endif
