#ifndef _FTREMIXVARIANTTUNERULES_H_
#define _FTREMIXVARIANTTUNERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of small per-fighter tuning patches of Smash Remix, so the headless unit tests (tests/remix) can run them. Only
 * int and float. Each function returns the vanilla value for every fighter the patch does not cover.
 */

#define FTREMIX_TUNE_JDK_SPINNINGKONG_AIR_VEL_Y 18.0F
#define FTREMIX_TUNE_JFALCON_FALCONDIVE_AIR_ACCEL_MUL 1.2F
#define FTREMIX_TUNE_JFALCON_FALCONDIVE_AIR_SPEED_MAX_MUL 0.84F
#define FTREMIX_TUNE_PEPPY_FIREFOX_PITCH_OFFSET 0.0F

/** JDK's Spinning Kong start velocity (JDK.asm spinning_kong_startup_, 0x8015B9C4): 18.0 against 20.3 in the US game. */
extern float ftRemixVariantTuneRulesSpinningKongAirVelY(int fkind, float vanilla_vel_y);

/** J Falcon's Falcon Dive aerial acceleration multiplier (JFalcon.asm up_special_aerial_drift_2_, 0x80160454). */
extern float ftRemixVariantTuneRulesFalconDiveAirAccelMul(int fkind, float vanilla_mul);

/** J Falcon's Falcon Dive aerial max speed multiplier (JFalcon.asm up_special_aerial_drift_1_ and _2_, 0x80160428 and 0x80160454). */
extern float ftRemixVariantTuneRulesFalconDiveAirSpeedMaxMul(int fkind, float vanilla_mul);

/** Pitch offset subtracted from the Firefox model pitch (Peppy.asm up_special_rotation_offset_air_, 0x8015C074): Peppy uses 0, not Fox's 90 degrees. */
extern float ftRemixVariantTuneRulesFireFoxPitchOffset(int fkind, float vanilla_offset);

/**
 * TRUE when the fighter keeps its facing at the start of the entry (Sonic.asm sonic_facing_, 0x8013DC50), FALSE when the entry
 * clears it (fp->lr = 0). The facing is kept by the polygon fighters (NMARIO to NNESS and NWARIO onward), Sonic and Super Sonic.
 */
extern int ftRemixVariantTuneRulesEntryKeepsFacing(int fkind);

#endif

#endif
