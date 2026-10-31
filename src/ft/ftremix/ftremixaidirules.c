#include "ft/ftremix/ftremixaidirules.h"

#define DI_NOT_SET (-1)
#define DI_FIRST_HIT_MIN_FRAME 10
#define DI_EARLY_HIT_LIMIT 2
#define DI_LEVEL_10 10
#define DI_ADVANCED_MIN_LEVEL 4
#define DI_ADVANCED_MEDIUM_MIN_LEVEL 7
#define DI_ADVANCED_CHOICES 9
#define DI_SMASH_ROUTINES_PER_ROW 4
#define DI_TYPE_CHOICES 2
#define DI_STRENGTH_CHOICES 3
#define DI_DIRECTION_CHOICES 4

#define STICK_POSITIVE 0x50
#define STICK_NEGATIVE 0xB0
#define STICK_CENTER 0x00
#define STICK_Y_COMMAND 0xB0
#define STICK_X_COMMAND 0xA0
#define SCRIPT_END 0xFF

static void ftRemixAiDiSetRoutine(FTRemixAiDiOut *out, int32_t is_slide, int32_t strength, int32_t direction)
{
    out->has_routine = 1;
    out->is_slide = is_slide;
    out->strength = strength;
    out->direction = direction;
}

/* Index 1 to 8 of the advanced AI roll: 1 to 4 smash DI, 5 to 8 slide DI, one per direction. */
static void ftRemixAiDiSetAdvancedRoutine(FTRemixAiDiOut *out, int32_t strength, int32_t advanced_index)
{
    int32_t index = advanced_index - 1;

    if (index < DI_SMASH_ROUTINES_PER_ROW)
    {
        ftRemixAiDiSetRoutine(out, 0, strength, index);
        return;
    }
    ftRemixAiDiSetRoutine(out, 1, strength, index - DI_SMASH_ROUTINES_PER_ROW);
}

static int32_t ftRemixAiDiGetAdvancedIndex(const int8_t *saved_params, int8_t *params, FTRemixAiDiRandomFn random_below)
{
    int32_t index = saved_params[FTREMIXAIDI_PARAM_ADVANCED];

    if (index >= 0)
    {
        return index;
    }
    index = random_below(DI_ADVANCED_CHOICES);
    params[FTREMIXAIDI_PARAM_ADVANCED] = (int8_t)index;

    return index;
}

/* Reads a saved menu-random parameter, or rolls and stores it when the combo has none yet. */
static int32_t ftRemixAiDiGetSavedOrRandom(const int8_t *saved_params, int8_t *params, int32_t slot, int32_t choices,
                                           FTRemixAiDiRandomFn random_below)
{
    int32_t value = saved_params[slot];

    if (value >= 0)
    {
        return value;
    }
    value = random_below(choices);
    params[slot] = (int8_t)value;

    return value;
}

static int32_t ftRemixAiDiResolveAwayToward(int32_t direction, int32_t is_facing_right)
{
    int32_t is_toward;

    if (direction < FTREMIXAIDI_MENU_DIRECTION_AWAY - 1)
    {
        return direction;
    }
    is_toward = direction - (FTREMIXAIDI_MENU_DIRECTION_AWAY - 1);

    return (is_toward == ((is_facing_right != 0) ? 1 : 0)) ? FTREMIXAIDI_DIRECTION_RIGHT : FTREMIXAIDI_DIRECTION_LEFT;
}

