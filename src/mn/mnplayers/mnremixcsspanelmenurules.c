/**
 * mnremixcsspanelmenurules.c - pure rules of the per-port panel menu of the Remix character select, see mnremixcsspanelmenurules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcsspanelmenurules.h"

#define APPLIES_VS 0x20
#define APPLIES_1P 0x10
#define APPLIES_TRAINING 0x08
#define APPLIES_BONUS 0x06
#define APPLIES_ALLSTAR 0x01
#define APPLIES_ALL 0x3F
#define WHO_CPU 0x01
#define WHO_HUMAN 0x02
#define WHO_BOTH 0x03

typedef struct MNRemixPanelMenuItemRule
{
    const char *label;
    int is_numeric;
    int min_value;
    int max_value;
    int default_value;
    int applies_to;
    int applies_to_who;
    int disables_high_scores;
} MNRemixPanelMenuItemRule;

/* The css/X.asm constants LABEL, VALUE_TYPE, MIN_VALUE, MAX_VALUE, DEFAULT_VALUE, APPLIES_TO, APPLIES_TO_HUMAN_CPU and
 * DISABLES_HIGH_SCORES of each item, in the order of add_menu_item (CharacterSelectDebugMenu.asm 2720-2739). */
static const MNRemixPanelMenuItemRule sItemRules[MNREMIX_PANELMENU_ITEM_COUNT] =
{
    { "Shield", 0, 0, 17, 0, APPLIES_ALL, WHO_BOTH, 0 },
    { "Player Tag", 0, 0, 20, 0, APPLIES_ALL, WHO_BOTH, 0 },
    { "Visibility", 0, 0, 3, 0, APPLIES_ALL, WHO_BOTH, 0 },
    { "Skeleton", 0, 0, 1, 0, APPLIES_ALL, WHO_BOTH, 0 },
    { "Model", 0, 0, 2, 0, APPLIES_ALL, WHO_BOTH, 0 },
    { "Input HUD", 0, 0, 2, 0, APPLIES_ALL, WHO_BOTH, 0 },
    { "Size", 0, 0, 2, 0, APPLIES_ALL, WHO_BOTH, 1 },
    { "Stock", 0, 0, 2, 0, APPLIES_VS, WHO_BOTH, 0 },
    { "Knockback", 0, 0, 1, 0, APPLIES_ALL, WHO_BOTH, 1 },
    { "Delay", 0, 0, 12, 0, APPLIES_ALL, WHO_HUMAN, 0 },
    { "Handicap", 0, 0, 0x28, 0, APPLIES_VS | APPLIES_TRAINING, WHO_CPU, 1 },
    { "Start With", 0, 0, 19, 0, APPLIES_ALL, WHO_BOTH, 1 },
    { "Taunt Itm.", 0, 0, 19, 0, APPLIES_ALL, WHO_BOTH, 1 },
    { "Taunt Btn.", 0, 0, 8, 0, APPLIES_ALL, WHO_HUMAN, 0 },
    { "Kirby Hat", 0, 0, 0x2A, 0, APPLIES_ALL, WHO_BOTH, 1 },
    { "Dpad map", 0, 0, 3, 0, APPLIES_ALL, WHO_HUMAN, 1 },
    { "Dpad ctrl", 0, 0, 4, 0, APPLIES_ALL, WHO_HUMAN, 0 },
    { "Initial Damage", 1, 0, 999, 0, APPLIES_VS | APPLIES_1P | APPLIES_BONUS, WHO_BOTH, 1 },
    { "Poison Dmg", 0, 0, 4, 0, APPLIES_VS | APPLIES_1P | APPLIES_BONUS | APPLIES_ALLSTAR, WHO_BOTH, 1 },
    { "Practice", 0, 0, 11, 0, APPLIES_1P, WHO_HUMAN, 1 },
};

static int isItemValid(int item)
{
    return ((item >= 0) && (item < MNREMIX_PANELMENU_ITEM_COUNT)) ? 1 : 0;
}

