/**
 * grremixpeachbumper.c - Peach's Castle bumper hitbox of the Remix build. See grremixpeachbumper.h.
 */

#ifdef PORT

#include "grremixpeachbumper.h"
#include "grremixgating.h"
#include "grremixids.h"
#include <it/itvars.h>
#include <sc/scene.h>

#define GRREMIX_CASINO_BUMPER_FGM_ID 0x03D5
#define GRREMIX_CASINO_BUMPER_SIZE 250.0F
#define GRREMIX_CASINO_BUMPER_FKB 0x40

/* peach_bumper_hitbox_ (Hazards.asm 0x8017D710, _casino): the casino hitbox sets the low byte of knockback_scale (sb 0x0143) */
static void grRemixPeachBumperSetCasinoHitbox(ITStruct *ip)
{
    ip->attack_coll.fgm_id = GRREMIX_CASINO_BUMPER_FGM_ID;
    ip->attack_coll.size = GRREMIX_CASINO_BUMPER_SIZE;
    ip->attack_coll.knockback_scale = (ip->attack_coll.knockback_scale & ~0xFF) | GRREMIX_CASINO_BUMPER_FKB;
}

/* peach_bumper_hitbox_ (Hazards.asm 0x8017D710): castle knockback on the Peach's Castle stages, casino hitbox on Casino */
void grRemixPeachBumperSetupHitbox(GObj *item_gobj)
{
    s32 gkind = gSCManagerBattleState->gkind;
    ITStruct *ip = itGetStruct(item_gobj);

    if (grRemixStageHasPeachBumper(gkind))
    {
        ip->attack_coll.knockback_weight = ITBUMPER_CASTLE_KNOCKBACK;
        ip->attack_coll.angle = ITBUMPER_CASTLE_ANGLE;
        return;
    }

    if (gkind == nGRRemixStage_CASINO)
    {
        grRemixPeachBumperSetCasinoHitbox(ip);
    }
}

#endif