static void ftRemixAiDiDecideTraining(const FTRemixAiDiIn *in, const int8_t *saved_params, int8_t *params,
                                      FTRemixAiDiRandomFn random_below, FTRemixAiDiOut *out)
{
    int32_t is_slide;
    int32_t strength;
    int32_t direction;

    if (in->menu_type == FTREMIXAIDI_MENU_TYPE_NONE)
    {
        return;
    }
    is_slide = in->menu_type - (FTREMIXAIDI_MENU_TYPE_RANDOM + 1);
    if (is_slide < 0)
    {
        is_slide = ftRemixAiDiGetSavedOrRandom(saved_params, params, FTREMIXAIDI_PARAM_TYPE, DI_TYPE_CHOICES, random_below);
    }
    strength = in->menu_strength - 1;
    if (strength < 0)
    {
        strength = ftRemixAiDiGetSavedOrRandom(saved_params, params, FTREMIXAIDI_PARAM_STRENGTH, DI_STRENGTH_CHOICES, random_below);
    }
    direction = in->menu_direction - 1;
    if (direction < 0)
    {
        direction = ftRemixAiDiGetSavedOrRandom(saved_params, params, FTREMIXAIDI_PARAM_DIRECTION, DI_DIRECTION_CHOICES, random_below);
    }
    ftRemixAiDiSetRoutine(out, (is_slide != 0) ? 1 : 0, strength, ftRemixAiDiResolveAwayToward(direction, in->is_facing_right));
}

static void ftRemixAiDiDecideLevel10(const int8_t *saved_params, int8_t *params, FTRemixAiDiRandomFn random_below, FTRemixAiDiOut *out)
{
    int32_t advanced_index = ftRemixAiDiGetAdvancedIndex(saved_params, params, random_below);

    if (advanced_index == 0)
    {
        return;
    }
    ftRemixAiDiSetAdvancedRoutine(out, FTREMIXAIDI_STRENGTH_HIGH, advanced_index);
}

static void ftRemixAiDiDecideAdvanced(const FTRemixAiDiIn *in, const int8_t *saved_params, int8_t *params,
                                      FTRemixAiDiRandomFn random_below, FTRemixAiDiOut *out)
{
    int32_t advanced_index;

    if (in->uses_advanced_ai == 0)
    {
        return;
    }
    advanced_index = ftRemixAiDiGetAdvancedIndex(saved_params, params, random_below);
    if ((advanced_index == 0) || (in->cpu_level < DI_ADVANCED_MIN_LEVEL))
    {
        return;
    }
    ftRemixAiDiSetAdvancedRoutine(out, (in->cpu_level < DI_ADVANCED_MEDIUM_MIN_LEVEL) ? FTREMIXAIDI_STRENGTH_LOW : FTREMIXAIDI_STRENGTH_MEDIUM,
                                  advanced_index);
}

static int32_t ftRemixAiDiCanStart(const FTRemixAiDiIn *in)
{
    if (in->is_in_hitlag == 0)
    {
        return 0;
    }
    if (in->is_training != 0)
    {
        return (in->hit_count >= in->menu_first_hit) ? 1 : 0;
    }
    if ((in->hit_count < DI_EARLY_HIT_LIMIT) && (in->status_frame < DI_FIRST_HIT_MIN_FRAME))
    {
        return 0;
    }
    return 1;
}

void ftRemixAiDiDecide(const FTRemixAiDiIn *in, const int8_t *saved_params, FTRemixAiDiRandomFn random_below, FTRemixAiDiOut *out)
{
    int8_t params[FTREMIXAIDI_PARAM_COUNT];
    int32_t i;

    for (i = 0; i < FTREMIXAIDI_PARAM_COUNT; i++)
    {
        params[i] = (in->hit_count != 0) ? saved_params[i] : (int8_t)DI_NOT_SET;
    }
    out->has_routine = 0;
    out->is_slide = 0;
    out->strength = 0;
    out->direction = 0;

    if ((ftRemixAiDiCanStart(in) != 0) && (in->is_damage_status != 0))
    {
        if (in->is_training != 0)
        {
            ftRemixAiDiDecideTraining(in, saved_params, params, random_below, out);
        }
        else if (in->cpu_level == DI_LEVEL_10)
        {
            ftRemixAiDiDecideLevel10(saved_params, params, random_below, out);
        }
        else
        {
            ftRemixAiDiDecideAdvanced(in, saved_params, params, random_below, out);
        }
    }
    for (i = 0; i < FTREMIXAIDI_PARAM_COUNT; i++)
    {
        out->params[i] = params[i];
    }
}

