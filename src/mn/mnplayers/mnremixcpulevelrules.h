#ifndef _MNREMIXCPULEVELRULES_H_
#define _MNREMIXCPULEVELRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure CPU level rules of the VS CSS (AI.asm handle_right_arrow_press_, render_right_arrow_, render_10_), so the headless
 * unit tests (tests/remix) can run them. */

#define MNREMIX_CPU_LEVEL_VANILLA_MAX 9
#define MNREMIX_CPU_LEVEL_MAX 10
#define MNREMIX_HANDICAP_MAX 40

/**
 * TRUE when the right arrow may raise a CPU level, that is while the level is below MNREMIX_CPU_LEVEL_MAX. The arrow
 * clamps at the maximum; it does not wrap back to 1.
 */
extern int mnRemixCpuLevelCanRaise(int level);

/**
 * TRUE when a CPU level needs two digits on the CSS, which is only level 10 (the tens digit is "1").
 */
extern int mnRemixCpuLevelNeedsTens(int level);

/**
 * Units digit of a CPU level, the digit drawn after the tens digit: 10 gives 0, levels 1 to 9 give themselves.
 */
extern int mnRemixCpuLevelUnitsDigit(int level);

/**
 * TRUE when the right arrow may raise a human handicap, that is while the value is below MNREMIX_HANDICAP_MAX (AI.asm
 * handle_right_arrow_press_, `lli t0, 0x28` for a human panel).
 */
extern int mnRemixHandicapCanRaise(int value);

/**
 * TRUE when a human handicap value is drawn with a character icon instead of a digit: values 10 and above (AI.asm
 * render_10_._handicap, `slt at, t6, 10`).
 */
extern int mnRemixHandicapNeedsIcon(int value);

/**
 * Index of a handicap icon in the handicap indicator table of AI.asm, for a value from 10 to MNREMIX_HANDICAP_MAX: 10 gives
 * 0. Values below 10 give -1, since they are drawn as digits.
 */
extern int mnRemixHandicapIconIndex(int value);

#endif

#endif
