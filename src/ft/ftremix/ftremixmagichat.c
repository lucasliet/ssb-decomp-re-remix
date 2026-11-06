/**
 * ftremixmagichat.c - '???' Magic Kirby Hat, see ftremixmagichat.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixmagichat.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixmagichatrules.h"
#include "remix/remix_roster_data.h"
#include <ef/efmanager.h>
#include <ft/ftparam.h>
#include <ft/ftmanager.h>
#include <gm/gmcollision.h>
#include <sys/utils.h>

#define FTREMIXMAGICHAT_PORTS 4
#define FTREMIXMAGICHAT_HAT_JOINT 6
#define FTREMIXMAGICHAT_BOWSER_AMMO 0x14
#define FTREMIXMAGICHAT_HALFWORD_MASK 0xFFFF
#define FTREMIXMAGICHAT_SPARK_OFFSET_Y 128.0F       /* 0x43000000 */
#define FTREMIXMAGICHAT_SPARK_SCALE 1.25F           /* 0x3FA00000 */
#define FTREMIXMAGICHAT_BOWSER_AMMO_WORD 0x0AE0

/* magic_kirby_hat_history: fighter id of the hat each port used last. */
static s32 sFTRemixMagicHatHistory[FTREMIXMAGICHAT_PORTS];

/* magic_kirby_hat_charge: charge each port stored for each hat row. */
static s8 sFTRemixMagicHatCharges[FTREMIXMAGICHAT_CHARGE_SLOTS][FTREMIXMAGICHAT_PORTS];

static sb32 ftRemixMagicHatIsKirby(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY);
}

static sb32 ftRemixMagicHatHasPort(FTStruct *fp)
{
    return (fp->player >= 0) && (fp->player < FTREMIXMAGICHAT_PORTS);
}

/* Sparkle over the head: a point 128 units above the hat joint, as the asm builds it. */
static void ftRemixMagicHatMakeSpark(FTStruct *fp)
{
    Vec3f pos = { 0.0F, FTREMIXMAGICHAT_SPARK_OFFSET_Y, 0.0F };

    if (fp->joints[FTREMIXMAGICHAT_HAT_JOINT] == NULL)
    {
        return;
    }
    gmCollisionGetFighterPartsWorldPosition(fp->joints[FTREMIXMAGICHAT_HAT_JOINT], &pos);
    efManagerSparkleWhiteScaleMakeEffect(&pos, FTREMIXMAGICHAT_SPARK_SCALE);
}

static void ftRemixMagicHatSwapHat(GObj *fighter_gobj, FTStruct *fp, s32 hat_id)
{
    ftParamSetModelPartDefaultID(fighter_gobj, FTREMIXMAGICHAT_HAT_JOINT, hat_id);
    ftParamResetModelPartAll(fighter_gobj);
    ftRemixMagicHatMakeSpark(fp);
}

/* _get_random_hat: rolls 1..41 over the hat table until it is not the magic entry. */
static s32 ftRemixMagicHatRollFighter(void)
{
    s32 fkind;

    do
    {
        fkind = ftRemixMagicHatFighterFromRoll(1 + syUtilsRandIntRange(FTREMIXMAGICHAT_ROLLS));

    } while (fkind == REMIX_ID_NONE);

    return fkind;
}

/* _store_hat_charge: remembers the charge of the hat used last. Returns TRUE when it is full, the stored charge is then cleared. */
static sb32 ftRemixMagicHatStoreCharge(FTStruct *fp, s32 last_fkind)
{
    s32 slot = ftRemixMagicHatChargeSlot(last_fkind);
    s32 charge;

    if (slot < 0)
    {
        return FALSE;
    }
    charge = *ftRemixFighterWord(fp, ftRemixMagicHatAmmoOffset(last_fkind));
    sFTRemixMagicHatCharges[slot][fp->player] = (s8)charge;

    if (charge != ftRemixMagicHatChargeMax(slot))
    {
        return FALSE;
    }
    sFTRemixMagicHatCharges[slot][fp->player] = 0;

    return TRUE;
}

