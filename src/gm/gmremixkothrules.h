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

/* Colour tags of KingOfTheHill.colors and team_colors (Color.tag), in the order of the team colours. */
#define GMREMIX_KOTHRULES_TAG_RED 0
#define GMREMIX_KOTHRULES_TAG_BLUE 1
#define GMREMIX_KOTHRULES_TAG_GREEN 2
#define GMREMIX_KOTHRULES_TAG_YELLOW 3
#define GMREMIX_KOTHRULES_TAG_GRAY 4

/**
 * Colour tag of a score key (render_ground_gfx_ and the fighter glow of track_score_): free-for-all uses KingOfTheHill.colors
 * (red, blue, yellow, green by port) and gray for a CPU (use_cpu_color); teams use team_colors (red, blue, green, yellow) and
 * never gray.
 *
 * @param key Team (the port in free-for-all) 0..3.
 * @param is_team_battle Non-zero in a team battle.
 * @param is_cpu Non-zero when the free-for-all fighter is a CPU.
 */
extern int gmRemixKothRulesTag(int key, int is_team_battle, int is_cpu);

/** Color.tag value (0xRRGGBBAA) of a GMREMIX_KOTHRULES_TAG_*. */
extern unsigned int gmRemixKothRulesTagColor(int tag);

/**
 * Gray channel of the hill while the contest buffer runs (render_ground_gfx_ _set_gray_color): CONTESTED_GRAY (0xAA per
 * channel) minus twice the buffer, so the hill starts dark when it is contested and lightens as the buffer runs out.
 *
 * @param buffer Frames left of the contest buffer, 1..30.
 */
extern int gmRemixKothRulesContestedGray(int buffer);

#endif

#endif
