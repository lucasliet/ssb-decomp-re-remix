/**
 * sc1premixhrcsigns.c - digit images of the Home-Run Contest distance signs, see sc1premixhrcsigns.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <gr/ground.h>
#include <sys/objdef.h>
#include <sys/objtypes.h>
#include "sc/sc1pmode/sc1premixhrcsignrules.h"
#include "sc/sc1pmode/sc1premixhrcsigns.h"

#define SC1PREMIX_HRC_SIGNS_LAYER 1
#define SC1PREMIX_HRC_SIGNS_SKIP_TO_SECOND 3
#define SC1PREMIX_HRC_SIGNS_SKIP_TO_NEXT 2
#define SC1PREMIX_HRC_SIGNS_COUNT 3

/** The three digit images of a sign, in draw order (HRC.asm initialize_signs_). */
typedef struct SC1PRemixHrcSignImages
{
    MObj *hundred_pair;
    MObj *hundreds;
    MObj *thousands;

} SC1PRemixHrcSignImages;

static SC1PRemixHrcSignImages sSC1PRemixHrcFarthestImages;
static SC1PRemixHrcSign sSC1PRemixHrcFarthestSign;

static DObj* sc1PRemixHrcSignsSkip(DObj *dobj, s32 count)
{
    while ((dobj != NULL) && (count-- > 0))
    {
        dobj = dobj->sib_next;
    }
    return dobj;
}

static sb32 sc1PRemixHrcSignsGetImages(DObj *sign_dobj, SC1PRemixHrcSignImages *images)
{
    if ((sign_dobj == NULL) || (sign_dobj->mobj == NULL) || (sign_dobj->mobj->next == NULL) || (sign_dobj->mobj->next->next == NULL))
    {
        return FALSE;
    }
    images->hundred_pair = sign_dobj->mobj;
    images->hundreds = images->hundred_pair->next;
    images->thousands = images->hundreds->next;
    return TRUE;
}

static void sc1PRemixHrcSignsSetImage(MObj *mobj, s32 index)
{
    mobj->texture_id_curr = index;
    mobj->anim_wait = AOBJ_ANIM_NULL;
}

static void sc1PRemixHrcSignsShow(const SC1PRemixHrcSignImages *images, const SC1PRemixHrcSign *sign)
{
    sc1PRemixHrcSignsSetImage(images->hundred_pair, sign->hundred_pair);
    sc1PRemixHrcSignsSetImage(images->hundreds, sign->hundreds);
    sc1PRemixHrcSignsSetImage(images->thousands, sign->thousands);
}

static sb32 sc1PRemixHrcSignsSetHundreds(DObj *sign_dobj, s32 hundreds, SC1PRemixHrcSignImages *images)
{
    SC1PRemixHrcSign sign;

    if (sc1PRemixHrcSignsGetImages(sign_dobj, images) == FALSE)
    {
        return FALSE;
    }
    sc1PRemixHrcSignReset(&sign);
    sign.hundreds = hundreds;
    sc1PRemixHrcSignsShow(images, &sign);
    return TRUE;
}

void sc1PRemixHrcSignsInit(void)
{
    DObj *sign_dobj;
    SC1PRemixHrcSignImages images;
    s32 hundreds;

    if (gGRCommonLayerGObjs[SC1PREMIX_HRC_SIGNS_LAYER] == NULL)
    {
        return;
    }
    sign_dobj = sc1PRemixHrcSignsSkip(DObjGetStruct(gGRCommonLayerGObjs[SC1PREMIX_HRC_SIGNS_LAYER])->child, SC1PREMIX_HRC_SIGNS_SKIP_TO_SECOND);

    for (hundreds = 1; hundreds <= SC1PREMIX_HRC_SIGNS_COUNT; hundreds++)
    {
        if (sc1PRemixHrcSignsSetHundreds(sign_dobj, hundreds, &images) == FALSE)
        {
            return;
        }
        sign_dobj = sc1PRemixHrcSignsSkip(sign_dobj, SC1PREMIX_HRC_SIGNS_SKIP_TO_NEXT);
    }
    sSC1PRemixHrcFarthestImages = images;
    sc1PRemixHrcSignReset(&sSC1PRemixHrcFarthestSign);
}

void sc1PRemixHrcSignsAdvance(void)
{
    if (sSC1PRemixHrcFarthestImages.hundred_pair == NULL)
    {
        return;
    }
    sc1PRemixHrcSignAdvance(&sSC1PRemixHrcFarthestSign);
    sc1PRemixHrcSignsShow(&sSC1PRemixHrcFarthestImages, &sSC1PRemixHrcFarthestSign);
}

#endif
