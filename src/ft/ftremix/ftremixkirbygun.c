/**
 * ftremixkirbygun.c - Kirby held-weapon parts (KirbyHats.asm:112 use_extended_special_parts_pause_, gun branch), see
 * ftremixkirbygunrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixkirbygunrules.h"
#include "ft/ftremix/ftremixparamhooks.h"

static sb32 ftRemixKirbyGunIsKirby(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY);
}

/**
 * Model part descriptor of a joint of Kirby in ftParamSetModelPartID. The held-weapon joint of a Kirby with the hat of a fighter
 * that holds a weapon reads the special part struct of Kirby's model file instead of the container descriptor; every other
 * joint and fighter keeps the container descriptor.
 *
 * @param fp Fighter whose model part is set.
 * @param joint_id Joint of the model part.
 * @param container_desc Descriptor of the model part container for that joint (may be NULL).
 * @return Descriptor to read the model part from.
 */
FTModelPartDesc* ftRemixKirbyGunPartDesc(FTStruct *fp, s32 joint_id, FTModelPartDesc *container_desc)
{
    s32 file_offset;

    if ((joint_id != FTREMIX_KIRBY_GUN_JOINT) || !ftRemixKirbyGunIsKirby(fp))
    {
        return container_desc;
    }
    file_offset = ftRemixKirbyGunFileOffset(*ftRemixFighterKirbyHat(fp));

    if ((file_offset == FTREMIX_KIRBY_GUN_NO_OFFSET) || (fp->data->p_file_model == NULL) || (*fp->data->p_file_model == NULL))
    {
        return container_desc;
    }
    return (FTModelPartDesc*)((u8*)*fp->data->p_file_model + file_offset);
}

#endif
