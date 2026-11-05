#ifndef _SC1PTRAININGREMIXMENU_H_
#define _SC1PTRAININGREMIXMENU_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Remix Training custom menu, the Z menu (Training.asm run_ 1795-2090, Menu.asm update_, info at 3753, entries listed in
 * sc1ptrainingmenuentries.h). Z opens it from the pause, B closes it. The stick and the C buttons move the cursor and change values
 * (the D-pad keeps its quick shortcuts), R and Z turn the pages, A or START runs the action of an entry. While it is open the
 * vanilla pause menu is hidden and does not update, the role of Training.toggle_menu = CUSTOM_UP.
 */

/** Scene start (Training.asm setup_): the menu is closed and the cursor is on the first entry of the first page. */
extern void sc1PTrainingRemixMenuStartScene(void);

/** TRUE while the Z menu is open (Training.toggle_menu == CUSTOM_UP). The vanilla pause menu must not update then. */
extern sb32 sc1PTrainingRemixMenuIsOpen(void);

/** Every frame: opens the menu on Z during the pause, runs it while open, and puts its text on screen. */
extern void sc1PTrainingRemixMenuUpdate(void);

/** TRUE when "Preserve Menu Selections" is on: a reset keeps the CPU, Item, Speed and cursor of the vanilla pause menu. */
extern sb32 sc1PTrainingRemixMenuIsPreserved(void);

/** Selected entry (0 based, the chain order of the menu). */
extern s32 sc1PTrainingRemixMenuGetSelection(void);

/**
 * Value the menu shows for an entry of the port the Port entry names.
 *
 * @param slot SC1PTrainingMenuSlot.
 */
extern s32 sc1PTrainingRemixMenuGetValue(s32 slot);

#endif

#endif
