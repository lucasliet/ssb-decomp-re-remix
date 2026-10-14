/**
 * grremixmmm.c - Mad Monster Mansion thank-you routine. See grremixmmm.h.
 */

#ifdef PORT

#include "grremixmmm.h"
#include <gr/ground.h>
#include <ft/fighter.h>
#include <gm/gmdef.h>

/* Action.Taunt (Action.asm:194). */
#define GRREMIX_MMM_ACTION_TAUNT 0xBD
/* The asm reads FTStruct 0xEC and calls it the clipping id; in this port that field is coll_data.floor_line_id. */
#define GRREMIX_MMM_FLOOR_LINE_ID 2
/* asm uses 0x44D40000 (1696.0F, the comment says "1700ish") and 0x45020000 (2080.0F). */
#define GRREMIX_MMM_X_MIN 1696.0F
#define GRREMIX_MMM_X_MAX 2080.0F
#define GRREMIX_MMM_THANK_YOU_FGM 0x564

extern void* func_800269C0_275C0(u16 id);

static sb32 sGRRemixMmmThankYouDone;

/** Returns the fighter struct of the controller port, or NULL when no fighter is on that port. */
static FTStruct* grRemixMmmGetPortStruct(s32 port)
{
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

    while (fighter_gobj != NULL)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        if (fp->player == port)
        {
            return fp;
        }
        fighter_gobj = fighter_gobj->link_next;
    }
    return NULL;
}

/** TRUE when the fighter is taunting while standing on the marked floor line. */
static sb32 grRemixMmmIsTauntingOnLine(FTStruct *fp)
{
    return (fp->status_id == GRREMIX_MMM_ACTION_TAUNT) && (fp->coll_data.floor_line_id == GRREMIX_MMM_FLOOR_LINE_ID);
}

/** TRUE when the fighter's x coordinate lies in the thank-you band (inclusive on both ends). */
static sb32 grRemixMmmIsInThankYouBand(FTStruct *fp)
{
    f32 x = DObjGetStruct(fp->fighter_gobj)->translate.vec.f.x;

    return (GRREMIX_MMM_X_MIN <= x) && (x <= GRREMIX_MMM_X_MAX);
}

/** Per-frame routine (asm: mmm_main). Plays the thank-you sound once, then does nothing for the rest of the match. */
static void grRemixMmmProcUpdate(GObj *routine_gobj)
{
    s32 port;

    if (sGRRemixMmmThankYouDone)
    {
        return;
    }
    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        FTStruct *fp = grRemixMmmGetPortStruct(port);

        if ((fp == NULL) || !grRemixMmmIsTauntingOnLine(fp) || !grRemixMmmIsInThankYouBand(fp))
        {
            continue;
        }
        func_800269C0_275C0(GRREMIX_MMM_THANK_YOU_FGM);
        sGRRemixMmmThankYouDone = TRUE;
        return;
    }
}

/** Clears the thank-you flag and creates the routine object (asm: mmm_setup). */
void grRemixMmmMakeGround(void)
{
    GObj *routine_gobj;

    sGRRemixMmmThankYouDone = FALSE;
    routine_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(routine_gobj, grRemixMmmProcUpdate, nGCProcessKindFunc, 3);
}

#endif
