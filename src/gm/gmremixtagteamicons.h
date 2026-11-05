#ifndef _GMREMIXTAGTEAMICONS_H_
#define _GMREMIXTAGTEAMICONS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

/*
 * Stock icons of the Tag Team queue (TagTeam.asm icon_cache): the HUD shows one icon per character left in the queue, so the
 * icons of characters that are not loaded yet are resolved at the match start. Only the main file of a fighter is read for it
 * (its attributes hold the stock sprite and palettes); the files of the fighter itself are still loaded on the swap.
 */

/** Forgets the icons of the previous match. */
extern void gmRemixTagTeamIconsReset(void);

/**
 * Resolves the stock icon of a queue slot.
 *
 * @param port Port of the queue.
 * @param slot Queue slot.
 * @param fkind Character of the slot.
 * @param costume Costume of the slot, picks the icon palette.
 * @param is_classic TRUE for a Classic Sonic slot, whose icon is in the classic main file (TagTeam.asm icon cache).
 */
extern void gmRemixTagTeamIconsSet(s32 port, s32 slot, s32 fkind, s32 costume, sb32 is_classic);

/**
 * Gives a HUD stock sprite the icon of a queue slot.
 *
 * @param sobj Sprite object of the icon.
 * @param port Port of the queue.
 * @param slot Queue slot.
 * @return TRUE when the icon was set, FALSE when the slot has none (the caller keeps the fighter's icon).
 */
extern sb32 gmRemixTagTeamIconsApply(SObj *sobj, s32 port, s32 slot);

#endif

#endif