/* _load_hat_charge: gives the new hat the charge stored for it. */
static void ftRemixMagicHatLoadCharge(FTStruct *fp, s32 fkind)
{
    s32 slot = ftRemixMagicHatChargeSlot(fkind);

    if (slot < 0)
    {
        return;
    }
    *ftRemixFighterWord(fp, ftRemixMagicHatAmmoOffset(fkind)) = sFTRemixMagicHatCharges[slot][fp->player];
}

/* _clear_ammo and _get_random_hat: a new random power starts with no charge. */
static s32 ftRemixMagicHatPickNewPower(FTStruct *fp)
{
    s32 fkind;

    *ftRemixFighterWord(fp, ftRemixMagicHatAmmoOffset(REMIX_ID_DONKEY)) = 0;
    *ftRemixFighterWord(fp, ftRemixMagicHatAmmoOffset(REMIX_ID_KIRBY)) = 0;

    fkind = ftRemixMagicHatRollFighter();
    /* The copied special reads the power owner's files (e.g. the boomerang reads Link's special file), which are only
     * loaded for fighters in the match; the roll can pick any fighter (`r3_ft29_magichat` crashed on Link's boomerang) */
    ftManagerSetupFilesAllKind(fkind);
    ftRemixMagicHatLoadCharge(fp, fkind);

    return fkind;
}

/* _continue, Bowser-specific: a Bowser hat starts with full fire ammo (the low halfword of the word at 0x0AE0). */
static void ftRemixMagicHatFillBowserAmmo(FTStruct *fp)
{
    s32 *word = ftRemixFighterWord(fp, FTREMIXMAGICHAT_BOWSER_AMMO_WORD);

    *word = (*word & ~FTREMIXMAGICHAT_HALFWORD_MASK) | FTREMIXMAGICHAT_BOWSER_AMMO;
}

void ftRemixKirbyMagicHatRoll(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 last_fkind;
    s32 fkind;

    if (!ftRemixMagicHatHasPort(fp) || !ftRemixKirbyMagicHatIsOn(fp) || (*ftRemixFighterKirbyHat(fp) == 0))
    {
        return;
    }
    last_fkind = sFTRemixMagicHatHistory[fp->player];

    if (ftRemixMagicHatStoreCharge(fp, last_fkind))
    {
        fkind = fp->passive_vars.kirby.copy_id;
    }
    else fkind = ftRemixMagicHatPickNewPower(fp);

    if (fkind == REMIX_ID_BOWSER)
    {
        ftRemixMagicHatFillBowserAmmo(fp);
    }
    sFTRemixMagicHatHistory[fp->player] = fkind;
    fp->passive_vars.kirby.copy_id = fkind;

    ftRemixMagicHatSwapHat(fighter_gobj, fp, ftRemixKirbyGetCopyTable()[fkind].copy_modelpart_id);
}

void ftRemixKirbyMagicHatRestore(GObj *fighter_gobj, FTStruct *fp, s32 status_id)
{
    if (!ftRemixMagicHatIsKirby(fp) || !ftRemixMagicHatHasPort(fp) || !ftRemixKirbyMagicHatIsOn(fp))
    {
        return;
    }
    if ((*ftRemixFighterKirbyHat(fp) == FTREMIXMAGICHAT_MAGICIAN_HAT) || !ftRemixMagicHatShouldRestore(status_id))
    {
        return;
    }
    ftRemixMagicHatSwapHat(fighter_gobj, fp, FTREMIXMAGICHAT_MAGICIAN_HAT);
}

void ftRemixKirbyMagicHatClearCharge(s32 player)
{
    s32 slot;

    if ((player < 0) || (player >= FTREMIXMAGICHAT_PORTS))
    {
        return;
    }
    for (slot = 0; slot < FTREMIXMAGICHAT_CHARGE_SLOTS; slot++)
    {
        sFTRemixMagicHatCharges[slot][player] = 0;
    }
}

#endif
