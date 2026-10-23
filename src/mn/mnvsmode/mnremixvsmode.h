#ifndef _MNREMIXVSMODE_H_
#define _MNREMIXVSMODE_H_

#include <ssb_types.h>
#include <sys/objdef.h>

#define MNREMIX_VSMODE_PAGE_1 0
#define MNREMIX_VSMODE_PAGE_2 1

#define MNREMIX_VSMODE_OPTION_REMIX_MODES 4
#define MNREMIX_VSMODE_OPTION_LAST 4

#define MNREMIX_VSMODE_ROW_REMIX_MODES 5

#if defined(PORT) && defined(REGION_REMIX)

enum
{
    MNREMIX_VSMODE_DEFAULT,
    MNREMIX_VSMODE_TWELVE_CB,
    MNREMIX_VSMODE_TAG_TEAM,
    MNREMIX_VSMODE_KOTH,
    MNREMIX_VSMODE_SMASHKETBALL,
    MNREMIX_VSMODE_TUG_OF_WAR
};

extern s32 gMNVSModeRemixMode;

extern GObj* mnRemixVSModeMakeButton(s32 row_index, void *file);

#endif

#endif
