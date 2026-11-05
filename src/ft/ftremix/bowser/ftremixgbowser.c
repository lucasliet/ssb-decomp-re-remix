/**
 * ftremixgbowser.c - Giga Bowser, the final boss of the Remix 1P mode (src/GBowser/gbowser.asm
 * _remix_1p_prevent_bottom_blast_zone_KO).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/bowser/ftremixbowserhooks.h"
#include <ft/fighter.h>
#include <mp/mpcommon.h>
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_GBOWSER_BOUNCE_MIN_Y (-6500.0F) /* lui 0xC5CA */
#define FTREMIX_GBOWSER_BOUNCE_VEL_Y 256.0F     /* lui 0x4380 */
#define FTREMIX_GBOWSER_BOUNCE_X 4592.0F        /* lui 0x458F */
#define FTREMIX_GBOWSER_BOUNCE_FGM 0x369

extern void *func_800269C0_275C0(u16 id);

/* Sends the fighter back up, on the side of the stage he fell from, and back to wait or fall. */
static void ftRemixGBowserBounceUp(GObj *fighter_gobj, FTStruct *fp, Vec3f *pos)
{
    fp->physics.vel_air.y = FTREMIX_GBOWSER_BOUNCE_VEL_Y;
    fp->is_fastfall = FALSE;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;

    if (pos->x < 0.0F)
    {
        pos->x = -FTREMIX_GBOWSER_BOUNCE_X;
        fp->lr = +1;
    }
    else
    {
        pos->x = FTREMIX_GBOWSER_BOUNCE_X;
        fp->lr = -1;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
    func_800269C0_275C0(FTREMIX_GBOWSER_BOUNCE_FGM);
}

sb32 ftRemixGBowserSavedFromBottomBlastZone(GObj *fighter_gobj, Vec3f *pos)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((fp->fkind != REMIX_ID_GBOWSER) || (sc1PRemixModeIs(SC1PREMIX_MODE_REMIX_1P) == FALSE) || (fp->pkind == nFTPlayerKindMan))
    {
        return FALSE;
    }
    if ((fp->physics.vel_air.y < 0.0F) && (pos->y <= FTREMIX_GBOWSER_BOUNCE_MIN_Y))
    {
        ftRemixGBowserBounceUp(fighter_gobj, fp, pos);
    }
    return TRUE;
}

#endif
