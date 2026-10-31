#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaipostsheikrules.h"

#define FTREMIX_AI_SHEIK_USPG_BEGIN 0x0E4
#define FTREMIX_AI_SHEIK_USPG_MOVE 0x0E5
#define FTREMIX_AI_SHEIK_USPA_BEGIN 0x0E7
#define FTREMIX_AI_SHEIK_USPA_MOVE 0x0E8
#define FTREMIX_AI_SHEIK_DSP_BEGIN 0x0F0
#define FTREMIX_AI_SHEIK_NO_FLOOR_LINE (-1)

/** Sheik/AI/Attacks.asm teleport_down: the four USP actions, grounded or aerial, begin or move. */
int ftRemixAiSheikIsTeleportAction(int action)
{
    return (action == FTREMIX_AI_SHEIK_USPG_BEGIN) || (action == FTREMIX_AI_SHEIK_USPG_MOVE) ||
           (action == FTREMIX_AI_SHEIK_USPA_BEGIN) || (action == FTREMIX_AI_SHEIK_USPA_MOVE);
}

/** Sheik/AI/Attacks.asm cpu_post_process: DSP_BEGIN is the only action routed to dsp_control. */
int ftRemixAiSheikIsDspAction(int action)
{
    return action == FTREMIX_AI_SHEIK_DSP_BEGIN;
}

/** Sheik/AI/Attacks.asm teleport_down: no ground below (-1) skips the steer. */
int ftRemixAiSheikTeleportHasGroundBelow(int floor_line_id)
{
    return floor_line_id != FTREMIX_AI_SHEIK_NO_FLOOR_LINE;
}

/** Sheik/AI/Attacks.asm dsp_control: the comparison is less than or equal, so exactly 600 still presses B. */
int ftRemixAiSheikDspIsInRange(float distance)
{
    return distance <= FTREMIX_AI_SHEIK_DSP_PRESS_RANGE;
}

#endif
