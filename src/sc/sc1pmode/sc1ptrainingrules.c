#include "sc/sc1pmode/sc1ptrainingrules.h"

#define STICK_UP_JUMP 0x35
#define STICK_UP_SMASH 0x50
#define STICK_DOWN 0xB0
#define STICK_RIGHT 0x50
#define STICK_LEFT 0xB0
#define SPAM_TIMER_BYTE_MASK 0xFF

typedef struct OosEntry
{
    int32_t button_mask;
    int32_t stick_x;
    int32_t stick_y;

} OosEntry;

/* Training.asm oos_inputs_frame_1; the stick halfwords are x in the high byte and y in the low byte, as signed bytes. */
static const OosEntry sOosFrame1[SC1PTRAININGRULES_OOS_COUNT] =
{
    { SC1PTRAININGRULES_BUTTON_Z, 0, STICK_UP_JUMP },
    { SC1PTRAININGRULES_BUTTON_Z, -80, 0 },
    { SC1PTRAININGRULES_BUTTON_Z, 80, 0 },
    { SC1PTRAININGRULES_BUTTON_Z | SC1PTRAININGRULES_BUTTON_A, 0, 0 },
    { SC1PTRAININGRULES_BUTTON_Z, 0, STICK_UP_JUMP },
    { SC1PTRAININGRULES_BUTTON_Z, 0, STICK_UP_JUMP },
    { SC1PTRAININGRULES_BUTTON_Z, 0, STICK_UP_JUMP },
    { SC1PTRAININGRULES_BUTTON_Z, 0, STICK_UP_JUMP },
    { SC1PTRAININGRULES_BUTTON_Z, 0, -80 },
    { SC1PTRAININGRULES_BUTTON_Z, 0, -80 },
    { SC1PTRAININGRULES_BUTTON_Z, 0, -80 },
    { SC1PTRAININGRULES_BUTTON_Z, 0, 0 }
};

/* Training.asm oos_inputs_frame_2 */
static const OosEntry sOosFrame2[SC1PTRAININGRULES_OOS_COUNT] =
{
    { SC1PTRAININGRULES_BUTTON_Z, 0, STICK_UP_JUMP },
    { SC1PTRAININGRULES_BUTTON_Z, -80, 0 },
    { SC1PTRAININGRULES_BUTTON_Z, 80, 0 },
    { SC1PTRAININGRULES_BUTTON_Z | SC1PTRAININGRULES_BUTTON_A, 0, 0 },
    { SC1PTRAININGRULES_BUTTON_A, 0, STICK_UP_SMASH },
    { SC1PTRAININGRULES_BUTTON_B, 0, STICK_UP_SMASH },
    { SC1PTRAININGRULES_BUTTON_B, 0, -80 },
    { SC1PTRAININGRULES_BUTTON_A, 0, 0 },
    { SC1PTRAININGRULES_BUTTON_Z, 0, -80 },
    { SC1PTRAININGRULES_BUTTON_B, 0, -80 },
    { SC1PTRAININGRULES_BUTTON_A, 0, 0 },
    { SC1PTRAININGRULES_BUTTON_Z, 0, 0 }
};

int32_t sc1PTrainingRulesGetOosInput(int32_t option, int32_t is_second_frame, int32_t *out_stick_x, int32_t *out_stick_y)
{
    const OosEntry *table = (is_second_frame != 0) ? sOosFrame2 : sOosFrame1;
    const OosEntry *entry;

    if (option < 0)
    {
        option = 0;
    }
    if (option >= SC1PTRAININGRULES_OOS_COUNT)
    {
        option = SC1PTRAININGRULES_OOS_COUNT - 1;
    }
    entry = &table[option];
    *out_stick_x = entry->stick_x;
    *out_stick_y = entry->stick_y;

    return entry->button_mask;
}

static void sc1PTrainingRulesSetOutput(SC1PTrainingRulesShieldOut *out, int32_t action, int32_t is_second_frame, int32_t option)
{
    out->action = action;
    out->button_mask = sc1PTrainingRulesGetOosInput(option, is_second_frame, &out->stick_x, &out->stick_y);
}

