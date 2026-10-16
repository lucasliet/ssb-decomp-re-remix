#ifndef _GRREMIXHARDCODING_H_
#define _GRREMIXHARDCODING_H_

/*
 * Mushroom Kingdom Remix offsets of the vanilla Inishie map tables (Hazards.asm mk_remix_hardcoding_1..8).
 * The Remix map file lays its tables out differently, so the vanilla makers pick the Remix offset on that stage only.
 */

#include <ssb_types.h>

#ifdef REGION_REMIX
#define GRREMIX_MK_OFFSET(vanilla_offset, remix_offset) grRemixSelectMKOffset((intptr_t)(vanilla_offset), (intptr_t)(remix_offset))
#else
#define GRREMIX_MK_OFFSET(vanilla_offset, remix_offset) (vanilla_offset)
#endif

/** Returns remix_offset when the current stage is Mushroom Kingdom Remix, vanilla_offset otherwise. */
intptr_t grRemixSelectMKOffset(intptr_t vanilla_offset, intptr_t remix_offset);

#endif
