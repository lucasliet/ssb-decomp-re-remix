/**
 * efremixgroundrules.c - background sprite tables of the Remix stages. See efremixgroundrules.h.
 *
 * Rows transcribed from Stages.asm add_bg_animation (5078-5265) and the stages/<stage>.asm + .bin files they include:
 * sprite_info_arrays become the params, each 0x40-byte sprite of the .bin becomes a sprite (only the sprites a param
 * names; flags 0x7, display link 4, transforms 0x1C/0x1B and effect status -1 are the same in every .bin).
 * The asm writes direction 0xF where the vanilla Sector Z and Yoshi's Island tables have -1 and -3: a sprite with lr 15
 * starts at the right bound and efGroundCheckEffectInBounds removes it at once, so the port keeps the vanilla value
 * (marked "asm 0xF"; docs/remix/deviations.md).
 */

#include "ef/efremixgroundrules.h"
#include "gr/grremix/grremixids.h"

#include <stddef.h>

#define EFREMIXGROUND_COUNT(array) ((int32_t)(sizeof(array) / sizeof((array)[0])))

/* stages/spiralm.asm, spiralm.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundSpiralmParams[] =
{
    { 0, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundSpiralmSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0xAEB8, 0xAF3C, 0x11110, 0x11180 },
};

/* stages/zlanding.asm, zlanding.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundZlandingParams[] =
{
    { 0, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundZlandingSprites[] =
{
    { 4000.0F, -1000.0F, -2000.0F, 1.3F, nEFRemixGroundSpriteProcCommon, 0x67B8, 0x0, 0x692C, 0x0 },
};

/* stages/smashville.asm, smashville.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundSmashville2Params[] =
{
    { 0, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundSmashville2Sprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x7D08, 0x7D8C, 0xAEA0, 0xAEF4 },
};

/* stages/yoshi_story.asm, yoshi_story.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundYoshiStory2Params[] =
{
    { 3, 0, 0, 2 },
    { 3, 1, 0, 1 },
    { 1, 0, -3, 1 },  /* asm 0xF */
    { 2, 0, 3, 1 },
    { 4, 0, 0, 1 },
    { 5, 0, 0, 1 },
    { 0, 0, 0, 4 },
};

static const EFRemixGroundSprite dEFRemixGroundYoshiStory2Sprites[] =
{
    { 3000.0F, 500.0F, -6000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0x13150, 0x131D4, 0x14B84, 0x14D88 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0x14EC0, 0x14F44, 0x15D24, 0x16104 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0x16240, 0x14F44, 0x15D24, 0x16104 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x163F8, 0x1647C, 0x171A4, 0x17588 },
    { 3000.0F, 500.0F, -2000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x176C0, 0x17744, 0x18464, 0x18848 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x18980, 0x18A04, 0x18464, 0x17588 },
};

