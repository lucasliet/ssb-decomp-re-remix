#ifndef _MNREMIXGALLERY_H_
#define _MNREMIXGALLERY_H_

#include <ssb_types.h>

/* Starts the Remix Gallery on the Congratulations scene at its first picture.
 * B returns to previous_scene. */
extern void mnRemixGalleryActivate(s32 previous_scene);
/* Same, at the picture `page` (Toggles.asm view_gallery). */
extern void mnRemixGalleryActivateAtPage(s32 previous_scene, s32 page);
/* Picture the player was on when B left the Gallery last time (Toggles.entry_view_gallery + 4). */
extern s32 mnRemixGalleryGetLastPage(void);
extern sb32 mnRemixGalleryIsActive(void);
/* Gallery.asm initial_: picks the idle picture and its song, or keeps the Gallery song. Call when the Congratulations scene loads,
 * before mnRemixGalleryGetFighterKind. */
extern void mnRemixGalleryPrepareScene(void);
extern s32 mnRemixGalleryGetFighterKind(void);
/* Per-frame input while the gallery is active (Gallery.asm main_): pictures with the stick and the D-pad, A song, START idle mode,
 * L/R/Z drum kit, B leaves.
 * Returns the frames of the fade that must start before the next picture loads, or 0 when no fade starts. */
extern s32 mnRemixGalleryUpdateInput(void);
/* Called when the fade ended: stops a picked song and loads the next picture. */
extern void mnRemixGalleryOnFadeEnd(void);

#endif
