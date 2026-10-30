#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaicloudrules.h"

/** Goemon USP actions: Magic Cloud Ride, Attack, Jump and Escape (usp_check_ has all four). */
int ftRemixAiCloudIsGoemonUspAction(int action)
{
    return (action == FTREMIX_AI_CLOUD_ACTION_RIDE) || (action == FTREMIX_AI_CLOUD_ACTION_ATTACK) ||
           (action == FTREMIX_AI_CLOUD_ACTION_JUMP) || (action == FTREMIX_AI_CLOUD_ACTION_ESCAPE);
}

/** Ebisumaru USP action: only Magic Cloud Ride. */
int ftRemixAiCloudIsEbisumaruUspAction(int action)
{
    return action == FTREMIX_AI_CLOUD_ACTION_RIDE;
}

/** Goemon stick-up actions of recovery_stick_y_fix: Ride and Attack. Jump is not in that list in the asm. */
int ftRemixAiCloudIsGoemonStickUpAction(int action)
{
    return (action == FTREMIX_AI_CLOUD_ACTION_RIDE) || (action == FTREMIX_AI_CLOUD_ACTION_ATTACK);
}

/** Keeps the low byte of button_inputs and sets its high byte. */
int ftRemixAiCloudReplaceButtonHighByte(int button_inputs, int high_byte)
{
    return (button_inputs & 0x00FF) | ((high_byte & 0xFF) << 8);
}

#endif
