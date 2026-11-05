/**
 * mnremixcsshighscores.c - 1P record switch of the character selects, see mnremixcsshighscores.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcsshighscores.h"
#include "mn/mnplayers/mnremixcsspanelmenurules.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_toggles.h"
#include "sc/sc1pmode/sc1premixenemy.h"
#include "sc/sc1pmode/sc1premixhighscores.h"

#define HIGH_SCORES_TOGGLE_CAPACITY 96
/* Render.draw_string(0x22, 0x1B, string_high_scores_disabled, ..., 0x43400000, 0x43590000, 0xFF8080FF, 0x3F500000, LEFT), CharacterSelect.asm 4767 */
#define HIGH_SCORES_TEXT_SLOT 96
#define HIGH_SCORES_TEXT_X 192.0F
#define HIGH_SCORES_TEXT_Y 217.0F
#define HIGH_SCORES_TEXT_RGBA 0xFF8080FF
#define HIGH_SCORES_TEXT_SCALE 0.8125F

/** Packs one toggle group the way Toggles.asm stores it in its SRAM block. */
static void mnRemixCssHighScoresPackGroup(s32 group, unsigned int *words, s32 word_count)
{
    s32 values[HIGH_SCORES_TOGGLE_CAPACITY];
    s32 maxima[HIGH_SCORES_TOGGLE_CAPACITY];
    s32 count = remix_toggle_group_snapshot(group, values, maxima, HIGH_SCORES_TOGGLE_CAPACITY);

    mnRemixPanelMenuRulesPackToggles(values, maxima, count, words, word_count);
}

static void mnRemixCssHighScoresReadGlobals(MNRemixPanelMenuHighScoreGlobals *globals)
{
    mnRemixCssHighScoresPackGroup(REMIX_TOGGLE_GROUP_GAMEPLAY, globals->gameplay_words, MNREMIX_PANELMENU_GAMEPLAY_WORDS);
    mnRemixCssHighScoresPackGroup(REMIX_TOGGLE_GROUP_POKEMON, &globals->pokemon_word, 1);
    globals->enemy_port = sc1PRemixEnemyPortGet();
}

sb32 mnRemixCssHighScoresUpdate(s32 mode, const s32 *item_values)
{
    MNRemixPanelMenuHighScoreGlobals globals;
    sb32 is_enabled;

    mnRemixCssHighScoresReadGlobals(&globals);
    is_enabled = (mnRemixPanelMenuRulesHighScoresEnabled(mode, item_values, &globals) != 0) ? TRUE : FALSE;
    sc1PRemixHighScoresSetEnabled(is_enabled);

    if (is_enabled == FALSE)
    {
        remix_hud_text_put(HIGH_SCORES_TEXT_SLOT, "High Scores Disabled", HIGH_SCORES_TEXT_X, HIGH_SCORES_TEXT_Y, HIGH_SCORES_TEXT_RGBA,
                           HIGH_SCORES_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
        return FALSE;
    }
    remix_hud_text_hide(HIGH_SCORES_TEXT_SLOT);
    return TRUE;
}

#endif
