/**
 * mnremixcsssliderules.c - pure rules of the slide-in of the Remix VS character select portraits, see mnremixcsssliderules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "remix/remix_css_data.h"
#include "mn/mnplayers/mnremixcsssliderules.h"

#define MNREMIX_CSS_SLIDE_LEFT_START_X (-20.0F)
#define MNREMIX_CSS_SLIDE_RIGHT_START_X 310.0F

int mnRemixCssSlideRulesColumn(int slot)
{
    if ((slot < 0) || (slot >= (REMIX_CSS_NUM_SLOTS + 2)))
    {
        return -1;
    }
    if (slot < REMIX_CSS_NUM_SLOTS)
    {
        return slot % REMIX_CSS_NUM_COLUMNS;
    }
    return (slot == REMIX_CSS_NUM_SLOTS) ? MNREMIX_CSS_SLIDE_BONUS_COLUMN : MNREMIX_CSS_SLIDE_RANDOM_COLUMN;
}

float mnRemixCssSlideRulesStartX(int column)
{
    if (column == MNREMIX_CSS_SLIDE_RANDOM_COLUMN)
    {
        return MNREMIX_CSS_SLIDE_RIGHT_START_X;
    }
    if (column == MNREMIX_CSS_SLIDE_BONUS_COLUMN)
    {
        return MNREMIX_CSS_SLIDE_LEFT_START_X;
    }
    return (column < (REMIX_CSS_NUM_COLUMNS / 2)) ? MNREMIX_CSS_SLIDE_LEFT_START_X : MNREMIX_CSS_SLIDE_RIGHT_START_X;
}

float mnRemixCssSlideRulesNextX(int column, float current_x)
{
    float target = kRemixCssPortraitX[column];
    float next_x = current_x + kRemixCssPortraitVelocity[column];

    if (current_x == target)
    {
        return target;
    }
    if (target < current_x)
    {
        return (next_x <= target) ? target : next_x;
    }
    return (next_x >= target) ? target : next_x;
}

#endif
