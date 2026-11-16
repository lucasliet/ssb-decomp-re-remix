/**
 * ftremixmarinarules.c - pure rules of Marina's midair jumps and Cargo shake gem, see ftremixmarinarules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/marina/ftremixmarinarules.h"

/* Marina.asm:875-882 jump_height_table */
static const unsigned char sFTRemixMarinaJumpHeights[FTREMIX_MARINA_JUMP_HEIGHTS] = { 0, 80, 70, 60, 50, 40 };

int ftRemixMarinaJumpStickRange(int jumps_used)
{
    if ((jumps_used < 0) || (jumps_used >= FTREMIX_MARINA_JUMP_HEIGHTS))
    {
        return 0;
    }
    return sFTRemixMarinaJumpHeights[jumps_used];
}

int ftRemixMarinaJumpIsLocked(int status_id, int temp_variable_2)
{
    if ((status_id != FTREMIX_MARINA_JUMP_STATUS_AERIAL_F) && (status_id != FTREMIX_MARINA_JUMP_STATUS_AERIAL_B))
    {
        return 0;
    }
    return (temp_variable_2 == 0) ? 1 : 0;
}

int ftRemixMarinaShakeDamageApplies(int damage, int hit_status)
{
    if (damage == 0)
    {
        return 0;
    }
    return (hit_status == FTREMIX_MARINA_SHAKE_HITSTATUS_VULNERABLE) ? 1 : 0;
}

int ftRemixMarinaShakeSpawnsGem(int damage_applied, int gem_spawned)
{
    if (damage_applied == 0)
    {
        return 0;
    }
    return (gem_spawned == 0) ? 1 : 0;
}

#endif
