#ifndef _MNREMIXCSSAFKRULES_H_
#define _MNREMIXCSSAFKRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the "afk" player tag automation on the VS character select (CharacterSelect.asm awayMode): once the VS CSS is ready to
 * fight for 120 frames with no button held, START is pressed by itself.
 */

/** Frames the CSS must be ready before START is pressed (_handle_css: afk_timer < 120 waits). */
#define MNREMIX_CSS_AFK_START_FRAMES 120

/** Frames after the CSS opened before the ready time counts (_css_ready_wait: gMnBattleTotalTimeTics < 61 clears the timer). */
#define MNREMIX_CSS_AFK_OPEN_FRAMES 61

/** Mask of the buttons whose hold cancels the automation: every button but START (_handle_css: 0xEFFF). */
#define MNREMIX_CSS_AFK_BUTTON_MASK 0xEFFF

/**
 * Value of the ready timer after one frame (_css_ready_wait): cleared while the CSS is not ready or was just opened, else one more.
 *
 * @param timer Timer before the frame.
 * @param is_ready Non-zero when the CSS is ready to fight.
 * @param total_tics Frames since the CSS opened.
 */
extern int mnRemixCssAfkRulesNextTimer(int timer, int is_ready, int total_tics);

/**
 * Whether the automation presses START this frame (_handle_css).
 *
 * @param timer Ready timer.
 * @param is_button_held Non-zero while any button but START is held on any controller.
 */
extern int mnRemixCssAfkRulesShouldStart(int timer, int is_button_held);

#endif

#endif
