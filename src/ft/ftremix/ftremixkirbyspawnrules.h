#ifndef _FTREMIXKIRBYSPAWNRULES_H_
#define _FTREMIXKIRBYSPAWNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Kirby Hat a player picks in the CSS panel and of the power flags a fighter spawn sets (src/KirbyHats.asm
 * spawn_with_table_ and kirby_hat_select_, Size.asm initialize_base_multiplier_table_ lines 358-381, use_extended_special_parts_pause_
 * gun branch), so the headless unit tests (tests/remix) can run them without game state. ftremixkirbyhats.c and
 * ftremixkirbypower.c read the game state and call these. */

#define FTREMIX_KIRBYSPAWN_PANEL_NONE 0
#define FTREMIX_KIRBYSPAWN_PANEL_FIGHTER_COUNT 40
#define FTREMIX_KIRBYSPAWN_PANEL_MAGIC 41
#define FTREMIX_KIRBYSPAWN_PANEL_RANDOM 42

/**
 * Remix fighter id behind a Kirby Hat panel value.
 *
 * @param panel_value Panel value, 0 (None) to 42 (Random).
 * @param random_roll Roll in 0..FTREMIX_KIRBYSPAWN_PANEL_FIGHTER_COUNT - 1, read only for the Random value.
 * @return The fighter whose power the hat gives (REMIX_ID_KIRBY for None, REMIX_ID_NONE for the magic hat), or REMIX_ID_KIRBY
 * when a value or a roll is out of range.
 */
extern int ftRemixKirbySpawnPanelFighter(int panel_value, int random_roll);

/**
 * Whether a fighter spawn turns the magic hat of its port on: the spawn power is the magic power or the panel holds '???'.
 *
 * @param spawn_copy_kind Copy id of the spawn description.
 * @param panel_value Kirby Hat panel value of the port.
 * @return 1 when the magic hat starts on.
 */
extern int ftRemixKirbySpawnMagicHatStartsOn(int spawn_copy_kind, int panel_value);

/**
 * Whether Kirby loses his power when hit or taunting: only with no hat chosen in the panel.
 *
 * @param panel_value Kirby Hat panel value of the port.
 * @return 1 when the power can be lost.
 */
extern int ftRemixKirbySpawnTauntLosesPower(int panel_value);

/**
 * File offset, in Kirby's model file, of the held-weapon part table of a hat (use_extended_special_parts_pause_ gun branch).
 *
 * @param hat_id Hat id byte of Kirby (N64 FTStruct 0x0980).
 * @return The offset, or 0 when the hat has no weapon part.
 */
extern unsigned int ftRemixKirbyGunHatOffset(int hat_id);

#endif

#endif
