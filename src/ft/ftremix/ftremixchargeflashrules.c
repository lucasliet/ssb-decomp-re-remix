/**
 * ftremixchargeflashrules.c - pure rule of the Remix gfx_routine_end table, see ftremixchargeflashrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixchargeflashrules.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_CHARGE_WORD_LEVEL 0x0ADC
#define FTREMIX_CHARGE_WORD_RECOIL 0x0AE0
#define FTREMIX_CHARGE_WORD_DEDEDE 0x0AE4

#define FTREMIX_SAMUS_CHARGE_MAX 7              /* FTSAMUS_CHARGE_MAX */
#define FTREMIX_DONKEY_CHARGE_MAX 10            /* FTDONKEY_GIANTPUNCH_CHARGE_MAX */

/* GFXRoutine ids from build/remixasm/bass.log */
#define FTREMIX_GFX_MEWTWO_CHARGE 0x58
#define FTREMIX_GFX_SHEIK_CHARGE 0x6D
#define FTREMIX_GFX_MARINA_CHARGE 0x72
#define FTREMIX_GFX_DEDEDE_CHARGE 0x74

static const FTRemixChargeFlash sSamusFlash = { FTREMIX_CHARGE_WORD_LEVEL, FTREMIX_SAMUS_CHARGE_MAX, FTREMIX_CHARGE_FLASH_COMMON };
static const FTRemixChargeFlash sDonkeyFlash = { FTREMIX_CHARGE_WORD_LEVEL, FTREMIX_DONKEY_CHARGE_MAX, FTREMIX_CHARGE_FLASH_COMMON };
static const FTRemixChargeFlash sMewtwoFlash = { FTREMIX_CHARGE_WORD_LEVEL, 7, FTREMIX_GFX_MEWTWO_CHARGE };     /* Mewtwo.asm:547 */
static const FTRemixChargeFlash sMarinaFlash = { FTREMIX_CHARGE_WORD_LEVEL, 4, FTREMIX_GFX_MARINA_CHARGE };     /* Marina.asm:701 */
static const FTRemixChargeFlash sSheikFlash = { FTREMIX_CHARGE_WORD_RECOIL, 6, FTREMIX_GFX_SHEIK_CHARGE };      /* Sheik.asm:566 */
static const FTRemixChargeFlash sPeppyFlash = { FTREMIX_CHARGE_WORD_RECOIL, 5, FTREMIX_GFX_SHEIK_CHARGE };      /* Peppy.asm:313, uses SHEIK_CHARGE */
static const FTRemixChargeFlash sDededeFlash = { FTREMIX_CHARGE_WORD_DEDEDE, 2, FTREMIX_GFX_DEDEDE_CHARGE };    /* Dedede.asm:708 */

static const FTRemixChargeFlash *ftRemixChargeFlashRow(int fkind)
{
    switch (fkind)
    {
    case REMIX_ID_DSAMUS:   /* inherited from SAMUS */
    case REMIX_ID_JSAMUS:
    case REMIX_ID_ESAMUS:
    case REMIX_ID_NDSAMUS:
        return &sSamusFlash;

    case REMIX_ID_JDK:      /* inherited from DONKEY */
        return &sDonkeyFlash;

    case REMIX_ID_MTWO:
        return &sMewtwoFlash;

    case REMIX_ID_MARINA:
        return &sMarinaFlash;

    case REMIX_ID_SHEIK:
        return &sSheikFlash;

    case REMIX_ID_PEPPY:
        return &sPeppyFlash;

    case REMIX_ID_DEDEDE:
        return &sDededeFlash;

    default:
        return 0;
    }
}

int ftRemixChargeFlashFind(int fkind, FTRemixChargeFlash *flash)
{
    const FTRemixChargeFlash *row = ftRemixChargeFlashRow(fkind);

    if (row == 0)
    {
        return 0;
    }
    *flash = *row;
    return 1;
}

#endif
