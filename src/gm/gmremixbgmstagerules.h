#ifndef _GMREMIXBGMSTAGERULES_H_
#define _GMREMIXBGMSTAGERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the stage song pick (BGM.asm apply_alternate_music_ and alternate_music_): the C buttons held at the stage
 * load, the alternate roll, the Salty Runback preservation and the VS demo Final Destination skip. Separate from the engine
 * so the headless unit tests (tests/remix) can run them.
 */

/* N64 controller masks of the four C buttons (Joypad.asm CU, CD, CL, CR). */
#define GMREMIX_BGM_STAGE_C_UP 0x0008
#define GMREMIX_BGM_STAGE_C_DOWN 0x0004
#define GMREMIX_BGM_STAGE_C_LEFT 0x0002
#define GMREMIX_BGM_STAGE_C_RIGHT 0x0001

/* Slot a C button forces. Slots 0, 1 and 2 are the occasional, rare and rare 2 tracks of the alternate table. */
#define GMREMIX_BGM_STAGE_SLOT_NONE (-2)
#define GMREMIX_BGM_STAGE_SLOT_DEFAULT (-1)
#define GMREMIX_BGM_STAGE_SLOT_OCCASIONAL 0
#define GMREMIX_BGM_STAGE_SLOT_RARE 1
#define GMREMIX_BGM_STAGE_SLOT_RARE_2 2

/* BGM.asm alternate_music_ CHANCE_MAIN, CHANCE_OCCASIONAL, CHANCE_RARE and the roll range (CHANCE_RARE_2 takes the rest). */
#define GMREMIX_BGM_STAGE_ROLL_RANGE 100
#define GMREMIX_BGM_STAGE_CHANCE_MAIN 65
#define GMREMIX_BGM_STAGE_CHANCE_OCCASIONAL 16
#define GMREMIX_BGM_STAGE_CHANCE_RARE 10

/* BGM.stage MASTER_HAND_1 (the Final Destination intro) and FINAL_DESTINATION. */
#define GMREMIX_BGM_STAGE_FD_INTRO 24
#define GMREMIX_BGM_STAGE_FD_MAIN 25

/** Song of the stage pick and whether Random Music may still replace it. */
typedef struct GMRemixBgmStagePick
{
    int track;
    int random_disabled;

} GMRemixBgmStagePick;

/**
 * Slot the C buttons held on any controller force (apply order: C-Up, C-Left, C-Right, C-Down).
 *
 * @param held_buttons Held buttons of the four controllers, ORed.
 * @return GMREMIX_BGM_STAGE_SLOT_DEFAULT for C-Up, SLOT_OCCASIONAL for C-Left, SLOT_RARE for C-Right, SLOT_RARE_2 for C-Down,
 *         GMREMIX_BGM_STAGE_SLOT_NONE when none is held.
 */
extern int gmRemixBgmStageRulesCButtonSlot(unsigned int held_buttons);

/**
 * Slot of the alternate roll (65 percent main, 16 occasional, 10 rare, 9 rare 2).
 *
 * @param roll Random number below GMREMIX_BGM_STAGE_ROLL_RANGE.
 * @return GMREMIX_BGM_STAGE_SLOT_DEFAULT for the main song, else the slot rolled.
 */
extern int gmRemixBgmStageRulesRollSlot(int roll);

/**
 * Whether the pick consumes the alternate roll (no C button is held and no song is preserved).
 *
 * @param forced_slot Result of gmRemixBgmStageRulesCButtonSlot.
 * @param preserved_track Song a Salty Runback preserves, GMREMIX_BGM_NO_TRACK when the song is not preserved.
 */
extern int gmRemixBgmStageRulesNeedsRoll(int forced_slot, int preserved_track);

/**
 * Picks the stage song before Random Music (BGM.asm alternate_music_). A C button forces its slot and blocks Random Music
 * unless the stage has no track in that slot; a preserved song replaces the roll; otherwise the roll chooses the main or an
 * alternate song, and an empty slot plays the main song.
 *
 * @param default_track Main song of the stage.
 * @param alternates Occasional, rare and rare 2 tracks, GMREMIX_BGM_NO_TRACK for an empty slot.
 * @param forced_slot Result of gmRemixBgmStageRulesCButtonSlot.
 * @param preserved_track Song a Salty Runback preserves, GMREMIX_BGM_NO_TRACK when the song is not preserved.
 * @param roll Random number below GMREMIX_BGM_STAGE_ROLL_RANGE, used only when gmRemixBgmStageRulesNeedsRoll is non-zero.
 */
extern GMRemixBgmStagePick gmRemixBgmStageRulesPick(int default_track, const int *alternates, int forced_slot,
                                                    int preserved_track, int roll);

/**
 * Song the VS demo uses for a stage whose file song is the Final Destination intro: the demo skips the intro and plays the
 * main song (apply_alt_or_random_music_).
 *
 * @param file_track Song in the header of the stage file.
 */
extern int gmRemixBgmStageRulesDemoTrack(int file_track);

#endif

#endif