static int getModeMask(int mode)
{
    switch (mode)
    {
    case MNREMIX_PANELMENU_MODE_VS:
        return APPLIES_VS;

    case MNREMIX_PANELMENU_MODE_1P:
        return APPLIES_1P;

    case MNREMIX_PANELMENU_MODE_TRAINING:
        return APPLIES_TRAINING;

    case MNREMIX_PANELMENU_MODE_BONUS:
        return APPLIES_BONUS;

    case MNREMIX_PANELMENU_MODE_ALLSTAR:
        return APPLIES_ALLSTAR;

    default:
        return 0;
    }
}

/** The human and CPU bitmasks only split the lists of the VS and Training CSS (add_menu_item checks them for those two only). */
static int isListSplitByKind(int mode)
{
    return ((mode == MNREMIX_PANELMENU_MODE_VS) || (mode == MNREMIX_PANELMENU_MODE_TRAINING)) ? 1 : 0;
}

static int isItemListed(int item, int mode, int who_mask)
{
    const MNRemixPanelMenuItemRule *rule = &sItemRules[item];

    if ((rule->applies_to & getModeMask(mode)) == 0)
    {
        return 0;
    }
    if (isListSplitByKind(mode) == 0)
    {
        return 1;
    }
    return ((rule->applies_to_who & who_mask) != 0) ? 1 : 0;
}

static int listItemsOfKinds(int mode, int who_mask, int *items, int capacity)
{
    int count = 0;
    int item;

    for (item = 0; (item < MNREMIX_PANELMENU_ITEM_COUNT) && (count < capacity); item++)
    {
        if (isItemListed(item, mode, who_mask) != 0)
        {
            items[count++] = item;
        }
    }
    return count;
}

int mnRemixPanelMenuRulesListItems(int mode, int is_cpu, int *items, int capacity)
{
    int who_mask = ((is_cpu != 0) && (isListSplitByKind(mode) != 0)) ? WHO_CPU : WHO_HUMAN;

    return listItemsOfKinds(mode, who_mask, items, capacity);
}

int mnRemixPanelMenuRulesListResetItems(int mode, int *items, int capacity)
{
    return listItemsOfKinds(mode, WHO_BOTH, items, capacity);
}

int mnRemixPanelMenuRulesItemMin(int item)
{
    return (isItemValid(item) != 0) ? sItemRules[item].min_value : 0;
}

int mnRemixPanelMenuRulesItemMax(int item)
{
    return (isItemValid(item) != 0) ? sItemRules[item].max_value : 0;
}

int mnRemixPanelMenuRulesItemDefault(int item)
{
    return (isItemValid(item) != 0) ? sItemRules[item].default_value : 0;
}

const char *mnRemixPanelMenuRulesItemLabel(int item)
{
    return (isItemValid(item) != 0) ? sItemRules[item].label : "";
}

int mnRemixPanelMenuRulesItemIsNumeric(int item)
{
    return (isItemValid(item) != 0) ? sItemRules[item].is_numeric : 0;
}

int mnRemixPanelMenuRulesItemDisablesHighScores(int item)
{
    return (isItemValid(item) != 0) ? sItemRules[item].disables_high_scores : 0;
}

int mnRemixPanelMenuRulesMaxPage(int item_count)
{
    if (item_count <= 0)
    {
        return 0;
    }
    return (item_count - 1) / MNREMIX_PANELMENU_ROWS;
}

int mnRemixPanelMenuRulesClampPage(int page, int max_page)
{
    return (page > max_page) ? max_page : page;
}

int mnRemixPanelMenuRulesStepPage(int page, int direction, int max_page)
{
    int next = page + direction;

    if (next < 0)
    {
        return max_page;
    }
    return (next > max_page) ? 0 : next;
}

int mnRemixPanelMenuRulesRowsOnPage(int page, int item_count)
{
    int remaining = item_count - (page * MNREMIX_PANELMENU_ROWS);

    if (remaining <= 0)
    {
        return 0;
    }
    return (remaining < MNREMIX_PANELMENU_ROWS) ? remaining : MNREMIX_PANELMENU_ROWS;
}

