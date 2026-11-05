#ifndef _MNREMIXTITLEVERSION_H_
#define _MNREMIXTITLEVERSION_H_

#include <ssb_types.h>

/* Boot.asm string_version, drawn by Boot.draw_version_on_title_screen_ (Render.asm _title branch). */
#define MN_REMIX_TITLE_VERSION_TEXT "Smash Remix v2.0.1"

/* Slot of remix_hud_text (port/remix/remix_hud_text.h) taken by the title screen; no other label is drawn in that scene. */
#define MN_REMIX_TITLE_VERSION_SLOT 3

/* Boot.asm:160 Render.draw_string arguments: X 0x43200000, Y 0x435A0000, color 0x888800FF, scale 0x3F700000. */
#define MN_REMIX_TITLE_VERSION_X 160.0F
#define MN_REMIX_TITLE_VERSION_Y 218.0F
#define MN_REMIX_TITLE_VERSION_RGBA 0x888800FFU
#define MN_REMIX_TITLE_VERSION_SCALE 0.9375F

/**
 * Keeps the "Smash Remix v2.0.1" text of the title screen visible; call once per frame while the title runs.
 */
extern void mnRemixTitleVersionPut(void);

#endif
