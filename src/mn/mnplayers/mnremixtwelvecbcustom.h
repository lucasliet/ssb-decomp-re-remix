#ifndef _MNREMIXTWELVECBCUSTOM_H_
#define _MNREMIXTWELVECBCUSTOM_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * The Custom character set of the 12-Character Battle character select (TwelveCharBattle.asm check_input_ 4056-4464): a set each
 * player edits while holding the token over one of his portraits. Each player owns 12 entries, three rows of the four characters
 * his half shows; the other half of a row repeats the same four. Pure so the headless unit tests (tests/remix) run it.
 */

#define MNREMIX_TCB_CUSTOM_SET 4
#define MNREMIX_TCB_CUSTOM_PRESETS 4
#define MNREMIX_TCB_CUSTOM_ENTRIES 12

/** Frames the D-pad up has to be held to copy the opponent's set instead of changing the preset (copy_timer 60). */
#define MNREMIX_TCB_CUSTOM_COPY_FRAMES 60
/** Z or R held: the first press cycles at once, then every 8 frames after a 30 frame delay (portrait_cycle_timer). */
#define MNREMIX_TCB_CUSTOM_REPEAT_DELAY 30
#define MNREMIX_TCB_CUSTOM_REPEAT_STEP 8
#define MNREMIX_TCB_CUSTOM_REPEAT_IDLE (-1)

typedef enum MNRemixTwelveCBCustomDpadUp
{
    MNREMIX_TCB_CUSTOM_DPAD_UP_NONE,
    MNREMIX_TCB_CUSTOM_DPAD_UP_PRESET,
    MNREMIX_TCB_CUSTOM_DPAD_UP_COPY
} MNRemixTwelveCBCustomDpadUp;

/** Puts both players on the Default set with the first preset next, as at the start of the program. */
extern void mnRemixTwelveCBCustomReset(void);

/** Index (0..11) in a player's table of the portrait slot, -1 for an invalid slot; the two halves of a row share it. */
extern int mnRemixTwelveCBCustomIndexOfSlot(int slot);

/** Fighter kind the Custom set shows in a slot (the table of the player that owns the slot), -1 for an invalid slot. */
extern int mnRemixTwelveCBCustomFkind(int slot);

/** Fighter kind a player's own Custom table holds for a portrait slot (the table has all 24 slots, both halves of a row alike), -1 for an invalid player or slot. */
extern int mnRemixTwelveCBCustomFkindOf(int player, int slot);

/**
 * Moves the character of a portrait to the next or the previous one of the roster, skipping one entry (the Sandbag).
 *
 * @param slot Portrait slot under the token.
 * @param direction -1 for Z (previous), 1 for R (next).
 * @param roster Fighter kinds in menu order.
 * @param roster_count Entries of roster.
 * @param skip_fkind Entry that is never taken.
 * @return 1 when the portrait changed, 0 when the slot is invalid or the roster is empty.
 */
extern int mnRemixTwelveCBCustomCycle(int slot, int direction, const int *roster, int roster_count, int skip_fkind);

/** L released: every portrait of the player takes the character. */
extern void mnRemixTwelveCBCustomApplyAll(int player, int fkind);

/** D-pad up released early: the player's set becomes the next preset (Default, Japanese, Polygon, Remix, in turn). */
extern void mnRemixTwelveCBCustomPreset(int player);

/** D-pad up held 60 frames: the player's set becomes a copy of the opponent's. */
extern void mnRemixTwelveCBCustomCopyOpponent(int player);

/**
 * D-pad down: every portrait of the player takes a random character.
 *
 * @param pick Returns a fighter kind each call.
 */
extern void mnRemixTwelveCBCustomRandomize(int player, int (*pick)(void));

/**
 * Timer of the Z and R repeat.
 *
 * @param timer Per player timer; MNREMIX_TCB_CUSTOM_REPEAT_IDLE while neither button was held.
 * @param is_held Non-zero while Z or R is held.
 * @return 1 when the portrait must cycle this frame.
 */
extern int mnRemixTwelveCBCustomRepeat(int *timer, int is_held);

/**
 * D-pad up: a short press changes the preset, holding it copies the opponent.
 *
 * @param timer Per player frames the button has been held.
 * @param is_held Non-zero while D-pad up is held.
 * @param is_released Non-zero the frame it is released.
 * @return The action of this frame; a release after the copy does nothing, so the copy does not send an extra press.
 */
extern MNRemixTwelveCBCustomDpadUp mnRemixTwelveCBCustomDpadUp(int *timer, int is_held, int is_released);

#endif

#endif
