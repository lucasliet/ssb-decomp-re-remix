#ifndef _SC1PTRAININGREMIXLEGEND_H_
#define _SC1PTRAININGREMIXLEGEND_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * D-pad legend of the Remix Training pause (Training.asm setup_ 1621-1632, run_ 1956-1966): four D-pad glyphs with a yellow
 * square on the direction each line uses, and the lines Toggle Pause, Quick Reset, Frame Advance and Model Display. Group 0x19
 * holds the lines of the toggles and group 0x1A the Quick Reset line; the "D-pad Controls" setting decides which show.
 */

/** Scene start: makes the D-pad glyphs of both groups. */
extern void sc1PTrainingRemixLegendMake(void);

/** Every frame: shows the groups the setting allows while the match is paused and puts their text and squares. */
extern void sc1PTrainingRemixLegendUpdate(void);

#endif

#endif
