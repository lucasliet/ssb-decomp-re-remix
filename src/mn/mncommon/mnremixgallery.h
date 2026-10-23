#ifndef _MNREMIXGALLERY_H_
#define _MNREMIXGALLERY_H_

#include <ssb_types.h>

/* Starts the Remix Gallery on the Congratulations scene at its first picture.
 * B returns to previous_scene. */
extern void mnRemixGalleryActivate(s32 previous_scene);
extern sb32 mnRemixGalleryIsActive(void);
extern s32 mnRemixGalleryGetFighterKind(void);
/* Per-frame input while the gallery is active (L/R, D-pad, stick, B). */
extern void mnRemixGalleryUpdateInput(void);

#endif
