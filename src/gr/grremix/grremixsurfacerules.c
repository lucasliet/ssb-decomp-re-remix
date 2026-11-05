/**
 * grremixsurfacerules.c - pure surface knockback rules, see grremixsurfacerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixsurfacerules.h"

#define GRREMIX_SURFACE_RULES_CRUEL_SHIFT 16

/*
 * Surface.asm knockback_table: the six original structs (ORIGINAL_STRUCT_1..6, materials 7..0xB and 0xF) followed by the
 * add_surface calls with bool_knockback set, in material order. Columns: material, type, damage, angle, growth, fixed,
 * base, effect, fgm. The asm adds the fgm -1 word to the original structs.
 */
static const GRRemixSurfaceKnockback sGRRemixSurfaceKnockbacks[GRREMIX_SURFACE_RULES_ROW_COUNT] =
{
    { 0x07, 4, 1, 361, 100, 100, 0, 1, -1 },
    { 0x08, 5, 10, 90, 100, 200, 0, 1, -1 },
    { 0x09, 6, 10, 90, 100, 100, 0, 1, -1 },
    { 0x0A, 7, 10, 361, 100, 80, 0, 3, -1 },
    { 0x0B, 8, 1, 90, 100, 100, 0, 1, -1 },
    { 0x0F, 9, 1, 90, 100, 100, 0, 1, -1 },
    { 0x10, 8, 4, 105, 5, 0, 125, 1, -1 },           /* big_blue_surface_1 */
    { 0x12, 0x00010008, 7, 90, 0, 0, 0x20, 0, 0x38 }, /* failed_z_cancel */
    { 0x13, 8, 4, 90, 5, 0, 130, 1, -1 },            /* big_blue_surface_2 */
    { 0x14, 8, 60, 190, 20, 0, 130, 2, 0x3C },       /* corneria_surface_1 */
    { 0x15, 8, 30, 180, 20, 0, 130, 2, 0x16 },       /* corneria_surface_2 */
    { 0x16, 20, 15, 90, 100, 0, 65, 0, 0x1F },       /* mute */
    { 0x17, 20, 10, 315, 100, 0, 65, 0, 0x3D6 },     /* casino_left_diagonal */
    { 0x18, 20, 10, 100, 100, 0, 65, 0, 0x3D6 },     /* casino_left_top */
    { 0x19, 20, 10, 0, 100, 0, 65, 0, 0x3D6 },       /* casino_left_side */
    { 0x1A, 20, 10, 225, 100, 0, 65, 0, 0x3D6 },     /* casino_right_diagonal */
    { 0x1B, 20, 10, 100, 80, 0, 65, 0, 0x3D6 },      /* casino_right_top */
    { 0x1C, 20, 10, 180, 0, 0, 65, 0, 0x3D6 },       /* casino_right_side */
    { 0x1D, 8, 20, 90, 100, 200, 0, 0, 0x11F },      /* toadsturnpike_car */
    { 0x1E, 0x00010005, 10, 90, 100, 200, 0, 1, 0x11E }, /* failed_z_cancel_lava */
};

int grRemixSurfaceRulesFindRow(unsigned int material)
{
    int row;

    for (row = 0; row < GRREMIX_SURFACE_RULES_ROW_COUNT; row++)
    {
        if ((unsigned int)sGRRemixSurfaceKnockbacks[row].material == material)
        {
            return row;
        }
    }
    return -1;
}

const GRRemixSurfaceKnockback* grRemixSurfaceRulesGetRow(int row)
{
    if ((row < 0) || (row >= GRREMIX_SURFACE_RULES_ROW_COUNT))
    {
        return 0;
    }
    return &sGRRemixSurfaceKnockbacks[row];
}

int grRemixSurfaceRulesIsCruel(const GRRemixSurfaceKnockback *knockback)
{
    return ((unsigned int)knockback->kind >> GRREMIX_SURFACE_RULES_CRUEL_SHIFT) != 0;
}

int grRemixSurfaceRulesIsBlockedByHazardMode(const GRRemixSurfaceKnockback *knockback, int hazards_are_on)
{
    if (grRemixSurfaceRulesIsCruel(knockback))
    {
        return 0;
    }
    return hazards_are_on == 0;
}

int grRemixSurfaceRulesBumperMaterial(unsigned int wall_surface, int is_left_wall, int lr)
{
    int material = (int)wall_surface;

    if (wall_surface <= GRREMIX_SURFACE_RULES_BUMPER_LAST_NON_BUMPER)
    {
        return -1;
    }
    if (lr != 1)
    {
        return material;
    }
    if (is_left_wall)
    {
        return material - GRREMIX_SURFACE_RULES_BUMPER_SIDE_SHIFT;
    }
    return material + GRREMIX_SURFACE_RULES_BUMPER_SIDE_SHIFT;
}

int grRemixSurfaceRulesBumperStatusApplies(int status_id)
{
    if (status_id < GRREMIX_SURFACE_RULES_STATUS_DAMAGE_FIRST)
    {
        return 1;
    }
    return status_id > GRREMIX_SURFACE_RULES_STATUS_DAMAGE_LAST;
}

#endif
