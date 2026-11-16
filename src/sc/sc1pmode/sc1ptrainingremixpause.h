#ifndef _SC1PTRAININGREMIXPAUSE_H_
#define _SC1PTRAININGREMIXPAUSE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/objtypes.h>
#include <ssb_types.h>

/*
 * The Training pause any controller may open and close (Training.asm check_enter_menu_ 626, check_leave_menu_ 673,
 * check_disabled_item_spawn_ 720, load_from_reset_ 880-890). The port that pressed START becomes gSCManagerSceneData.player while
 * the menu is up, so the vanilla menu reads its controller; the owner (the human of the character select) is remembered and put
 * back when the menu closes, on a reset and on the exit.
 */

/**
 * Port whose controller pressed one of `buttons` this frame, the port that has the menu first.
 *
 * @return The port, or -1 when no controller pressed.
 */
extern s32 sc1PTrainingRemixPauseFindPort(u16 buttons);

/** Gives the menu to `port` for this pause and remembers the owner. Does nothing when `port` already has it. */
extern void sc1PTrainingRemixPauseTakeMenu(s32 port);

/**
 * Gives the menu back to the owner.
 *
 * @return The owner port.
 */
extern s32 sc1PTrainingRemixPauseReturnMenu(void);

/** The owner port, also while another port has the menu. */
extern s32 sc1PTrainingRemixPauseGetOwner(void);

/**
 * Fighter the pause menu acts on (item spawn point, close-up view): the fighter of the port that has the menu, or the owner's
 * when that port has none (check_disabled_item_spawn_).
 */
extern GObj* sc1PTrainingRemixPauseGetFighter(void);

#endif

#endif
