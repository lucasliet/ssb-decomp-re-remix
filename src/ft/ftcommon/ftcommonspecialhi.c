#include <ft/fighter.h>
#ifdef REGION_REMIX
#include "mn/mnvsmode/mnremixvsstats.h"
#include "sc/sc1pmode/sc1premix1p.h"
#endif
#ifdef PORT
extern void port_log(const char *fmt, ...);
#include "fighter_registry.h"
#endif

// // // // // // // // // // // //
//                               //
//       INITIALIZED DATA        //
//                               //
// // // // // // // // // // // //

// 0x80188CE0
void (*dFTCommonSpecialHiStatusList[/* */])(GObj*) =
{
    ftMarioSpecialHiSetStatus,
    ftFoxSpecialHiStartSetStatus,
    ftDonkeySpecialHiSetStatus,
    ftSamusSpecialHiSetStatus,
    ftMarioSpecialHiSetStatus,
    ftLinkSpecialHiSetStatus,
    ftYoshiSpecialHiSetStatus,
    ftCaptainSpecialHiSetStatus,
    ftKirbySpecialHiSetStatus,
    ftPikachuSpecialHiStartSetStatus,
    ftPurinSpecialHiSetStatus,
    ftNessSpecialHiStartSetStatus,
    ftMarioSpecialHiSetStatus,
    ftMarioSpecialHiSetStatus,
    ftMarioSpecialHiSetStatus,
    ftFoxSpecialHiStartSetStatus,
    ftDonkeySpecialHiSetStatus,
    ftSamusSpecialHiSetStatus,
    ftMarioSpecialHiSetStatus,
    ftLinkSpecialHiSetStatus,
    ftYoshiSpecialHiSetStatus,
    ftCaptainSpecialHiSetStatus,
    ftKirbySpecialHiSetStatus,
    ftPikachuSpecialHiStartSetStatus,
    ftPurinSpecialHiSetStatus,
    ftNessSpecialHiStartSetStatus,
    ftDonkeySpecialHiSetStatus
};

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80151160
sb32 ftCommonSpecialHiCheckInterruptCommon(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

#ifdef PORT
    if (fp->input.pl.button_tap & fp->input.button_mask_b)
    {
        port_log("SSB64: SpecialHiCheck fkind=%d B_tap=0x%04X have_hi=%d stick_y=%d (need>=%d)\n",
            fp->fkind, fp->input.pl.button_tap & fp->input.button_mask_b,
            attr->is_have_specialhi, fp->input.pl.stick_range.y,
            FTCOMMON_SPECIALHI_STICK_RANGE_MIN);
    }
#endif
    if ((fp->input.pl.button_tap & fp->input.button_mask_b) && (attr->is_have_specialhi) && (fp->input.pl.stick_range.y >= FTCOMMON_SPECIALHI_STICK_RANGE_MIN))
    {
#ifdef REGION_REMIX
        if (sc1PRemix1PBlocksCpuUpSpecial(fp->fkind, (fp->pkind != nFTPlayerKindMan) ? TRUE : FALSE) != FALSE)
        {
            return FALSE;
        }
        mnRemixVSStatsCountSpecial(fp->player, nMNRemixVSStatsSpecialUp);
#endif
#ifdef PORT
        {
            PortFTSpecialEnterFn h = port_fighter_special_handler(fp->fkind, PORT_FIGHTER_SPECIAL_HI);
            if (h != NULL) h(fighter_gobj);
        }
#else
        dFTCommonSpecialHiStatusList[fp->fkind](fighter_gobj);
#endif

        return TRUE;
    }
    else return FALSE;
}
