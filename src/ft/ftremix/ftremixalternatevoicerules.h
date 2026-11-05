#ifndef _FTREMIXALTERNATEVOICERULES_H_
#define _FTREMIXALTERNATEVOICERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of Wario's alternate neutral special voice (Wario.asm:638 alternate_voice_, hook at 0x800E80FC in ftParamPlayVoice):
 * holding L swaps the voice 0x2FB for 0x300 once, until the flag resets on death or character load. Only int, so the headless
 * unit tests (tests/remix) can run it.
 */

#define FTREMIX_ALTERNATE_VOICE_WARIO_NSP 0x2FB
#define FTREMIX_ALTERNATE_VOICE_WARIO_NSP_ALTERNATE 0x300
#define FTREMIX_ALTERNATE_VOICE_BUTTON_L 0x0020

/**
 * TRUE when a voice play must use the alternate voice and set the used flag.
 *
 * @param voice_id FGM id the fighter is about to play.
 * @param button_hold Buttons held by the fighter this frame (N64 bits).
 * @param is_flag_set Flag word of the fighter (N64 0x0ADC): TRUE once the alternate voice has played.
 */
extern int ftRemixAlternateVoiceShouldSwap(int voice_id, unsigned int button_hold, int is_flag_set);

#endif

#endif