/* stages/congoj_dl.asm, congoj_dl.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundCongojDlParams[] =
{
    { 0, 0, 0, 2 },
    { 0, 1, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundCongojDlSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0xC160, 0xC1E4, 0xEB24, 0xEBF4 },
};

/* stages/congoj_o.asm, congoj_o.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundCongojOParams[] =
{
    { 0, 0, 0, 2 },
    { 0, 1, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundCongojOSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0xB820, 0xB8A4, 0xE1E4, 0xE2B4 },
};

/* stages/pcastle_dl.asm, pcastle_dl.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundPcastleDlParams[] =
{
    { 0, 0, 0, 2 },
    { 1, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundPcastleDlSprites[] =
{
    { 4000.0F, -1000.0F, -2000.0F, 1.3F, nEFRemixGroundSpriteProcCommon, 0x3DD8, 0x0, 0x401C, 0x0 },
    { 4000.0F, -1000.0F, -1000.0F, 1.0F, nEFRemixGroundSpriteProcCommon, 0x3DD8, 0x0, 0x401C, 0x0 },
};

/* stages/pcastle_o.asm, pcastle_o.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundPcastleOParams[] =
{
    { 0, 0, 0, 2 },
    { 1, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundPcastleOSprites[] =
{
    { 4000.0F, -1000.0F, -2000.0F, 1.3F, nEFRemixGroundSpriteProcCommon, 0x3568, 0x0, 0x37AC, 0x0 },
    { 4000.0F, -1000.0F, -1000.0F, 1.0F, nEFRemixGroundSpriteProcCommon, 0x3568, 0x0, 0x37AC, 0x0 },
};

/* stages/sector_z_dl.asm, sector_z_dl.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundSectorZDlParams[] =
{
    { 3, 0, -1, 3 },  /* asm 0xF */
    { 4, 0, 1, 3 },
    { 4, 1, 1, 4 },
    { 0, 0, -1, 3 },  /* asm 0xF */
    { 1, 0, -1, 2 },  /* asm 0xF */
    { 2, 0, -1, 2 },  /* asm 0xF */
    { 2, 0, 1, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundSectorZDlSprites[] =
{
    { 5000.0F, 0.0F, -10000.0F, 6.0F, nEFRemixGroundSpriteProcStep, 0x69A0, 0x0, 0x6BBC, 0x0 },
    { 5000.0F, 0.0F, -3000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x69A0, 0x0, 0x6BBC, 0x0 },
    { 5000.0F, 0.0F, -10000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x69A0, 0x0, 0x6BBC, 0x0 },
    { 4000.0F, 0.0F, -8000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x6D78, 0x6DFC, 0x72D0, 0x7300 },
    { 4000.0F, 0.0F, -8000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x6D78, 0x6DFC, 0x72D0, 0x7300 },
};

/* stages/yoshi_island_dl.asm, yoshi_island_dl.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundYoshiIslandDlParams[] =
{
    { 3, 0, 0, 2 },
    { 3, 1, 0, 1 },
    { 1, 0, -3, 1 },  /* asm 0xF */
    { 2, 0, 3, 1 },
    { 4, 0, 0, 1 },
    { 5, 0, 0, 1 },
    { 0, 0, 0, 4 },
};

static const EFRemixGroundSprite dEFRemixGroundYoshiIslandDlSprites[] =
{
    { 3000.0F, 500.0F, -6000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0x9980, 0x9A04, 0xB3B4, 0xB5B8 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0xB6F0, 0xB774, 0xC554, 0xC934 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0xCA70, 0xB774, 0xC554, 0xC934 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xCC28, 0xCCAC, 0xD9D4, 0xDDB8 },
    { 3000.0F, 500.0F, -2000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xDEF0, 0xDF74, 0xEC94, 0xF078 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xF1B0, 0xF234, 0xEC94, 0xDDB8 },
};

/* stages/zebes_dl.asm, zebes_dl.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundZebesDlParams[] =
{
    { 0, 0, 0, 1 },
    { 0, 1, 0, 1 },
    { 1, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundZebesDlSprites[] =
{
    { 4000.0F, 1000.0F, -5000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x10F48, 0x10FCC, 0x11CFC, 0x11D4C },
    { 6000.0F, 2000.0F, -10000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0x11F20, 0x11FA4, 0x12908, 0x12998 },
};

/* stages/sector_z_o.asm, sector_z_o.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundSectorZOParams[] =
{
    { 3, 0, -1, 3 },  /* asm 0xF */
    { 4, 0, 1, 3 },
    { 4, 1, 1, 4 },
    { 0, 0, -1, 3 },  /* asm 0xF */
    { 1, 0, -1, 2 },  /* asm 0xF */
    { 2, 0, -1, 2 },  /* asm 0xF */
    { 2, 0, 1, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundSectorZOSprites[] =
{
    { 5000.0F, 0.0F, -10000.0F, 6.0F, nEFRemixGroundSpriteProcStep, 0x69D0, 0x0, 0x6BEC, 0x0 },
    { 5000.0F, 0.0F, -3000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x69D0, 0x0, 0x6BEC, 0x0 },
    { 5000.0F, 0.0F, -10000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x69D0, 0x0, 0x6BEC, 0x0 },
    { 4000.0F, 0.0F, -8000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x6DA8, 0x6E2C, 0x7300, 0x7330 },
    { 4000.0F, 0.0F, -8000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0x6DA8, 0x6E2C, 0x7300, 0x7330 },
};

/* stages/yoshi_island_o.asm, yoshi_island_o.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundYoshiIslandOParams[] =
{
    { 3, 0, 0, 2 },
    { 3, 1, 0, 1 },
    { 1, 0, -3, 1 },  /* asm 0xF */
    { 2, 0, 3, 1 },
    { 4, 0, 0, 1 },
    { 5, 0, 0, 1 },
    { 0, 0, 0, 4 },
};

