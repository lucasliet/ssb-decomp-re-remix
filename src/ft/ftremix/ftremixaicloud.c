/*
 * ftremixaicloud.c - Magic Cloud hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm usp_check_ and
 * recovery_stick_y_fix). Pure action rules live in ftremixaicloudrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixaicloud.h"
#include "ft/ftremix/ftremixaicloudrules.h"

/**
 * usp_check_ (AI.asm): Fox, JFox, Peppy and Falco share the Fox USP branch of the air check.
 */
sb32 ftRemixAiUspIsFoxFamily(FTStruct *fp)
{
    return (fp->fkind == nFTKindFox) || (fp->fkind == REMIX_ID_JFOX) || (fp->fkind == REMIX_ID_PEPPY) ||
           (fp->fkind == REMIX_ID_FALCO);
}

/**
 * usp_check_ (AI.asm): Goemon and Ebisumaru in a Magic Cloud action go to their USP branch, which skips the vanilla
 * stick Y zeroing of the air check.
 */
sb32 ftRemixAiUspIsCloudAction(FTStruct *fp)
{
    if (fp->fkind == REMIX_ID_GOEMON)
    {
        return ftRemixAiCloudIsGoemonUspAction(fp->status_id);
    }
    if (fp->fkind == REMIX_ID_EBI)
    {
        return ftRemixAiCloudIsEbisumaruUspAction(fp->status_id);
    }
    return FALSE;
}

/**
 * Ebisumaru Magic Cloud Ride: stick up while recovering. Once above a floor line the stick is released and A is pressed
 * to start the nair. coll_data.floor_line_id is -1 when the fighter is not above a floor line (asm 0xFFFF word).
 */
static void ftRemixAiCloudEbisumaruInput(FTStruct *fp)
{
    if (!ftRemixAiCloudIsEbisumaruUspAction(fp->status_id))
    {
        return;
    }
    fp->input.cp.stick_range.y = FTREMIX_AI_CLOUD_STICK_UP;
    if (fp->coll_data.floor_line_id == -1)
    {
        return;
    }
    fp->input.cp.stick_range.y = 0;
    fp->input.cp.button_inputs =
        ftRemixAiCloudReplaceButtonHighByte(fp->input.cp.button_inputs, FTREMIX_AI_CLOUD_BUTTON_A);
}

/**
 * Goemon Magic Cloud Ride and Attack: stick up, and Z once above a floor line. Magic Cloud Escape clears the Z press.
 */
static void ftRemixAiCloudGoemonInput(FTStruct *fp)
{
    if (ftRemixAiCloudIsGoemonStickUpAction(fp->status_id))
    {
        fp->input.cp.stick_range.y = FTREMIX_AI_CLOUD_STICK_UP;
        if (fp->coll_data.floor_line_id != -1)
        {
            fp->input.cp.button_inputs =
                ftRemixAiCloudReplaceButtonHighByte(fp->input.cp.button_inputs, FTREMIX_AI_CLOUD_BUTTON_Z);
        }
        return;
    }
    if (fp->status_id == FTREMIX_AI_CLOUD_ACTION_ESCAPE)
    {
        fp->input.cp.button_inputs = ftRemixAiCloudReplaceButtonHighByte(fp->input.cp.button_inputs, 0);
    }
}

/**
 * recovery_stick_y_fix (AI.asm): runs after the command script of each frame, for Goemon and Ebisumaru CPUs only.
 */
void ftRemixAiCloudRecoveryInput(FTStruct *fp)
{
    if (fp->fkind == REMIX_ID_EBI)
    {
        ftRemixAiCloudEbisumaruInput(fp);
    }
    else if (fp->fkind == REMIX_ID_GOEMON)
    {
        ftRemixAiCloudGoemonInput(fp);
    }
}

#endif
