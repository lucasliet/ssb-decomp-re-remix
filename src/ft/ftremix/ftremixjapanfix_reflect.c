/**
 * ftremixjapanfix_reflect.c - Smash Remix Japanese.asm reflect_: a weapon reflected by a J fighter takes the Japanese
 * reflect damage multiplier (1.5F) instead of the US one (WEAPON_REFLECT_MUL_DEFAULT, 1.8F, REGION_US is also set in the Remix build).
 *
 * The asm has no toggle. It reads the sound type of the reflecting owner and replaces the multiplier that the
 * reflect damage update uses (wpprocess.c, wpProcessProcHitCollisions).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixjapanfix_reflect.h"
#include <ft/fighter.h>
#include <sys/objdef.h>
#include "remix/remix_roster_data.h"

/* Japan.asm J_MULTIPLIER (0x3FC0 as the upper halfword, 1.5F) */
#define FTREMIX_JAPANFIX_REFLECT_J_MUL 1.5F

/**
 * Returns the reflect damage multiplier of a weapon reflected by owner_gobj. vanilla_mul is returned unchanged unless
 * the owner is a fighter with the J sound type. Weapons with a non-fighter owner keep vanilla_mul.
 */
f32 ftRemixJapanFixReflectMul(GObj *owner_gobj, f32 vanilla_mul)
{
    FTStruct *owner_fp;

    if ((owner_gobj == NULL) || (owner_gobj->link_id != nGCCommonLinkIDFighter))
    {
        return vanilla_mul;
    }
    owner_fp = ftGetStruct(owner_gobj);
    if ((owner_fp->fkind < 0) || (owner_fp->fkind >= ARRAY_COUNT(kRemixRoster)))
    {
        return vanilla_mul;
    }
    if (kRemixRoster[owner_fp->fkind].sound_type == REMIX_SOUND_J)
    {
        return FTREMIX_JAPANFIX_REFLECT_J_MUL;
    }
    return vanilla_mul;
}

#endif
