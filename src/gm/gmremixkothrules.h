#ifndef _GMREMIXKOTHRULES_H_
#define _GMREMIXKOTHRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * King of the Hill king rules (KingOfTheHill.asm track_score_), pure so the headless unit tests (tests/remix) can run them.
 */

#define GMREMIX_KOTHRULES_NO_KING (-1)
#define GMREMIX_KOTHRULES_CONTESTED (-2)

/** One fighter of a frame: whether it stands on the hill, whom it holds and the score key (team) it counts for. */
typedef struct GMRemixKothRulesFighter
{
    int is_on_hill;
    int holds_enemy;
    int key;

} GMRemixKothRulesFighter;

/**
 * King candidate of a frame: the key all the free fighters on the hill share, NO_KING with nobody on it and CONTESTED when two keys
 * share it or a fighter on it holds an enemy.
 *
 * @param fighters Fighters of the match.
 * @param count How many.
 * @param eligible Receives 1 per fighter that is on the hill and holds no enemy (count entries).
 */
extern int gmRemixKothRulesCandidate(const GMRemixKothRulesFighter *fighters, int count, int *eligible);

/**
 * Lets a contested hill keep the old king for a while: a contest sets the buffer, and the candidate only takes the hill when the
 * buffer ran out.
 *
 * @param candidate Candidate of the frame.
 * @param buffer Frames of the buffer, updated.
 * @param buffer_frames Length the buffer gets on a contest.
 * @return The king the frame settles on, which is CONTESTED while the buffer runs.
 */
extern int gmRemixKothRulesSettle(int candidate, int *buffer, int buffer_frames);

/* Toggles.entry_dragon_king_hud values: 0 default (Dragon King stages only), 1 always, 2 never. */
#define GMREMIX_KOTHRULES_DK_HUD_DEFAULT 0
#define GMREMIX_KOTHRULES_DK_HUD_ALWAYS 1
#define GMREMIX_KOTHRULES_DK_HUD_OFF 2

/**
 * Vertical position of the score meters (KingOfTheHill.asm setup_): the frame sprite and the bar rectangle sit 8 pixels apart,
 * at the bottom of the screen, or at the top on the Dragon King HUD position.
 *
 * @param dragon_king_hud Toggles.entry_dragon_king_hud.
 * @param is_dragon_king_stage Non-zero on Dragon King or Dragon King Remix.
 * @param frame_y Receives the Y of the frame sprite.
 * @param bar_y Receives the Y of the bar and flash rectangles.
 */
extern void gmRemixKothRulesMeterY(int dragon_king_hud, int is_dragon_king_stage, float *frame_y, float *bar_y);

/**
 * Left edge of the bar of a port: 40 pixels plus 70 per port. The frame sprite sits 3 pixels to the left of it.
 *
 * @param port Port 0..3 (the port, not the position among the active players).
 */
extern float gmRemixKothRulesMeterX(int port);

#endif

#endif
