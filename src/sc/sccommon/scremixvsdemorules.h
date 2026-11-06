#ifndef _SCREMIXVSDEMORULES_H_
#define _SCREMIXVSDEMORULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * State-free rules of the Remix VS demo fighter pick (VsDemo.asm enable_nonfocused_remix_chars_), kept apart so the headless
 * unit tests (tests/remix) can run them.
 */

/** Demo player index (port 4) whose fighter must also differ from the one of the player before it (VsDemo.asm `bne t0, s1`). */
#define SC_REMIX_VS_DEMO_PORT_4_PLAYER 3

/**
 * Whether a drawn fighter must be drawn again because port 4 would repeat port 3's fighter.
 *
 * @param player Demo player index being filled, 2 or 3.
 * @param fkind Fighter kind just drawn.
 * @param port_3_fkind Fighter kind already given to demo player index 2.
 * @return Non-zero when player is port 4 and fkind equals port_3_fkind.
 */
extern int scRemixVsDemoRepeatsPort3Fighter(int player, int fkind, int port_3_fkind);

#endif

#endif
