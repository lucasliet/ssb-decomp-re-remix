/**
 * sc1ptrainingmenurules.c - pure navigation rules of the Training Z menu, see sc1ptrainingmenurules.h.
 */

#include "sc/sc1pmode/sc1ptrainingmenurules.h"

int32_t sc1PTrainingMenuRulesIsStickStep(int32_t frames_held)
{
    if (frames_held <= 0)
    {
        return 0;
    }
    if (frames_held == 1)
    {
        return 1;
    }
    if (frames_held < SC1PTRAININGMENURULES_STICK_REPEAT_DELAY)
    {
        return 0;
    }
    return ((frames_held % SC1PTRAININGMENURULES_STICK_REPEAT_INTERVAL) == 0) ? 1 : 0;
}

int32_t sc1PTrainingMenuRulesGetPage(int32_t selection)
{
    return selection / SC1PTRAININGMENURULES_ROWS_PER_PAGE;
}

int32_t sc1PTrainingMenuRulesGetRow(int32_t selection)
{
    return selection % SC1PTRAININGMENURULES_ROWS_PER_PAGE;
}

/* The selection of the first entry of `page`. */
static int32_t sc1PTrainingMenuRulesGetPageStart(int32_t page)
{
    return page * SC1PTRAININGMENURULES_ROWS_PER_PAGE;
}

/* Keeps a selection inside the list ("sw v0, 0x000C(a0)" after the page change when the row has no entry). */
static int32_t sc1PTrainingMenuRulesClampSelection(int32_t selection, int32_t entry_count)
{
    return (selection > (entry_count - 1)) ? (entry_count - 1) : selection;
}

int32_t sc1PTrainingMenuRulesMoveDown(int32_t selection, int32_t entry_count)
{
    int32_t next;

    if (selection >= (entry_count - 1))
    {
        if (entry_count > SC1PTRAININGMENURULES_ROWS_PER_PAGE)
        {
            return sc1PTrainingMenuRulesGetPageStart(sc1PTrainingMenuRulesGetPage(selection));
        }
        return 0;
    }
    next = selection + 1;

    if (sc1PTrainingMenuRulesGetRow(next) == 0)
    {
        return next - SC1PTRAININGMENURULES_ROWS_PER_PAGE;
    }
    return next;
}

int32_t sc1PTrainingMenuRulesMoveUp(int32_t selection, int32_t entry_count)
{
    int32_t last_on_page;

    if (sc1PTrainingMenuRulesGetRow(selection) != 0)
    {
        return selection - 1;
    }
    last_on_page = selection + SC1PTRAININGMENURULES_ROWS_PER_PAGE - 1;

    return sc1PTrainingMenuRulesClampSelection(last_on_page, entry_count);
}

int32_t sc1PTrainingMenuRulesNextPage(int32_t selection, int32_t entry_count)
{
    int32_t next_page = sc1PTrainingMenuRulesGetPage(selection) + 1;

    if (sc1PTrainingMenuRulesGetPageStart(next_page) >= entry_count)
    {
        return sc1PTrainingMenuRulesGetRow(selection);
    }
    return sc1PTrainingMenuRulesClampSelection(selection + SC1PTRAININGMENURULES_ROWS_PER_PAGE, entry_count);
}

int32_t sc1PTrainingMenuRulesPrevPage(int32_t selection, int32_t entry_count)
{
    int32_t last_page = sc1PTrainingMenuRulesGetPage(entry_count - 1);
    int32_t previous_page;

    if (last_page == 0)
    {
        return selection;
    }
    previous_page = (selection < SC1PTRAININGMENURULES_ROWS_PER_PAGE) ? last_page : (sc1PTrainingMenuRulesGetPage(selection) - 1);

    return sc1PTrainingMenuRulesClampSelection(sc1PTrainingMenuRulesGetPageStart(previous_page) + sc1PTrainingMenuRulesGetRow(selection),
                                               entry_count);
}

int32_t sc1PTrainingMenuRulesStepValue(int32_t value, int32_t min, int32_t max, int32_t direction)
{
    if (direction == SC1PTRAININGMENURULES_STEP_RIGHT)
    {
        return (value < max) ? (value + 1) : min;
    }
    return (min < value) ? (value - 1) : max;
}

int32_t sc1PTrainingMenuRulesClampValue(int32_t value, int32_t max)
{
    return (value > max) ? max : value;
}