int mnRemixPanelMenuRulesStepValue(int value, int direction, int min, int max)
{
    int next = value + direction;

    if (next < min)
    {
        return max;
    }
    return (next > max) ? min : next;
}

int mnRemixPanelMenuRulesBlinkNext(int timer)
{
    int next = timer + 1;

    return (next >= MNREMIX_PANELMENU_BLINK_PERIOD) ? 0 : next;
}

int mnRemixPanelMenuRulesBlinkIsVisible(int timer)
{
    return (timer < MNREMIX_PANELMENU_BLINK_ON_FRAMES) ? 1 : 0;
}

int mnRemixPanelMenuRulesHoldAdvance(MNRemixPanelMenuHold *hold, int is_held)
{
    unsigned int held_frames;

    if (is_held == 0)
    {
        hold->held_frames = 0;
        hold->repeat_timer = 0;
        return 0;
    }
    held_frames = hold->held_frames + 1;

    if (held_frames < MNREMIX_PANELMENU_HOLD_FAST_BUFFER)
    {
        hold->held_frames = held_frames;
    }
    if (held_frames < MNREMIX_PANELMENU_HOLD_BUFFER)
    {
        return 0;
    }
    hold->repeat_timer = (hold->repeat_timer + 1) & 0xFF;

    if ((held_frames < MNREMIX_PANELMENU_HOLD_FAST_BUFFER) && (hold->repeat_timer < MNREMIX_PANELMENU_HOLD_TIMER))
    {
        return 0;
    }
    hold->repeat_timer = 0;
    return 1;
}

void mnRemixPanelMenuRulesHoldRestart(MNRemixPanelMenuHold *hold)
{
    hold->held_frames = 0;
}

float mnRemixPanelMenuRulesRowTextY(int row, int is_value)
{
    float base = (is_value != 0) ? MNREMIX_PANELMENU_VALUE_Y : MNREMIX_PANELMENU_LABEL_Y;

    return base + (float)(row * MNREMIX_PANELMENU_ROW_HEIGHT);
}

float mnRemixPanelMenuRulesRowArrowY(int row)
{
    return MNREMIX_PANELMENU_ITEM_ARROW_Y + (float)(row * MNREMIX_PANELMENU_ROW_HEIGHT);
}

float mnRemixPanelMenuRulesItemArrowX(float panel_x, int panel_width, int is_right)
{
    if (is_right == 0)
    {
        return panel_x + MNREMIX_PANELMENU_ITEM_ARROW_LEFT_X;
    }
    return panel_x + (float)panel_width - MNREMIX_PANELMENU_ITEM_ARROW_RIGHT_INSET;
}

float mnRemixPanelMenuRulesPageArrowX(float panel_x, int panel_width, int is_right)
{
    float inset = (is_right != 0) ? MNREMIX_PANELMENU_PAGE_ARROW_RIGHT_INSET : MNREMIX_PANELMENU_PAGE_ARROW_LEFT_INSET;

    return panel_x + (float)panel_width - inset;
}

int mnRemixPanelMenuRulesStaminaDamage(int value, int direction, int total_hp)
{
    if (value <= total_hp)
    {
        return value;
    }
    return (direction > 0) ? 0 : total_hp;
}

int mnRemixPanelMenuRulesStockValue(int keeps_value, int value)
{
    return (keeps_value != 0) ? value : 0;
}

int mnRemixPanelMenuRulesHighScoresEnabled(int mode, const int *values)
{
    int items[MNREMIX_PANELMENU_ITEM_COUNT];
    int count = mnRemixPanelMenuRulesListItems(mode, 0, items, MNREMIX_PANELMENU_ITEM_COUNT);
    int index;

    for (index = 0; index < count; index++)
    {
        int item = items[index];

        if ((mnRemixPanelMenuRulesItemDisablesHighScores(item) != 0) && (values[item] != mnRemixPanelMenuRulesItemDefault(item)))
        {
            return 0;
        }
    }
    return 1;
}

#endif
