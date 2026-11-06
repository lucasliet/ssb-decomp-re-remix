#if defined(PORT) && defined(REGION_REMIX)
#ifndef _SCREMIXVSDEMO_H_
#define _SCREMIXVSDEMO_H_

#include <ssb_types.h>

/* VsDemo.asm (Smash Remix): picks the two focused auto-demo fighters. Hooked at the epilogue of mnTitleSetDemoFighterKinds. */
extern void scRemixVsDemoPickFocusFighters(void);

/* VsDemo.asm (Smash Remix): picks a non-focused auto-demo fighter for player 2 and up. Hooked in scAutoDemoGetFighterKind. */
extern s32 scRemixVsDemoPickNonFocusedFighter(s32 player);

/* VsDemo.asm (Smash Remix): picks a random battle stage for the auto demo. Hooked in scAutoDemoInitDemo. */
extern s32 scRemixVsDemoPickStage(s32 fallback_stage_id);

/* VsDemo.asm (Smash Remix): 1P name texture offset for a Remix fighter, or -1 when the Remix RAM image is not loaded. */
extern s32 scRemixVsDemoGetNameTextureOffset(s32 fkind);

#endif
#endif
