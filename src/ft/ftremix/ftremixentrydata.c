/**
 * ftremixentrydata.c - Remix entry effect structs, see ftremixentry.h.
 *
 * samusshared.asm dsamus_entry_anim_gfx_struct and crash_entry_anim_gfx_struct (get_entry_anim_gfx_struct, 0x8010347C).
 * captainshared.asm entry_anim_struct_JFALCON/_BOWSER/_CONKER (get_entry_anim_struct_, 0x801035A0).
 * Wolf.asm entry_anim_struct_WOLF and wolfen_entry_2/_3 (Fox Arwing, 0x80103804..0x801038C4).
 * dkshared.asm sonic_entry_struct_left and lanky_entry_struct (0x80103428).
 * linkshared.asm entry_anim_struct_1_* (Link wave, get_entry_anim_struct_1 at 0x80102AE8) and
 * entry_anim_struct_2_* (Link beam, get_entry_anim_struct_2 at 0x80102B44).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixentry.h"

#define ENTRY_ROW(fkind, effect, file_n, base, dobj, mobj, anim, matanim) \
    { fkind, effect, file_n, base, dobj, mobj, anim, matanim }

const FTRemixEntryOverride dFTRemixEntryOverrides[] =
{
    ENTRY_ROW(REMIX_ID_YLINK,  nFTRemixEntryEffectLinkWave, 7, FTREMIX_ENTRY_SAME_BASE, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP),
    ENTRY_ROW(REMIX_ID_YLINK,  nFTRemixEntryEffectLinkBeam, 7, FTREMIX_ENTRY_SAME_BASE, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP),

    ENTRY_ROW(REMIX_ID_MARTH,  nFTRemixEntryEffectLinkWave, 8, FTREMIX_ENTRY_SAME_BASE, 0x110, 0x218, 0x344, 0x3A0),
    ENTRY_ROW(REMIX_ID_MARTH,  nFTRemixEntryEffectLinkBeam, 8, FTREMIX_ENTRY_SAME_BASE, 0x638, 0x740, 0x76C, 0x7B4),

    ENTRY_ROW(REMIX_ID_ROY,    nFTRemixEntryEffectLinkWave, 8, FTREMIX_ENTRY_SAME_BASE, 0x110, 0x218, 0x344, 0x3A0),
    ENTRY_ROW(REMIX_ID_ROY,    nFTRemixEntryEffectLinkBeam, 8, FTREMIX_ENTRY_SAME_BASE, 0x638, 0x740, 0x76C, 0x7B4),

    ENTRY_ROW(REMIX_ID_MARINA, nFTRemixEntryEffectLinkWave, 9, FTREMIX_ENTRY_SAME_BASE, 0x150, 0x260, 0x42C, 0x458),
    ENTRY_ROW(REMIX_ID_MARINA, nFTRemixEntryEffectLinkBeam, 9, FTREMIX_ENTRY_SAME_BASE, 0x628, 0x740, 0x90C, 0x940),

    ENTRY_ROW(REMIX_ID_GOEMON, nFTRemixEntryEffectLinkWave, FTREMIX_ENTRY_SKIP, FTREMIX_ENTRY_SAME_BASE, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP),
    ENTRY_ROW(REMIX_ID_GOEMON, nFTRemixEntryEffectLinkBeam, 9, FTREMIX_ENTRY_SAME_BASE, 0x990, 0xA98, 0xCA4, 0xCEC),

    ENTRY_ROW(REMIX_ID_EBI,    nFTRemixEntryEffectLinkWave, FTREMIX_ENTRY_SKIP, FTREMIX_ENTRY_SAME_BASE, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP),
    ENTRY_ROW(REMIX_ID_EBI,    nFTRemixEntryEffectLinkBeam, 9, FTREMIX_ENTRY_SAME_BASE, 0x990, 0xA98, 0xCA4, 0xCEC),

    ENTRY_ROW(REMIX_ID_BANJO,  nFTRemixEntryEffectLinkWave, 8, nFTRemixEntryEffectLinkBeam, 0x878, 0x980, 0xA54, 0xA9C),
    ENTRY_ROW(REMIX_ID_BANJO,  nFTRemixEntryEffectLinkBeam, 8, nFTRemixEntryEffectLinkWave, 0x240, 0x348, 0x378, 0x3D4),

    ENTRY_ROW(REMIX_ID_WOLF,   nFTRemixEntryEffectFoxArwing, 8, FTREMIX_ENTRY_SAME_BASE, 0x2610, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP),

    ENTRY_ROW(REMIX_ID_DSAMUS, nFTRemixEntryEffectSamusPoint, 6,FTREMIX_ENTRY_SAME_BASE, 0xBD0, 0x0, 0x22F0, 0x0),
    ENTRY_ROW(REMIX_ID_CRASH,  nFTRemixEntryEffectSamusPoint, 8, FTREMIX_ENTRY_SAME_BASE, 0x2310, 0x0, 0x27E8, 0x0),

    /* DKShared.asm barrel_alternate: Sonic's Tails picks the right struct when the fighter faces right (status entry.lr == +1), the left one otherwise. */
    ENTRY_ROW(REMIX_ID_SONIC,  nFTRemixEntryEffectDonkeyBarrelRight, 8, FTREMIX_ENTRY_SAME_BASE, 0x2A28, 0x0, 0x68C8, 0x0),
    ENTRY_ROW(REMIX_ID_SONIC,  nFTRemixEntryEffectDonkeyBarrel, 8, FTREMIX_ENTRY_SAME_BASE, 0xA5E8, 0x0, 0x112AC, 0x0),
    ENTRY_ROW(REMIX_ID_LANKY,  nFTRemixEntryEffectDonkeyBarrel, 7, FTREMIX_ENTRY_SAME_BASE, 0xF88, 0x0, 0x14E4, 0x0),

    ENTRY_ROW(REMIX_ID_JFALCON, nFTRemixEntryEffectCaptainCar, 7, FTREMIX_ENTRY_SAME_BASE, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP),
    ENTRY_ROW(REMIX_ID_BOWSER,  nFTRemixEntryEffectCaptainCar, 7, FTREMIX_ENTRY_SAME_BASE, 0x1E80, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP),
    ENTRY_ROW(REMIX_ID_CONKER,  nFTRemixEntryEffectCaptainCar, 8, FTREMIX_ENTRY_SAME_BASE, 0xF60, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP)
};

const s32 dFTRemixEntryOverridesNum = ARRAY_COUNT(dFTRemixEntryOverrides);

const FTRemixEntryCarAnim dFTRemixEntryCarAnims[] =
{
    { REMIX_ID_JFALCON, 7, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP, FTREMIX_ENTRY_KEEP },
    { REMIX_ID_BOWSER,  7, 0x248C, 0x24C0, 0x2530 },
    { REMIX_ID_CONKER,  8, 0x188C, 0x18C0, 0x1930 }
};

const s32 dFTRemixEntryCarAnimsNum = ARRAY_COUNT(dFTRemixEntryCarAnims);

const FTRemixEntryArwingAnim dFTRemixEntryArwingAnims[] =
{
    { REMIX_ID_WOLF, 8, 0x284C, 7, 0xF74, 0xB24 }
};

const s32 dFTRemixEntryArwingAnimsNum = ARRAY_COUNT(dFTRemixEntryArwingAnims);

#endif