static void ftRemixAiDiGetStick(int32_t direction, int32_t *out_x, int32_t *out_y)
{
    *out_x = STICK_CENTER;
    *out_y = STICK_CENTER;
    switch (direction)
    {
    case FTREMIXAIDI_DIRECTION_LEFT:
        *out_x = STICK_NEGATIVE;
        break;

    case FTREMIXAIDI_DIRECTION_RIGHT:
        *out_x = STICK_POSITIVE;
        break;

    case FTREMIXAIDI_DIRECTION_UP:
        *out_y = STICK_POSITIVE;
        break;

    default:
        *out_y = STICK_NEGATIVE;
        break;
    }
}

static int32_t ftRemixAiDiGetWait(int32_t strength)
{
    switch (strength)
    {
    case FTREMIXAIDI_STRENGTH_HIGH:
        return 1;

    case FTREMIXAIDI_STRENGTH_MEDIUM:
        return 3;

    default:
        return 5;
    }
}

static int32_t ftRemixAiDiPutStick(uint8_t *script, int32_t length, int32_t command, int32_t wait, int32_t value)
{
    script[length++] = (uint8_t)(command | wait);
    script[length++] = (uint8_t)value;

    return length;
}

/* Smash DI: tap the stick for `wait` frames, then release it for the same time (di_input, non slide branch). */
static int32_t ftRemixAiDiBuildSmash(int32_t wait, int32_t stick_x, int32_t stick_y, uint8_t *script)
{
    int32_t length = 0;

    length = ftRemixAiDiPutStick(script, length, STICK_Y_COMMAND, 0, stick_y);
    length = ftRemixAiDiPutStick(script, length, STICK_X_COMMAND, wait, stick_x);
    length = ftRemixAiDiPutStick(script, length, STICK_Y_COMMAND, 0, STICK_CENTER);
    length = ftRemixAiDiPutStick(script, length, STICK_X_COMMAND, wait, STICK_CENTER);

    return length;
}

/* Slide DI: the tap repeated with the stick swept across the other axis (di_input, slide branch). */
static int32_t ftRemixAiDiBuildSlide(int32_t wait, int32_t stick_x, int32_t stick_y, uint8_t *script)
{
    int32_t is_horizontal = (stick_y == STICK_CENTER) ? 1 : 0;
    int32_t x_second = is_horizontal ? stick_x : STICK_NEGATIVE;
    int32_t x_fourth = is_horizontal ? stick_x : STICK_POSITIVE;
    int32_t y_second = is_horizontal ? STICK_NEGATIVE : stick_y;
    int32_t y_fourth = is_horizontal ? STICK_POSITIVE : stick_y;
    int32_t length = 0;

    length = ftRemixAiDiPutStick(script, length, STICK_Y_COMMAND, 0, stick_y);
    length = ftRemixAiDiPutStick(script, length, STICK_X_COMMAND, wait, stick_x);
    length = ftRemixAiDiPutStick(script, length, STICK_Y_COMMAND, 0, y_second);
    length = ftRemixAiDiPutStick(script, length, STICK_X_COMMAND, wait, x_second);
    length = ftRemixAiDiPutStick(script, length, STICK_Y_COMMAND, 0, stick_y);
    length = ftRemixAiDiPutStick(script, length, STICK_X_COMMAND, wait, stick_x);
    length = ftRemixAiDiPutStick(script, length, STICK_Y_COMMAND, 0, y_fourth);
    length = ftRemixAiDiPutStick(script, length, STICK_X_COMMAND, wait, x_fourth);

    return length;
}

int32_t ftRemixAiDiBuildScript(int32_t is_slide, int32_t strength, int32_t direction, uint8_t *out_script)
{
    int32_t wait = ftRemixAiDiGetWait(strength);
    int32_t stick_x;
    int32_t stick_y;
    int32_t length;

    ftRemixAiDiGetStick(direction, &stick_x, &stick_y);
    length = (is_slide != 0) ? ftRemixAiDiBuildSlide(wait, stick_x, stick_y, out_script)
                             : ftRemixAiDiBuildSmash(wait, stick_x, stick_y, out_script);
    out_script[length++] = SCRIPT_END;

    return length;
}
