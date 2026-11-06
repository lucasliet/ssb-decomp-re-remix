/**
 * sysremixenvmap.c - Smash Remix SinglePlayer.asm env_mapping_fix_: look-at vectors for environment mapping on the screens
 * whose camera sets none. The two vectors are the asm tables custom_lighting_1 and custom_lighting_2, byte for byte.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sys/sysremixenvmap.h"
#include "sys/sysremixenvmaprules.h"
#include <sc/scene.h>

_Static_assert(nSCKind1PIntro == 0x0E, "env_mapping_fix_ VS preview screen id");
_Static_assert(nSCKindPlayersVS == 0x10, "env_mapping_fix_ first CSS screen id");
_Static_assert(nSCKind1PBonus2Players == 0x14, "env_mapping_fix_ last CSS screen id");
_Static_assert(nSCKindVSResults == 0x18, "env_mapping_fix_ results screen id");
_Static_assert(nSCKindEnding == 0x30, "env_mapping_fix_ room screen id");
_Static_assert(nSCKind1PContinue == 0x31, "env_mapping_fix_ Continue screen id");

static const Light_t sSYRemixEnvMapLookAtX =
{
    { 0x00, 0x00, 0x00 }, 0x00,
    { 0x44, 0x00, 0x00 }, 0x00,
    { 0x40, 0x00, 0x02 }, 0x00
};

static const Light_t sSYRemixEnvMapLookAtY =
{
    { 0x00, 0x80, 0x00 }, 0x00,
    { 0x00, 0x80, 0x00 }, 0x00,
    { 0x01, 0x75, 0x00 }, 0x00
};

void sysRemixEnvMapAppendLookAt(Gfx **dl)
{
    if (sysRemixEnvMapIsLookAtScreen(gSCManagerSceneData.scene_curr) == 0)
    {
        return;
    }
    gSPLookAtX((*dl)++, &sSYRemixEnvMapLookAtX);
    gSPLookAtY((*dl)++, &sSYRemixEnvMapLookAtY);
}

#endif
