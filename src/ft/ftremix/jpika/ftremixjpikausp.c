/**
 * ftremixjpikausp.c - C ports of Smash Remix J Pikachu (JPika) up special routines
 * (.toolchain/pkgs/smashremix-src/src/JPika/JPikaSpecial.asm).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include "mp/mpcommon.h"
#include "ft/ftchar/ftpikachu/ftpikachufunctions.h"

/* JPikaUSP.JPika_SpecialHiProcMap_: collision map proc, the J version of ftPikachuSpecialHiProcMap
 * (0x80152B6C). The walls check after the floor branch runs on both paths, as in the asm. */
static void ftRemixJPikaUSPSpecialHiProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (mpCommonCheckFighterOnFloor(fighter_gobj) == FALSE)
    {
        if (fp->coll_data.mask_curr & (MAP_FLAG_RWALL | MAP_FLAG_LWALL))
        {
            mpCommonSetFighterAir(fp);
            ftPikachuSpecialAirHiEndSetStatus(fighter_gobj);
        }
        else ftPikachuSpecialHiSwitchStatusAir(fighter_gobj);
    }
    if (fp->coll_data.mask_curr & (MAP_FLAG_RWALL | MAP_FLAG_LWALL))
    {
        ftPikachuSpecialHiEndSetStatus(fighter_gobj);
    }
}

/* Remix address -> port routine for every JPika USP routine its action array or special tables reference. */
const FTRemixProc dFTRemixJPikaUSPProcs[] =
{
    { REMIX_SYM_JPikaUSP__JPika_SpecialHiProcMap_, ftRemixJPikaUSPSpecialHiProcMap },
};

const s32 dFTRemixJPikaUSPProcsNum = ARRAY_COUNT(dFTRemixJPikaUSPProcs);

#endif
