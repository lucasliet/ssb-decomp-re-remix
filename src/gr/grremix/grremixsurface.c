/**
 * grremixsurface.c - Remix surface frictions for the vanilla ground physics. See ft/ftremix/ftremixhooks.h.
 */

#ifdef PORT

#include <ssb_types.h>
#include <mp/map.h>

#define GRREMIX_SURFACE_VANILLA_COUNT 16
#define GRREMIX_SURFACE_FRICTION_DEFAULT 4.0F

/* Surface.asm add_surface friction values, in the order of the new surface ids 0x10..0x24. */
static const f32 sGRRemixSurfaceFrictions[] =
{
    4.0F, /* 0x10 big_blue_surface_1 */
    0.3F, /* 0x11 cool_cool_surface_1 */
    4.0F, /* 0x12 failed_z_cancel */
    4.0F, /* 0x13 big_blue_surface_2 */
    4.0F, /* 0x14 corneria_surface_1 */
    4.0F, /* 0x15 corneria_surface_2 */
    4.0F, /* 0x16 mute */
    4.0F, /* 0x17 casino_left_diagonal */
    4.0F, /* 0x18 casino_left_top */
    4.0F, /* 0x19 casino_left_side */
    4.0F, /* 0x1A casino_right_diagonal */
    4.0F, /* 0x1B casino_right_top */
    4.0F, /* 0x1C casino_right_side */
    4.0F, /* 0x1D toadsturnpike_car */
    4.0F, /* 0x1E failed_z_cancel_lava */
    4.0F, /* 0x1F push_right_2 */
    4.0F, /* 0x20 push_left_2 */
    4.0F, /* 0x21 push_right_1 */
    4.0F, /* 0x22 push_left_1 */
    4.0F, /* 0x23 acid */
    4.0F, /* 0x24 bounce */
};

/**
 * Ground friction of a material id. Ids 0x00..0x0F read the vanilla table, 0x10..0x24 the Remix surfaces;
 * any other id gets the default friction instead of reading past the tables.
 */
f32 grRemixSurfaceFriction(u32 material)
{
    u32 remix_index = material - GRREMIX_SURFACE_VANILLA_COUNT;

    if (material < GRREMIX_SURFACE_VANILLA_COUNT)
    {
        return dMPCollisionMaterialFrictions[material];
    }
    if (remix_index >= (sizeof(sGRRemixSurfaceFrictions) / sizeof(sGRRemixSurfaceFrictions[0])))
    {
        return GRREMIX_SURFACE_FRICTION_DEFAULT;
    }
    return sGRRemixSurfaceFrictions[remix_index];
}

#endif