/*
 * Training.asm _oos: the next status is frame 2 when frame 1 already ran, else frame 1. For Down Special and Neutral Air the
 * second frame waits for the last frame of the shield jumpsquat.
 */
static int32_t sc1PTrainingRulesGetOosStatus(const SC1PTrainingRulesShieldIn *in)
{
    int32_t next = (in->status == SC1PTRAININGRULES_STATUS_OOS_FRAME_1) ? SC1PTRAININGRULES_STATUS_OOS_FRAME_2
                                                                       : SC1PTRAININGRULES_STATUS_OOS_FRAME_1;

    if ((in->oos_option != SC1PTRAININGRULES_OOS_DSP) && (in->oos_option != SC1PTRAININGRULES_OOS_NAIR))
    {
        return next;
    }
    if (in->is_in_shield_jumpsquat == 0)
    {
        return next;
    }
    return (in->is_last_jumpsquat_frame != 0) ? next : SC1PTRAININGRULES_STATUS_OOS_FRAME_1;
}

static void sc1PTrainingRulesDoOos(const SC1PTrainingRulesShieldIn *in, SC1PTrainingRulesShieldOut *out)
{
    out->new_status = sc1PTrainingRulesGetOosStatus(in);
    sc1PTrainingRulesSetOutput(out, SC1PTRAININGRULES_ACTION_OOS_INPUT,
                               (out->new_status == SC1PTRAININGRULES_STATUS_OOS_FRAME_2) ? 1 : 0, in->oos_option);
}

static void sc1PTrainingRulesForceShield(SC1PTrainingRulesShieldOut *out, int32_t refill)
{
    out->action = SC1PTRAININGRULES_ACTION_FORCE_SHIELD;
    out->max_shield = refill;
}

void sc1PTrainingRulesShieldStep(const SC1PTrainingRulesShieldIn *in, SC1PTrainingRulesShieldOut *out)
{
    out->new_status = in->status;
    out->max_shield = 0;
    out->action = SC1PTRAININGRULES_ACTION_NONE;
    out->button_mask = 0;
    out->stick_x = 0;
    out->stick_y = 0;

    if (in->mode == SC1PTRAININGRULES_SHIELD_BREAK_OFF)
    {
        return;
    }
    if (in->mode == SC1PTRAININGRULES_SHIELD_BREAK_INFINITE)
    {
        sc1PTrainingRulesForceShield(out, 1);
        return;
    }
    switch (in->status)
    {
    case SC1PTRAININGRULES_STATUS_SHIELD:
        if (in->is_in_shield_stun != 0)
        {
            out->new_status = SC1PTRAININGRULES_STATUS_STUN;
            sc1PTrainingRulesForceShield(out, 0);
            return;
        }
        sc1PTrainingRulesForceShield(out, 1);
        return;

    case SC1PTRAININGRULES_STATUS_STUN:
        if (in->is_in_shield_stun == 0)
        {
            out->new_status = SC1PTRAININGRULES_STATUS_SHIELD;
            return;
        }
        if (in->is_shield_stun_ending != 0)
        {
            sc1PTrainingRulesDoOos(in, out);
            return;
        }
        sc1PTrainingRulesForceShield(out, 0);
        return;

    case SC1PTRAININGRULES_STATUS_OOS_FRAME_2:
        out->new_status = SC1PTRAININGRULES_STATUS_SHIELD;
        sc1PTrainingRulesSetOutput(out, SC1PTRAININGRULES_ACTION_END_OOS, 1, in->oos_option);
        return;

    default:
        sc1PTrainingRulesDoOos(in, out);
        return;
    }
}

int32_t sc1PTrainingRulesSpamReloadTimer(int32_t is_random_interval, int32_t interval, int32_t random_below_100)
{
    return ((is_random_interval != 0) ? random_below_100 : interval) & SPAM_TIMER_BYTE_MASK;
}

int32_t sc1PTrainingRulesSpamTickTimer(int32_t timer, int32_t is_cpu, int32_t is_in_hitlag)
{
    if ((is_cpu == 0) || (is_in_hitlag != 0) || (timer <= 0))
    {
        return timer;
    }
    return timer - 1;
}
