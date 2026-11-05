/**
 * ftremixbowserammo.c - fire breath ammo of Bowser and of Kirby with the Bowser power (src/Bowser/bowser.asm
 * bowser_nsp_recharge, src/SinglePlayerModes.asm limbo_clear, src/jigglypuffkirbyshared.asm kirby_blast_fix_1). The pure
 * rules are in ftremixbowserammorules.c; the ammo word is the one ftremixbowser.c reads (a halfword at N64 0x0ADE for
 * Bowser, 0x0AE2 for Kirby).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/bowser/ftremixbowserhooks.h"
#include "ft/ftremix/bowser/ftremixbowserammorules.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_BOWSER_AMMO_BOWSER_OFFSET 0x0ADC
#define FTREMIX_BOWSER_AMMO_KIRBY_OFFSET 0x0AE0

/* BowserNSP statuses of Bowser (0xE4 ground, 0xE7 air) and of Kirby's copy (Kirby.Action BOWSER_NSP_Ground / _Air). */
#define FTREMIX_BOWSER_STATUS_FIRE_BREATH 0xE4
#define FTREMIX_BOWSER_STATUS_FIRE_BREATH_AIR 0xE7
#define FTREMIX_BOWSER_KIRBY_STATUS_FIRE_BREATH 0x13E
#define FTREMIX_BOWSER_KIRBY_STATUS_FIRE_BREATH_AIR 0x13F

static sb32 ftRemixBowserAmmoIsKirbyWithBowser(FTStruct *fp)
{
    return ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY)) && (fp->passive_vars.kirby.copy_id == REMIX_ID_BOWSER);
}

static sb32 ftRemixBowserAmmoIsFiring(FTStruct *fp)
{
    if (fp->fkind == REMIX_ID_BOWSER)
    {
        return (fp->status_id == FTREMIX_BOWSER_STATUS_FIRE_BREATH) || (fp->status_id == FTREMIX_BOWSER_STATUS_FIRE_BREATH_AIR);
    }
    return (fp->status_id == FTREMIX_BOWSER_KIRBY_STATUS_FIRE_BREATH) || (fp->status_id == FTREMIX_BOWSER_KIRBY_STATUS_FIRE_BREATH_AIR);
}

void ftRemixBowserAmmoInitFighter(FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_BOWSER)
    {
        return;
    }
    *ftRemixFighterWord(fp, FTREMIX_BOWSER_AMMO_BOWSER_OFFSET) = (s32)ftRemixBowserAmmoSpawnWord();
}

void ftRemixBowserAmmoRefillOnKO(FTStruct *fp)
{
    s32 *ammo_word;

    if (fp->fkind != REMIX_ID_BOWSER)
    {
        return;
    }
    ammo_word = ftRemixFighterWord(fp, FTREMIX_BOWSER_AMMO_BOWSER_OFFSET);
    *ammo_word = (s32)ftRemixBowserAmmoRefillWord((u32)*ammo_word);
}

void ftRemixBowserAmmoRecharge(FTStruct *fp)
{
    s32 *ammo_word;

    if (fp->fkind == REMIX_ID_BOWSER)
    {
        ammo_word = ftRemixFighterWord(fp, FTREMIX_BOWSER_AMMO_BOWSER_OFFSET);
    }
    else if (ftRemixBowserAmmoIsKirbyWithBowser(fp) != FALSE)
    {
        ammo_word = ftRemixFighterWord(fp, FTREMIX_BOWSER_AMMO_KIRBY_OFFSET);
    }
    else return;

    *ammo_word = (s32)ftRemixBowserAmmoRechargeWord((u32)*ammo_word, (ftRemixBowserAmmoIsFiring(fp) != FALSE) ? 1 : 0);
}

#endif
