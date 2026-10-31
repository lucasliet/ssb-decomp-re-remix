#ifndef _FTREMIXSWORDTRAIL_H_
#define _FTREMIXSWORDTRAIL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* One custom sword trail from SwordTrail.asm add_sword_trail. fkind is a Remix id, or FTREMIX_SWORD_TRAIL_ANY_FKIND. */
typedef struct FTRemixSwordTrail
{
    s32 fkind;
    u8 model_part;
    u8 axis;
    SYColorRGBA colour_1;
    SYColorRGBA colour_2;
    f32 start_pos;
    f32 end_pos;
} FTRemixSwordTrail;

/* Values the afterimage draw reads from a custom trail, in place of the vanilla Link or item swing constants. */
typedef struct FTRemixSwordTrailDraw
{
    f32 start_dist;
    f32 end_dist;
    SYColorRGBA colour_1;
    SYColorRGBA colour_2;
} FTRemixSwordTrailDraw;

#define FTREMIX_SWORD_TRAIL_ANY_FKIND (-1)

/* ftremixswordtrail.c: custom trail ids are afterimage.is_itemswing values 2 and up (ids 0 and 1 are vanilla). */
sb32 ftRemixSwordTrailIsCustomId(s32 trail_id);
sb32 ftRemixSwordTrailGetDraw(s32 trail_id, FTRemixSwordTrailDraw *draw);
void ftRemixSwordTrailRecord(FTStruct *fp);

#endif

#endif
