/**
 * ftremixswordtrail.c - Smash Remix SwordTrail.asm: custom sword trails. A custom trail id is stored in
 * afterimage.is_itemswing (player struct 0x0A9C) and is set by the vanilla SetAfterImage motion event, so
 * moveset commands 0xCC<id*4> already select it. Ids 0 and 1 stay vanilla (Link sword and item swing).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixswordtrail.h"
#include "gm/gmcollision.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_SWORD_TRAIL_FIRST_ID 2
#define FTREMIX_SWORD_TRAIL_AXIS_X 0
#define FTREMIX_SWORD_TRAIL_AXIS_Y 1
#define FTREMIX_SWORD_TRAIL_AXIS_Z 2
#define FTREMIX_SWORD_TRAIL_RGBA(rgba) { ((rgba) >> 24) & 0xFF, ((rgba) >> 16) & 0xFF, ((rgba) >> 8) & 0xFF, (rgba) & 0xFF }

/* SwordTrail.asm add_sword_trail entries in table order; entry i is trail id FIRST_ID + i. The asm name is in each comment. */
static const FTRemixSwordTrail dFTRemixSwordTrails[] =
{
    { REMIX_ID_MARTH, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* marth_default_trail */
    { REMIX_ID_CONKER, 0xD, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 150.0F, 800.0F },  /* conker_katana_trail */
    { REMIX_ID_MARTH, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0xFF000000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* marth_nsp_red_trail */
    { REMIX_ID_MARTH, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x0050FF00), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* marth_nsp_blue_trail */
    { REMIX_ID_MARTH, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x00FF0000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* marth_nsp_green_trail */
    { FTREMIX_SWORD_TRAIL_ANY_FKIND, 0xD, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0xFF000000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 50.0F, 300.0F },   /* kirby_marth_red_trail */
    { FTREMIX_SWORD_TRAIL_ANY_FKIND, 0xD, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x0050FF00), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 50.0F, 300.0F },   /* kirby_marth_blue_trail */
    { FTREMIX_SWORD_TRAIL_ANY_FKIND, 0xD, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x00FF0000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 50.0F, 300.0F },   /* kirby_marth_green_trail */
    { FTREMIX_SWORD_TRAIL_ANY_FKIND, 0x1A, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x00D0FF00), FTREMIX_SWORD_TRAIL_RGBA(0x0090FF00), -190.0F, 190.0F },  /* sonic_trail */
    { FTREMIX_SWORD_TRAIL_ANY_FKIND, 0x1A, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0xEEFF5F00), FTREMIX_SWORD_TRAIL_RGBA(0xE4D72600), -190.0F, 190.0F },  /* ssonic_trail */
    { REMIX_ID_DEDEDE, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0xEEEEEE00), FTREMIX_SWORD_TRAIL_RGBA(0xB68E5600), -576.0F, -192.0F }, /* dedede_hammer */
    { REMIX_ID_GOEMON, 0xD, FTREMIX_SWORD_TRAIL_AXIS_Y, FTREMIX_SWORD_TRAIL_RGBA(0xE0B00000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), -64.0F, 220.0F },   /* goemon_trail */
    { REMIX_ID_FALCO, 0x0, FTREMIX_SWORD_TRAIL_AXIS_Y, FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), -150.0F, 250.0F },   /* falco_trail */
    { REMIX_ID_FALCO, 0x1, FTREMIX_SWORD_TRAIL_AXIS_Y, FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), -150.0F, 250.0F },   /* falco_trail2 */
    { REMIX_ID_KIRBY, 0x0, FTREMIX_SWORD_TRAIL_AXIS_Y, FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), FTREMIX_SWORD_TRAIL_RGBA(0x00FFFF00), -140.0F, 240.0F },   /* kirby_falco_trail */
    { REMIX_ID_ROY, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0xFF440000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* roy_default_trail */
    { REMIX_ID_ROY, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0xFF000000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* roy_nsp_red_trail */
    { REMIX_ID_ROY, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x0050FF00), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* roy_nsp_blue_trail */
    { REMIX_ID_ROY, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0x00FF0000), FTREMIX_SWORD_TRAIL_RGBA(0xFFFFFF00), 70.0F, 370.0F },   /* roy_nsp_green_trail */
    { REMIX_ID_ROY, 0xE, FTREMIX_SWORD_TRAIL_AXIS_Z, FTREMIX_SWORD_TRAIL_RGBA(0xFF000000), FTREMIX_SWORD_TRAIL_RGBA(0xE06C0000), 50.0F, 380.0F },   /* roy_dsp_trail */
    { REMIX_ID_DSAMUS, 0x0, FTREMIX_SWORD_TRAIL_AXIS_Y, FTREMIX_SWORD_TRAIL_RGBA(0x40C0FF00), FTREMIX_SWORD_TRAIL_RGBA(0x80E0FF00), 0.0F, 180.0F }    /* dsamus_trail */
};

