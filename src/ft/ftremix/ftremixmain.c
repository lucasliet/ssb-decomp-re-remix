/**
 * ftremixmain.c - Remix hooks of ftMainProcPhysicsMap and ftMainSetStatus, see ftremixmain.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixmain.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixmainrules.h"
#include "remix/remix_roster_data.h"

/* Action.ThrownDK (Action.asm:189). */
#define FTREMIXMAIN_STATUS_THROWN_DK 0xB8

u32 ftRemixMainGetAnimFileID(FTStruct *fp, u32 anim_file_id)
{
    FTStruct *grabber_fp;
    s32 thrown_file;

    if ((fp->status_id != FTREMIXMAIN_STATUS_THROWN_DK) || (fp->capture_gobj == NULL))
    {
        return anim_file_id;
    }
    grabber_fp = ftGetStruct(fp->capture_gobj);

    if (grabber_fp->fkind != REMIX_ID_MARINA)
    {
        return anim_file_id;
    }
    thrown_file = ftRemixMainMarinaThrownFile(fp->fkind);

    return (thrown_file != 0) ? (u32)thrown_file : anim_file_id;
}

#endif
