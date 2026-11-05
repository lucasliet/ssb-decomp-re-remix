#ifndef _SC1PREMIXENEMYAFKRULES_H_
#define _SC1PREMIXENEMYAFKRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the "afk" player tag automation (SinglePlayerEnemy.asm _afk_1p, Credits.asm remix_credits_speed_assist_) that need no game
 * state, kept apart so the headless unit tests (tests/remix) run them. The automation is on while the custom player tag number 2 is
 * named exactly "afk" (the asm compares the first word of the tag text with 0x61666B00, that is 'a' 'f' 'k' and a terminator).
 */

/** Custom player tag whose name switches the automation on (CharacterSelectDebugMenu.PlayerTag.string_table + (2 * 4)). */
#define SC1PREMIX_AFK_TAG_NUMBER 2

/** CPU level of the player slot the automation hands to the CPU (_afk_1p._hmn_check). */
#define SC1PREMIX_AFK_BOT_LEVEL 10

/** Staff roll name count of the first Remix name (Credits.asm _check_name_id), where the speed assist applies. */
#define SC1PREMIX_AFK_CREDITS_FIRST_REMIX_NAME_ID 0x55

/**
 * Whether a custom player tag name switches the automation on.
 *
 * @param name Tag text, NULL allowed.
 * @return Non-zero only for the text "afk" (lower case, no other character).
 */
extern int sc1PRemixAfkIsTagName(const char *name);

/**
 * Whether the player slot of the 1P game becomes a CPU of level SC1PREMIX_AFK_BOT_LEVEL (_afk_1p._hmn_check).
 *
 * @param is_afk Non-zero while the automation is on.
 * @param is_player_spawn Non-zero for the spawn of the playing fighter (0x21).
 * @param is_contest Non-zero in the Home-Run Contest, which keeps the human.
 */
extern int sc1PRemixAfkMakesPlayerBot(int is_afk, int is_player_spawn, int is_contest);

/**
 * Whether the credits behave as if Start was pressed (remix_credits_speed_assist_): the automation is on, the roll reached the first
 * Remix name and the roll is not at its normal speed. Pressing Start in that state returns the roll to the normal speed.
 *
 * @param is_afk Non-zero while the automation is on.
 * @param name_id Staff roll name count.
 * @param speed Current roll speed.
 * @param normal_speed Roll speed when Start was not pressed.
 */
extern int sc1PRemixAfkSpeedAssistFires(int is_afk, int name_id, float speed, float normal_speed);

#endif

#endif