#define FTREMIX_SWORD_TRAIL_COUNT ARRAY_COUNT(dFTRemixSwordTrails)

/* Ring of afterimage.desc entries, the last index is the one before the wrap. */
#define FTREMIX_SWORD_TRAIL_LAST_DESC_INDEX (ARRAY_COUNT(((FTStruct *)0)->afterimage.desc) - 1)

/**
 * Trail entry for a custom trail id, or NULL for the vanilla ids (0 and 1) and ids past the table.
 */
static const FTRemixSwordTrail *ftRemixSwordTrailFromId(s32 trail_id)
{
    s32 index = trail_id - FTREMIX_SWORD_TRAIL_FIRST_ID;

    if ((index < 0) || (index >= FTREMIX_SWORD_TRAIL_COUNT))
    {
        return NULL;
    }
    return &dFTRemixSwordTrails[index];
}

/**
 * Trail entry a fighter records this frame. The id must be custom and the fighter must match the trail's fkind,
 * or the trail allows any fighter. Returns NULL when the fighter does not record.
 */
static const FTRemixSwordTrail *ftRemixSwordTrailForFighter(FTStruct *fp)
{
    const FTRemixSwordTrail *trail = ftRemixSwordTrailFromId(fp->afterimage.is_itemswing);

    if (trail == NULL)
    {
        return NULL;
    }
    if ((trail->fkind != FTREMIX_SWORD_TRAIL_ANY_FKIND) && (trail->fkind != fp->fkind))
    {
        return NULL;
    }
    return trail;
}

/**
 * Writes the current bone matrix into the next afterimage entry. The position is the translation column and the
 * direction is row `axis` of the matrix, as the vanilla record does for axis Z.
 */
static void ftRemixSwordTrailStoreDesc(FTStruct *fp, FTParts *parts, s32 axis)
{
    FTAfterImage *desc = &fp->afterimage.desc[fp->afterimage.desc_id];

    desc->translate_x = parts->mtx_translate[3][0];
    desc->translate_y = parts->mtx_translate[3][1];
    desc->translate_z = parts->mtx_translate[3][2];
    desc->vec.x = parts->mtx_translate[axis][0];
    desc->vec.y = parts->mtx_translate[axis][1];
    desc->vec.z = parts->mtx_translate[axis][2];
}

/**
 * Moves the ring to the next entry and raises drawstatus up to the ring size, as the vanilla record does.
 */
static void ftRemixSwordTrailAdvanceDesc(FTStruct *fp)
{
    if (fp->afterimage.desc_id == FTREMIX_SWORD_TRAIL_LAST_DESC_INDEX)
    {
        fp->afterimage.desc_id = 0;
    }
    else
    {
        fp->afterimage.desc_id++;
    }
    if (fp->afterimage.drawstatus <= FTREMIX_SWORD_TRAIL_LAST_DESC_INDEX)
    {
        fp->afterimage.drawstatus++;
    }
}

/**
 * Whether a custom trail id exists in the table. The vanilla draw check only tests the id, not the fighter.
 *
 * @param trail_id afterimage.is_itemswing of the fighter.
 * @return TRUE when the id selects a custom trail.
 */
sb32 ftRemixSwordTrailIsCustomId(s32 trail_id)
{
    return ftRemixSwordTrailFromId(trail_id) != NULL;
}

/**
 * Fills the draw values of a custom trail: distances along the direction and both colours.
 *
 * @param trail_id afterimage.is_itemswing of the fighter.
 * @param draw Receives the values; untouched when the id is not custom.
 * @return TRUE when draw was filled.
 */
sb32 ftRemixSwordTrailGetDraw(s32 trail_id, FTRemixSwordTrailDraw *draw)
{
    const FTRemixSwordTrail *trail = ftRemixSwordTrailFromId(trail_id);

    if (trail == NULL)
    {
        return FALSE;
    }
    draw->start_dist = trail->start_pos;
    draw->end_dist = trail->end_pos;
    draw->colour_1 = trail->colour_1;
    draw->colour_2 = trail->colour_2;
    return TRUE;
}

/**
 * Records one afterimage entry for a custom trail, from the joint of model part (model_part + nFTPartsJointCommonStart).
 * The asm skips the vanilla Link sword check for custom trails, so this does not test the model part visibility.
 *
 * @param fp Fighter whose afterimage is recorded.
 */
void ftRemixSwordTrailRecord(FTStruct *fp)
{
    const FTRemixSwordTrail *trail = ftRemixSwordTrailForFighter(fp);
    DObj *joint;
    FTParts *parts;

    if (trail == NULL)
    {
        return;
    }
    joint = fp->joints[trail->model_part + nFTPartsJointCommonStart];
    if ((joint == NULL) || (joint->user_data.p == NULL))
    {
        return;
    }
    func_ovl2_800EDBA4(joint);

    parts = joint->user_data.p;
    ftRemixSwordTrailStoreDesc(fp, parts, trail->axis);
    ftRemixSwordTrailAdvanceDesc(fp);
}

#endif