static const EFRemixGroundSprite dEFRemixGroundYoshiIslandOSprites[] =
{
    { 3000.0F, 500.0F, -6000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0x8F90, 0x9014, 0xA9C4, 0xABC8 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0xAD00, 0xAD84, 0xBB64, 0xBF44 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0xC080, 0xAD84, 0xBB64, 0xBF44 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xC238, 0xC2BC, 0xCFE4, 0xD3C8 },
    { 3000.0F, 500.0F, -2000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xD500, 0xD584, 0xE2A4, 0xE688 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xE7C0, 0xE844, 0xE2A4, 0xD3C8 },
};

/* stages/dream_land_o.asm, dream_land_o.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundDreamLandOParams[] =
{
    { 0, 0, 0, 2 },
    { 0, 1, 0, 1 },
    { 1, 0, 0, 2 },
    { 1, 1, 0, 1 },
    { 2, 0, 0, 1 },
    { 3, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundDreamLandOSprites[] =
{
    { 2000.0F, -300.0F, -4000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x8320, 0x83A4, 0x8AF0, 0x8CF4 },
    { 1500.0F, -300.0F, -3000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x8320, 0x83A4, 0x8DB4, 0x8F7C },
    { 3000.0F, 1000.0F, -5000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x90B8, 0x0, 0x9298, 0x0 },
    { 3000.0F, 1000.0F, -5000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0x90B8, 0x0, 0x9600, 0x0 },
};

/* stages/zebes_o.asm, zebes_o.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundZebesOParams[] =
{
    { 0, 0, 0, 1 },
    { 0, 1, 0, 1 },
    { 1, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundZebesOSprites[] =
{
    { 4000.0F, 1000.0F, -5000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x10A58, 0x10ADC, 0x1180C, 0x1185C },
    { 6000.0F, 2000.0F, -10000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0x11A30, 0x11AB4, 0x12418, 0x124A8 },
};

/* stages/japes.asm, japes.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundJapesParams[] =
{
    { 0, 0, 0, 2 },
    { 0, 1, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundJapesSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0xA4B0, 0xA534, 0xCE74, 0xCF44 },
};

/* stages/csiege.asm, csiege.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundCsiegeParams[] =
{
    { 0, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundCsiegeSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x7760, 0x77E4, 0xA124, 0xA1F4 },
};

/* stages/yoshi_island_II.asm, yoshi_island_II.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundYoshisIslandIiParams[] =
{
    { 3, 0, 0, 2 },
    { 3, 1, 0, 1 },
    { 1, 0, -3, 1 },  /* asm 0xF */
    { 2, 0, 3, 1 },
    { 4, 0, 0, 1 },
    { 5, 0, 0, 1 },
    { 0, 0, 0, 4 },
};

static const EFRemixGroundSprite dEFRemixGroundYoshisIslandIiSprites[] =
{
    { 3000.0F, 500.0F, -6000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0xC930, 0xC9B4, 0xE364, 0xE568 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0xE6A0, 0xE724, 0xF504, 0xF8E4 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcYaw, 0xFA20, 0xE724, 0xF504, 0xF8E4 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xFBD8, 0xFC5C, 0x10984, 0x10D68 },
    { 3000.0F, 500.0F, -2000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x10EA0, 0x10F24, 0x11C44, 0x12028 },
    { 3000.0F, 500.0F, -3000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x12160, 0x121E4, 0x11C44, 0x10D68 },
};

