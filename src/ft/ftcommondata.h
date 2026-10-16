#ifndef _FTPROGRAMDATA_H_
#define _FTPROGRAMDATA_H_

// Massive file containing data from various files

#include <ssb_types.h>
#include <sys/obj.h>
#include <ft/fttypes.h>
#include <gm/gmtypes.h>

// 0x8012C830
extern f32 dFTCommonDataHandicapTable[/* */][2];

// 0x8012C970
extern u16 dFTCommonDataDownBounceSFX[/* */];

// 0x8012C9A8
extern u16 dFTCommonDataPublicFighterCallFGMs[/* */];

// 0x8012C9E0
extern FTItemThrow dFTCommonDataItemThrowDescs[/* */];

// 0x8012CA38
extern FTItemSwing dFTCommonDataItemSwingAnimSpeeds[/* */];

// 0x8012CA78
extern SYColorRGBA dFTCommonDataShadowColorDefault;

// 0x8012CA7C
extern SYColorRGBA dFTCommonDataShadowColorTeams[/* */];

// 0x8012DBD0
extern GMColDesc dGMColScriptsDescs[/* */]; // 0x8012DBD0

#ifdef REGION_REMIX
#define GMCOLSCRIPTS_REMIX_VANILLA_NUM 0x56   /* ids below this are the vanilla table */
#define GMCOLSCRIPTS_REMIX_DESCS_MAX (GMCOLSCRIPTS_REMIX_VANILLA_NUM + 64)
extern s32 dGMColScriptsDescsNum;
#elif defined(PORT)
extern const s32 dGMColScriptsDescsNum;
#endif

#endif
