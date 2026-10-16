/**
 * grremixspawn.c - Remix respawn point overrides for the vanilla map object positions. See ft/ftremix/ftremixhooks.h.
 */

#ifdef PORT

#include <ssb_types.h>
#include <gr/grremix/grremixids.h>
#include <sc/scene.h>

#define GRREMIX_RESPAWN_DREAM_LAND_BETA_X 0.0F
#define GRREMIX_RESPAWN_DREAM_LAND_BETA_Y 2526.0F
#define GRREMIX_RESPAWN_SECTOR_Z_REMIX_X 6580.0F
#define GRREMIX_RESPAWN_SECTOR_Z_REMIX_Y 3150.0F

/**
 * Writes the respawn point of the Dream Land beta and Sector Z Remix stages (Spawn.asm load_respawn_point_) and
 * returns TRUE when the position was overridden. Other stages keep the vanilla map object position.
 */
sb32 grRemixSpawnRespawnPointOverride(Vec3f *pos)
{
    s32 gkind = gSCManagerBattleState->gkind;

    switch (gkind)
    {
    case nGRRemixStage_DREAM_LAND_BETA_1:
    case nGRRemixStage_DREAM_LAND_BETA_2:
        pos->x = GRREMIX_RESPAWN_DREAM_LAND_BETA_X;
        pos->y = GRREMIX_RESPAWN_DREAM_LAND_BETA_Y;
        pos->z = 0.0F;
        return TRUE;

    case nGRRemixStage_SECTOR_Z_REMIX:
        pos->x = GRREMIX_RESPAWN_SECTOR_Z_REMIX_X;
        pos->y = GRREMIX_RESPAWN_SECTOR_Z_REMIX_Y;
        pos->z = 0.0F;
        return TRUE;

    default:
        return FALSE;
    }
}

#endif
