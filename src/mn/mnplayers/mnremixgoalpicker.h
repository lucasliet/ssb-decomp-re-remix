#ifndef _MNREMIXGOALPICKER_H_
#define _MNREMIXGOALPICKER_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

extern sb32 mnRemixGoalPickerIsActive(void);
extern s32 mnRemixGoalPickerNext(s32 current_value);
extern s32 mnRemixGoalPickerPrev(s32 current_value);
extern f32 mnRemixGoalPickerNumberX(s32 number);

#endif

#endif