/* stages/dream_greens.asm, dream_greens.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundDreamLandSrParams[] =
{
    { 0, 0, 0, 2 },
    { 0, 1, 0, 1 },
    { 1, 0, 0, 2 },
    { 1, 1, 0, 1 },
    { 2, 0, 0, 1 },
    { 3, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundDreamLandSrSprites[] =
{
    { 2000.0F, -300.0F, -4000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0xA330, 0xA3B4, 0xAB00, 0xAD04 },
    { 1500.0F, -300.0F, -3000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0xA330, 0xA3B4, 0xADC4, 0xAF8C },
    { 3000.0F, 1000.0F, -5000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xB0C8, 0x0, 0xB2A8, 0x0 },
    { 3000.0F, 1000.0F, -5000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0xB0C8, 0x0, 0xB610, 0x0 },
};

/* stages/pcastle_beta.asm, pcastle_beta.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundPcastleBetaParams[] =
{
    { 0, 0, 0, 2 },
    { 1, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundPcastleBetaSprites[] =
{
    { 4000.0F, -1000.0F, -2000.0F, 1.3F, nEFRemixGroundSpriteProcCommon, 0x4C88, 0x0, 0x4ECC, 0x0 },
    { 4000.0F, -1000.0F, -1000.0F, 1.0F, nEFRemixGroundSpriteProcCommon, 0x4C88, 0x0, 0x4ECC, 0x0 },
};

/* stages/sector_z_remix.asm, sector_z_remix.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundSectorZRemixParams[] =
{
    { 3, 0, -1, 3 },  /* asm 0xF */
    { 4, 0, 1, 3 },
    { 4, 1, 1, 4 },
    { 0, 0, -1, 3 },  /* asm 0xF */
    { 1, 0, -1, 2 },  /* asm 0xF */
    { 2, 0, -1, 2 },  /* asm 0xF */
    { 2, 0, 1, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundSectorZRemixSprites[] =
{
    { 5000.0F, 0.0F, -10000.0F, 6.0F, nEFRemixGroundSpriteProcStep, 0xAD00, 0x0, 0xADB0, 0x0 },
    { 5000.0F, 0.0F, -3000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0xAD00, 0x0, 0xADB0, 0x0 },
    { 5000.0F, 0.0F, -10000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0xAD00, 0x0, 0xADB0, 0x0 },
    { 4000.0F, 0.0F, -8000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0xB6F8, 0xB3C0, 0xB780, 0xB7B0 },
    { 4000.0F, 0.0F, -8000.0F, 6.0F, nEFRemixGroundSpriteProcCommon, 0xB6F8, 0xB3C0, 0xB7E0, 0xB7B0 },
};

/* stages/ghz.asm, ghz.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundGhzParams[] =
{
    { 2, 0, 1, 1 },
    { 2, 0, 0, 1 },
    { 0, 0, 1, 1 },
    { 0, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundGhzSprites[] =
{
    { 2000.0F, -300.0F, -4000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x8DC8, 0x8E4C, 0x9F90, 0x9FBC },
    { 1500.0F, -300.0F, -3000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x8DC8, 0x8E4C, 0xA07C, 0x9FBC },
    { 3000.0F, 1000.0F, -5000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0xA1D8, 0x0, 0xA3B8, 0x0 },
};

/* stages/winter_dl.asm, winter_dl.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundWinterDlParams[] =
{
    { 0, 0, 0, 2 },
    { 0, 1, 0, 1 },
    { 1, 0, 0, 2 },
    { 1, 1, 0, 1 },
    { 2, 0, 0, 1 },
    { 3, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundWinterDlSprites[] =
{
    { 2000.0F, -300.0F, -4000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x7AE0, 0x7B64, 0x82B0, 0x84B4 },
    { 1500.0F, -300.0F, -3000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x7AE0, 0x7B64, 0x8574, 0x873C },
    { 3000.0F, 1000.0F, -5000.0F, 2.0F, nEFRemixGroundSpriteProcCommon, 0x8878, 0x0, 0x8A58, 0x0 },
    { 3000.0F, 1000.0F, -5000.0F, 3.0F, nEFRemixGroundSpriteProcCommon, 0x8878, 0x0, 0x8DC0, 0x0 },
};

/* stages/edo.asm, edo.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundEdoParams[] =
{
    { 0, 1, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundEdoSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0xBD58, 0xBDDC, 0xCEFC, 0xCF30 },
};

/* stages/scuttletown.asm, scuttletown.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundScuttleTownParams[] =
{
    { 0, 0, 0, 2 },
};

static const EFRemixGroundSprite dEFRemixGroundScuttleTownSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x85A0, 0x8624, 0xAF64, 0xAFBC },
};

/* stages/bbh.asm, bbh.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundBigBoosHauntParams[] =
{
    { 0, 1, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundBigBoosHauntSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x7258, 0x72DC, 0x7BEC, 0x7C14 },
};

/* stages/smashville_remix.asm, smashville_remix.bin */
static const EFRemixGroundSpriteParam dEFRemixGroundSmashvilleRemixParams[] =
{
    { 0, 0, 0, 1 },
};

static const EFRemixGroundSprite dEFRemixGroundSmashvilleRemixSprites[] =
{
    { 4000.0F, 3000.0F, -12000.0F, 1.4F, nEFRemixGroundSpriteProcCommon, 0x7A48, 0x7ACC, 0xABE0, 0xAC44 },
};

static const EFRemixGroundStage dEFRemixGroundStages[] =
{
    { nGRRemixStage_SPIRALM, 0x98A0, dEFRemixGroundSpiralmParams, EFREMIXGROUND_COUNT(dEFRemixGroundSpiralmParams), dEFRemixGroundSpiralmSprites, EFREMIXGROUND_COUNT(dEFRemixGroundSpiralmSprites) },
    { nGRRemixStage_ZLANDING, 0x48C0, dEFRemixGroundZlandingParams, EFREMIXGROUND_COUNT(dEFRemixGroundZlandingParams), dEFRemixGroundZlandingSprites, EFREMIXGROUND_COUNT(dEFRemixGroundZlandingSprites) },
    { nGRRemixStage_SMASHVILLE2, 0x6D28, dEFRemixGroundSmashville2Params, EFREMIXGROUND_COUNT(dEFRemixGroundSmashville2Params), dEFRemixGroundSmashville2Sprites, EFREMIXGROUND_COUNT(dEFRemixGroundSmashville2Sprites) },
    { nGRRemixStage_YOSHI_STORY_2, 0xE870, dEFRemixGroundYoshiStory2Params, EFREMIXGROUND_COUNT(dEFRemixGroundYoshiStory2Params), dEFRemixGroundYoshiStory2Sprites, EFREMIXGROUND_COUNT(dEFRemixGroundYoshiStory2Sprites) },
    { nGRRemixStage_CONGOJ_DL, 0xB788, dEFRemixGroundCongojDlParams, EFREMIXGROUND_COUNT(dEFRemixGroundCongojDlParams), dEFRemixGroundCongojDlSprites, EFREMIXGROUND_COUNT(dEFRemixGroundCongojDlSprites) },
    { nGRRemixStage_CONGOJ_O, 0xAE88, dEFRemixGroundCongojOParams, EFREMIXGROUND_COUNT(dEFRemixGroundCongojOParams), dEFRemixGroundCongojOSprites, EFREMIXGROUND_COUNT(dEFRemixGroundCongojOSprites) },
    { nGRRemixStage_PCASTLE_DL, 0x35F0, dEFRemixGroundPcastleDlParams, EFREMIXGROUND_COUNT(dEFRemixGroundPcastleDlParams), dEFRemixGroundPcastleDlSprites, EFREMIXGROUND_COUNT(dEFRemixGroundPcastleDlSprites) },
    { nGRRemixStage_PCASTLE_O, 0x2DC0, dEFRemixGroundPcastleOParams, EFREMIXGROUND_COUNT(dEFRemixGroundPcastleOParams), dEFRemixGroundPcastleOSprites, EFREMIXGROUND_COUNT(dEFRemixGroundPcastleOSprites) },
    { nGRRemixStage_SECTOR_Z_DL, 0x6250, dEFRemixGroundSectorZDlParams, EFREMIXGROUND_COUNT(dEFRemixGroundSectorZDlParams), dEFRemixGroundSectorZDlSprites, EFREMIXGROUND_COUNT(dEFRemixGroundSectorZDlSprites) },
    { nGRRemixStage_YOSHI_ISLAND_DL, 0x84D8, dEFRemixGroundYoshiIslandDlParams, EFREMIXGROUND_COUNT(dEFRemixGroundYoshiIslandDlParams), dEFRemixGroundYoshiIslandDlSprites, EFREMIXGROUND_COUNT(dEFRemixGroundYoshiIslandDlSprites) },
    { nGRRemixStage_ZEBES_DL, 0x75E0, dEFRemixGroundZebesDlParams, EFREMIXGROUND_COUNT(dEFRemixGroundZebesDlParams), dEFRemixGroundZebesDlSprites, EFREMIXGROUND_COUNT(dEFRemixGroundZebesDlSprites) },
    { nGRRemixStage_SECTOR_Z_O, 0x62D0, dEFRemixGroundSectorZOParams, EFREMIXGROUND_COUNT(dEFRemixGroundSectorZOParams), dEFRemixGroundSectorZOSprites, EFREMIXGROUND_COUNT(dEFRemixGroundSectorZOSprites) },
    { nGRRemixStage_YOSHI_ISLAND_O, 0x7AC8, dEFRemixGroundYoshiIslandOParams, EFREMIXGROUND_COUNT(dEFRemixGroundYoshiIslandOParams), dEFRemixGroundYoshiIslandOSprites, EFREMIXGROUND_COUNT(dEFRemixGroundYoshiIslandOSprites) },
    { nGRRemixStage_DREAM_LAND_O, 0x5DB8, dEFRemixGroundDreamLandOParams, EFREMIXGROUND_COUNT(dEFRemixGroundDreamLandOParams), dEFRemixGroundDreamLandOSprites, EFREMIXGROUND_COUNT(dEFRemixGroundDreamLandOSprites) },
    { nGRRemixStage_ZEBES_O, 0x7128, dEFRemixGroundZebesOParams, EFREMIXGROUND_COUNT(dEFRemixGroundZebesOParams), dEFRemixGroundZebesOSprites, EFREMIXGROUND_COUNT(dEFRemixGroundZebesOSprites) },
    { nGRRemixStage_JAPES, 0x83D8, dEFRemixGroundJapesParams, EFREMIXGROUND_COUNT(dEFRemixGroundJapesParams), dEFRemixGroundJapesSprites, EFREMIXGROUND_COUNT(dEFRemixGroundJapesSprites) },
    { nGRRemixStage_CSIEGE, 0x6230, dEFRemixGroundCsiegeParams, EFREMIXGROUND_COUNT(dEFRemixGroundCsiegeParams), dEFRemixGroundCsiegeSprites, EFREMIXGROUND_COUNT(dEFRemixGroundCsiegeSprites) },
    { nGRRemixStage_YOSHIS_ISLAND_II, 0xB668, dEFRemixGroundYoshisIslandIiParams, EFREMIXGROUND_COUNT(dEFRemixGroundYoshisIslandIiParams), dEFRemixGroundYoshisIslandIiSprites, EFREMIXGROUND_COUNT(dEFRemixGroundYoshisIslandIiSprites) },
    { nGRRemixStage_DREAM_LAND_SR, 0x90C8, dEFRemixGroundDreamLandSrParams, EFREMIXGROUND_COUNT(dEFRemixGroundDreamLandSrParams), dEFRemixGroundDreamLandSrSprites, EFREMIXGROUND_COUNT(dEFRemixGroundDreamLandSrSprites) },
    { nGRRemixStage_PCASTLE_BETA, 0x3950, dEFRemixGroundPcastleBetaParams, EFREMIXGROUND_COUNT(dEFRemixGroundPcastleBetaParams), dEFRemixGroundPcastleBetaSprites, EFREMIXGROUND_COUNT(dEFRemixGroundPcastleBetaSprites) },
    { nGRRemixStage_SECTOR_Z_REMIX, 0x8680, dEFRemixGroundSectorZRemixParams, EFREMIXGROUND_COUNT(dEFRemixGroundSectorZRemixParams), dEFRemixGroundSectorZRemixSprites, EFREMIXGROUND_COUNT(dEFRemixGroundSectorZRemixSprites) },
    { nGRRemixStage_GHZ, 0x76A0, dEFRemixGroundGhzParams, EFREMIXGROUND_COUNT(dEFRemixGroundGhzParams), dEFRemixGroundGhzSprites, EFREMIXGROUND_COUNT(dEFRemixGroundGhzSprites) },
    { nGRRemixStage_WINTER_DL, 0x6610, dEFRemixGroundWinterDlParams, EFREMIXGROUND_COUNT(dEFRemixGroundWinterDlParams), dEFRemixGroundWinterDlSprites, EFREMIXGROUND_COUNT(dEFRemixGroundWinterDlSprites) },
    { nGRRemixStage_EDO, 0xAF58, dEFRemixGroundEdoParams, EFREMIXGROUND_COUNT(dEFRemixGroundEdoParams), dEFRemixGroundEdoSprites, EFREMIXGROUND_COUNT(dEFRemixGroundEdoSprites) },
    { nGRRemixStage_SCUTTLE_TOWN, 0x53D0, dEFRemixGroundScuttleTownParams, EFREMIXGROUND_COUNT(dEFRemixGroundScuttleTownParams), dEFRemixGroundScuttleTownSprites, EFREMIXGROUND_COUNT(dEFRemixGroundScuttleTownSprites) },
    { nGRRemixStage_BIG_BOOS_HAUNT, 0x55D8, dEFRemixGroundBigBoosHauntParams, EFREMIXGROUND_COUNT(dEFRemixGroundBigBoosHauntParams), dEFRemixGroundBigBoosHauntSprites, EFREMIXGROUND_COUNT(dEFRemixGroundBigBoosHauntSprites) },
    { nGRRemixStage_SMASHVILLE_REMIX, 0x6A30, dEFRemixGroundSmashvilleRemixParams, EFREMIXGROUND_COUNT(dEFRemixGroundSmashvilleRemixParams), dEFRemixGroundSmashvilleRemixSprites, EFREMIXGROUND_COUNT(dEFRemixGroundSmashvilleRemixSprites) },
};

int32_t efRemixGroundRulesIsVanillaSlot(int32_t gkind)
{
    return ((gkind >= 0) && (gkind < EFREMIXGROUND_VANILLA_SLOTS)) ? 1 : 0;
}

const EFRemixGroundStage* efRemixGroundRulesFindStage(int32_t gkind)
{
    int32_t i;

    for (i = 0; i < EFREMIXGROUND_COUNT(dEFRemixGroundStages); i++)
    {
        if (dEFRemixGroundStages[i].gkind == gkind)
        {
            return &dEFRemixGroundStages[i];
        }
    }
    return NULL;
}

int32_t efRemixGroundRulesGetStageCount(void)
{
    return EFREMIXGROUND_COUNT(dEFRemixGroundStages);
}

const EFRemixGroundStage* efRemixGroundRulesGetStage(int32_t index)
{
    if ((index < 0) || (index >= EFREMIXGROUND_COUNT(dEFRemixGroundStages)))
    {
        return NULL;
    }
    return &dEFRemixGroundStages[index];
}
