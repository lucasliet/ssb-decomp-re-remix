#ifndef _IFREMIXRECTANGLE_H_
#define _IFREMIXRECTANGLE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

/*
 * Screen-space translucent rectangle (Render.asm draw_rectangle_ with alpha on), drawn by an interface display GObj.
 * Colour is 0xRRGGBBAA, the same packing the asm stores at object + 0x40.
 */
extern GObj* ifRemixRectangleMake(s32 ulx, s32 uly, s32 width, s32 height, u32 color);
extern void ifRemixRectangleSetColor(GObj *rectangle_gobj, u32 color);
extern u32 ifRemixRectangleGetColor(GObj *rectangle_gobj);
extern void ifRemixRectangleDestroy(GObj *rectangle_gobj);
extern void ifRemixRectangleResetAll(void);

#endif

#endif
