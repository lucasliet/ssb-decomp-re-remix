/*
 * mnremixvsstatspages.c - layout of the two VS match stats pages (VsStats.asm setup_ page 1 and page 2, draw_line_,
 * draw_underline_, draw_port_headers_). A row is the label (left aligned at X 24) and one right aligned cell per active
 * port at X 184 + 35 * column, every string at scale 0.8125 as in the asm.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>
#include <sc/scmanager.h>
#include "gm/gmremixtoggle.h"
#include "if/ifremixcombometer.h"
#include "mn/mnvsmode/mnremixvsstats.h"
#include "mn/mnvsmode/mnremixvsstatspages.h"
#include "remix/remix_hud_text.h"

#define MNVSSTATS_PAGES_TEXT_SLOT_FIRST 48
#define MNVSSTATS_PAGES_RECT_SLOT_BACKGROUND 0
#define MNVSSTATS_PAGES_RECT_SLOT_FIRST 1
#define MNVSSTATS_PAGES_FIRST_Y 30
#define MNVSSTATS_PAGES_LINE_STEP 11
#define MNVSSTATS_PAGES_GROUP_GAP 5
#define MNVSSTATS_PAGES_UNDERLINE_ADJUST 1
#define MNVSSTATS_PAGES_UNDERLINE_AFTER 3
#define MNVSSTATS_PAGES_UNDERLINE_X 24.0F
#define MNVSSTATS_PAGES_UNDERLINE_HEIGHT 1.0F
#define MNVSSTATS_PAGES_UNDERLINE_RGBA 0xFFFFFFFF
#define MNVSSTATS_PAGES_UNDERLINE_DAMAGE 75
#define MNVSSTATS_PAGES_UNDERLINE_COMBO 68
#define MNVSSTATS_PAGES_UNDERLINE_ZCANCEL 80
#define MNVSSTATS_PAGES_UNDERLINE_TECH 58
#define MNVSSTATS_PAGES_UNDERLINE_LEDGE 65
#define MNVSSTATS_PAGES_UNDERLINE_SPECIAL 105
#define MNVSSTATS_PAGES_STRIPE_X 23.0F
#define MNVSSTATS_PAGES_STRIPE_WIDTH 266.0F
#define MNVSSTATS_PAGES_STRIPE_HEIGHT 10.0F
#define MNVSSTATS_PAGES_STRIPE_RGBA 0x40407040
#define MNVSSTATS_PAGES_BACKGROUND_X 10.0F
#define MNVSSTATS_PAGES_BACKGROUND_Y 10.0F
#define MNVSSTATS_PAGES_BACKGROUND_WIDTH 300.0F
#define MNVSSTATS_PAGES_BACKGROUND_HEIGHT 220.0F
#define MNVSSTATS_PAGES_BACKGROUND_RGBA 0x000000B0
#define MNVSSTATS_PAGES_LABEL_X 24.0F
#define MNVSSTATS_PAGES_CELL_X 184.0F
#define MNVSSTATS_PAGES_CELL_STEP 35.0F
#define MNVSSTATS_PAGES_PORT_HEADER_Y 16.0F
#define MNVSSTATS_PAGES_TEXT_SCALE 0.8125F
#define MNVSSTATS_PAGES_WHITE 0xFFFFFFFF
#define MNVSSTATS_PAGES_INDENT 8
#define MNVSSTATS_PAGES_NO_PORT (-1)
#define MNVSSTATS_PAGES_DASH "-"
#define MNVSSTATS_PAGES_NUMBER_LENGTH 12

/* VsStats.asm p1..p4 row labels */
static const char *const dMNVSStatsPagesPortNames[MN_REMIX_VS_STATS_PORT_COUNT] =
{
    "1P", "2P", "3P", "4P"
};

typedef struct MNRemixVSStatsCursor
{
    s32 y;
    s32 text_slot;
    s32 rect_slot;
    sb32 stripe_on;
} MNRemixVSStatsCursor;

typedef enum MNRemixVSStatsValue
{
    nMNRemixVSStatsValueNone,
    nMNRemixVSStatsValueDealtTo,
    nMNRemixVSStatsValueTotalDealt,
    nMNRemixVSStatsValueTotalTaken,
    nMNRemixVSStatsValueHighestTaken,
    nMNRemixVSStatsValueComboVS,
    nMNRemixVSStatsValueMaxHitsTaken,
    nMNRemixVSStatsValueMaxDamageTaken,
    nMNRemixVSStatsValueZCancelPercent,
    nMNRemixVSStatsValueZCancelSuccess,
    nMNRemixVSStatsValueZCancelMiss,
    nMNRemixVSStatsValueTechPercent,
    nMNRemixVSStatsValueTechSuccess,
    nMNRemixVSStatsValueTechMiss,
    nMNRemixVSStatsValueLedgeGrabs,
    nMNRemixVSStatsValueUpSpecial,
    nMNRemixVSStatsValueNeutralSpecial,
    nMNRemixVSStatsValueDownSpecial
} MNRemixVSStatsValue;

/* row: the port of a per-port row, or -1. dash: the column drawn as "-", or -1. ignore_port: the row is skipped unless this
 * port is active, or -1. */
typedef struct MNRemixVSStatsRow
{
    const char *label;
    s32 indent;
    MNRemixVSStatsValue value;
    s32 row;
    s32 dash;
    s32 ignore_port;
} MNRemixVSStatsRow;

/** TRUE when the port has a fighter in the match (the asm port_check: a man or a CPU). */
static sb32 mnRemixVSStatsPagesIsPortActive(s32 port)
{
    return gSCManagerTransferBattleState.players[port].pkind != nFTPlayerKindNot;
}

/**
 * Value of a row cell. For the damage table row is the victim port and column the attacker port, for the combo table row
 * is the victim as well; the other rows ignore row.
 */
static s32 mnRemixVSStatsPagesGetValue(MNRemixVSStatsValue value, s32 row, s32 column)
{
    const IFRemixComboMeter *meter = ifRemixComboMeterGetPort(column);

    switch (value)
    {
    case nMNRemixVSStatsValueDealtTo:
        return gSCManagerTransferBattleState.players[row].total_damage_players[column];
    case nMNRemixVSStatsValueTotalDealt:
        return gSCManagerTransferBattleState.players[column].total_damage_given;
    case nMNRemixVSStatsValueTotalTaken:
        return gSCManagerTransferBattleState.players[column].total_damage_all;
    case nMNRemixVSStatsValueHighestTaken:
        return mnRemixVSStatsGetHighestPercent(column);
    case nMNRemixVSStatsValueComboVS:
        return meter->highest_combo_vs[row];
    case nMNRemixVSStatsValueMaxHitsTaken:
        return meter->max_combo_hits;
    case nMNRemixVSStatsValueMaxDamageTaken:
        return meter->max_combo_damage;
    case nMNRemixVSStatsValueZCancelPercent:
        return mnRemixVSStatsGetTrackerPercent(column, nMNRemixVSStatsTrackerZCancelSuccess, nMNRemixVSStatsTrackerZCancelMiss);
    case nMNRemixVSStatsValueZCancelSuccess:
        return mnRemixVSStatsGetTracker(column, nMNRemixVSStatsTrackerZCancelSuccess);
    case nMNRemixVSStatsValueZCancelMiss:
        return mnRemixVSStatsGetTracker(column, nMNRemixVSStatsTrackerZCancelMiss);
    case nMNRemixVSStatsValueTechPercent:
        return mnRemixVSStatsGetTrackerPercent(column, nMNRemixVSStatsTrackerTechSuccess, nMNRemixVSStatsTrackerTechMiss);
    case nMNRemixVSStatsValueTechSuccess:
        return mnRemixVSStatsGetTracker(column, nMNRemixVSStatsTrackerTechSuccess);
    case nMNRemixVSStatsValueTechMiss:
        return mnRemixVSStatsGetTracker(column, nMNRemixVSStatsTrackerTechMiss);
    case nMNRemixVSStatsValueLedgeGrabs:
        return mnRemixVSStatsGetTracker(column, nMNRemixVSStatsTrackerLedgeGrab);
    case nMNRemixVSStatsValueUpSpecial:
        return mnRemixVSStatsGetSpecialCount(column, nMNRemixVSStatsSpecialUp);
    case nMNRemixVSStatsValueNeutralSpecial:
        return mnRemixVSStatsGetSpecialCount(column, nMNRemixVSStatsSpecialNeutral);
    case nMNRemixVSStatsValueDownSpecial:
        return mnRemixVSStatsGetSpecialCount(column, nMNRemixVSStatsSpecialDown);
    default:
        return 0;
    }
}

/** Row of a per-port table: the label is the port name, the victim row is the port itself (the asm p1..p4 rows). */
static MNRemixVSStatsRow mnRemixVSStatsPagesPortRow(s32 port, MNRemixVSStatsValue value)
{
    MNRemixVSStatsRow row;

    row.label = dMNVSStatsPagesPortNames[port];
    row.indent = MNVSSTATS_PAGES_INDENT;
    row.value = value;
    row.row = port;
    row.dash = port;
    row.ignore_port = port;
    return row;
}

/** Row with one value per active port and no per-port meaning. */
static MNRemixVSStatsRow mnRemixVSStatsPagesStatRow(const char *label, s32 indent, MNRemixVSStatsValue value)
{
    MNRemixVSStatsRow row;

    row.label = label;
    row.indent = indent;
    row.value = value;
    row.row = MNVSSTATS_PAGES_NO_PORT;
    row.dash = MNVSSTATS_PAGES_NO_PORT;
    row.ignore_port = MNVSSTATS_PAGES_NO_PORT;
    return row;
}

/** Writes a decimal number, with a leading minus for a negative value, into out. */
static void mnRemixVSStatsPagesFormatNumber(char *out, s32 value)
{
    char digits[MNVSSTATS_PAGES_NUMBER_LENGTH];
    s32 count = 0;
    s32 length = 0;
    u32 magnitude = (value < 0) ? (u32)(-value) : (u32)value;

    do
    {
        digits[count++] = (char)('0' + (magnitude % 10));
        magnitude /= 10;
    } while (magnitude != 0);

    if (value < 0)
    {
        out[length++] = '-';
    }
    while (count > 0)
    {
        out[length++] = digits[--count];
    }
    out[length] = '\0';
}

static void mnRemixVSStatsPagesFormatDash(char *out);

/** Draws one string at the cursor text slot. The overlay keeps it for the frame it was put in and the next one. */
static void mnRemixVSStatsPagesPutText(MNRemixVSStatsCursor *cursor, const char *text, f32 x, f32 y, s32 align)
{
    remix_hud_text_put(cursor->text_slot, text, x, y, MNVSSTATS_PAGES_WHITE, MNVSSTATS_PAGES_TEXT_SCALE, align);
    cursor->text_slot++;
}

/** X of the right edge of the cell of the nth active port (the asm packs the columns of active ports only). */
static f32 mnRemixVSStatsPagesCellX(s32 column_index)
{
    return MNVSSTATS_PAGES_CELL_X + MNVSSTATS_PAGES_CELL_STEP * (f32)column_index;
}

/** Draws the cells of a row, one right aligned string per active port. */
static void mnRemixVSStatsPagesDrawCells(MNRemixVSStatsCursor *cursor, const MNRemixVSStatsRow *row)
{
    char number[MNVSSTATS_PAGES_NUMBER_LENGTH];
    s32 column;
    s32 column_index = 0;

    for (column = 0; column < MN_REMIX_VS_STATS_PORT_COUNT; column++)
    {
        if (!mnRemixVSStatsPagesIsPortActive(column))
        {
            continue;
        }
        if (column == row->dash)
        {
            mnRemixVSStatsPagesFormatDash(number);
        }
        else
        {
            mnRemixVSStatsPagesFormatNumber(number, mnRemixVSStatsPagesGetValue(row->value, row->row, column));
        }
        mnRemixVSStatsPagesPutText(cursor, number, mnRemixVSStatsPagesCellX(column_index), (f32)cursor->y,
                                   REMIX_HUD_ALIGN_RIGHT);
        column_index++;
    }
}

/** Writes the "-" of a cell that has no value (the row of a port against itself). */
static void mnRemixVSStatsPagesFormatDash(char *out)
{
    out[0] = MNVSSTATS_PAGES_DASH[0];
    out[1] = '\0';
}

/** Striped background of every second row that has values (asm draw_line_ stripe_on flip). */
static void mnRemixVSStatsPagesStripeRow(MNRemixVSStatsCursor *cursor)
{
    if (cursor->stripe_on)
    {
        remix_hud_rect_put(cursor->rect_slot, MNVSSTATS_PAGES_STRIPE_X, (f32)cursor->y, MNVSSTATS_PAGES_STRIPE_WIDTH,
                           MNVSSTATS_PAGES_STRIPE_HEIGHT, MNVSSTATS_PAGES_STRIPE_RGBA);
        cursor->rect_slot++;
    }
    cursor->stripe_on = !cursor->stripe_on;
}

/** Draws a row. A row whose ignore port is inactive is skipped without moving the cursor, as in the asm. */
static void mnRemixVSStatsPagesDrawRow(MNRemixVSStatsCursor *cursor, const MNRemixVSStatsRow *row)
{
    if ((row->ignore_port != MNVSSTATS_PAGES_NO_PORT) && !mnRemixVSStatsPagesIsPortActive(row->ignore_port))
    {
        return;
    }
    if (row->value != nMNRemixVSStatsValueNone)
    {
        mnRemixVSStatsPagesStripeRow(cursor);
    }
    mnRemixVSStatsPagesPutText(cursor, row->label, MNVSSTATS_PAGES_LABEL_X + (f32)row->indent, (f32)cursor->y,
                               REMIX_HUD_ALIGN_LEFT);
    mnRemixVSStatsPagesDrawCells(cursor, row);
    cursor->y += MNVSSTATS_PAGES_LINE_STEP;
}

/** Draws a label-only header line (asm draw_header). */
static void mnRemixVSStatsPagesDrawHeader(MNRemixVSStatsCursor *cursor, const char *label)
{
    mnRemixVSStatsPagesPutText(cursor, label, MNVSSTATS_PAGES_LABEL_X, (f32)cursor->y, REMIX_HUD_ALIGN_LEFT);
    cursor->y += MNVSSTATS_PAGES_LINE_STEP;
}

/** Draws the white underline of a header (asm draw_underline_ with the y adjustment of the page setup). */
static void mnRemixVSStatsPagesDrawUnderline(MNRemixVSStatsCursor *cursor, s32 width)
{
    cursor->y -= MNVSSTATS_PAGES_UNDERLINE_ADJUST;
    remix_hud_rect_put(cursor->rect_slot, MNVSSTATS_PAGES_UNDERLINE_X, (f32)cursor->y, (f32)width,
                       MNVSSTATS_PAGES_UNDERLINE_HEIGHT, MNVSSTATS_PAGES_UNDERLINE_RGBA);
    cursor->rect_slot++;
    cursor->y += MNVSSTATS_PAGES_UNDERLINE_AFTER;
}

/** Space between two sections (the asm "adjust y for cleaner spacing"). */
static void mnRemixVSStatsPagesDrawGap(MNRemixVSStatsCursor *cursor)
{
    cursor->y += MNVSSTATS_PAGES_GROUP_GAP;
}

/** Draws the 1P to 4P headers of the active ports above the page (asm draw_port_headers_), right aligned over the cells. */
static void mnRemixVSStatsPagesDrawPortHeader(MNRemixVSStatsCursor *cursor)
{
    s32 column;
    s32 column_index = 0;

    for (column = 0; column < MN_REMIX_VS_STATS_PORT_COUNT; column++)
    {
        if (!mnRemixVSStatsPagesIsPortActive(column))
        {
            continue;
        }
        mnRemixVSStatsPagesPutText(cursor, dMNVSStatsPagesPortNames[column], mnRemixVSStatsPagesCellX(column_index),
                                   MNVSSTATS_PAGES_PORT_HEADER_Y, REMIX_HUD_ALIGN_RIGHT);
        column_index++;
    }
}

/** Draws the victim rows of the damage table, one per port (asm draw_row p1..p4). */
static void mnRemixVSStatsPagesDrawDealtRows(MNRemixVSStatsCursor *cursor)
{
    s32 port;

    for (port = 0; port < MN_REMIX_VS_STATS_PORT_COUNT; port++)
    {
        MNRemixVSStatsRow row = mnRemixVSStatsPagesPortRow(port, nMNRemixVSStatsValueDealtTo);

        mnRemixVSStatsPagesDrawRow(cursor, &row);
    }
}

/** Draws the per-port rows of a combo table (asm draw_row with ComboMeter.combo_struct_p1). */
static void mnRemixVSStatsPagesDrawPortRows(MNRemixVSStatsCursor *cursor, MNRemixVSStatsValue value)
{
    s32 port;

    for (port = 0; port < MN_REMIX_VS_STATS_PORT_COUNT; port++)
    {
        MNRemixVSStatsRow row = mnRemixVSStatsPagesPortRow(port, value);

        mnRemixVSStatsPagesDrawRow(cursor, &row);
    }
}

/** Page 1 damage section: dealt to each port, total given, total taken and highest taken. */
static void mnRemixVSStatsPagesDrawDamage(MNRemixVSStatsCursor *cursor)
{
    MNRemixVSStatsRow total_dealt = mnRemixVSStatsPagesStatRow("Total Dealt", 0, nMNRemixVSStatsValueTotalDealt);
    MNRemixVSStatsRow total_taken = mnRemixVSStatsPagesStatRow("Total Taken", 0, nMNRemixVSStatsValueTotalTaken);
    MNRemixVSStatsRow highest = mnRemixVSStatsPagesStatRow("Highest Taken", 0, nMNRemixVSStatsValueHighestTaken);

    mnRemixVSStatsPagesDrawHeader(cursor, "Damage Stats");
    mnRemixVSStatsPagesDrawUnderline(cursor, MNVSSTATS_PAGES_UNDERLINE_DAMAGE);
    mnRemixVSStatsPagesDrawHeader(cursor, "Dealt to");
    mnRemixVSStatsPagesDrawDealtRows(cursor);
    mnRemixVSStatsPagesDrawRow(cursor, &total_dealt);
    mnRemixVSStatsPagesDrawRow(cursor, &total_taken);
    mnRemixVSStatsPagesDrawRow(cursor, &highest);
}

/** Page 1 combo section, only when the combo meter setting is on (asm _combo_stats_on_check). */
static void mnRemixVSStatsPagesDrawCombo(MNRemixVSStatsCursor *cursor)
{
    MNRemixVSStatsRow max_hits = mnRemixVSStatsPagesStatRow("Max Hits Taken", 0, nMNRemixVSStatsValueMaxHitsTaken);
    MNRemixVSStatsRow max_damage = mnRemixVSStatsPagesStatRow("Max Damage Taken", 0, nMNRemixVSStatsValueMaxDamageTaken);

    mnRemixVSStatsPagesDrawGap(cursor);
    mnRemixVSStatsPagesDrawHeader(cursor, "Combo Stats");
    mnRemixVSStatsPagesDrawUnderline(cursor, MNVSSTATS_PAGES_UNDERLINE_COMBO);
    mnRemixVSStatsPagesDrawHeader(cursor, "Longest Combo VS");
    mnRemixVSStatsPagesDrawPortRows(cursor, nMNRemixVSStatsValueComboVS);
    mnRemixVSStatsPagesDrawRow(cursor, &max_hits);
    mnRemixVSStatsPagesDrawRow(cursor, &max_damage);
}

/** Page 1: damage stats, then combo stats when the combo meter is on. */
static void mnRemixVSStatsPagesDrawPageOne(MNRemixVSStatsCursor *cursor)
{
    mnRemixVSStatsPagesDrawDamage(cursor);
    if (gmRemixToggle(combo_meter) != FALSE)
    {
        mnRemixVSStatsPagesDrawCombo(cursor);
    }
}

/** Page 2: Z-cancel, tech, ledge and special move stats (asm page 2 of setup_). */
static void mnRemixVSStatsPagesDrawPageTwo(MNRemixVSStatsCursor *cursor)
{
    MNRemixVSStatsRow zcancel_percent = mnRemixVSStatsPagesStatRow("Success %", 0, nMNRemixVSStatsValueZCancelPercent);
    MNRemixVSStatsRow zcancel_success = mnRemixVSStatsPagesStatRow("Successful", 8, nMNRemixVSStatsValueZCancelSuccess);
    MNRemixVSStatsRow zcancel_miss = mnRemixVSStatsPagesStatRow("Missed", 8, nMNRemixVSStatsValueZCancelMiss);
    MNRemixVSStatsRow tech_percent = mnRemixVSStatsPagesStatRow("Success %", 0, nMNRemixVSStatsValueTechPercent);
    MNRemixVSStatsRow tech_success = mnRemixVSStatsPagesStatRow("Successful", 8, nMNRemixVSStatsValueTechSuccess);
    MNRemixVSStatsRow tech_miss = mnRemixVSStatsPagesStatRow("Missed", 8, nMNRemixVSStatsValueTechMiss);
    MNRemixVSStatsRow grabbed = mnRemixVSStatsPagesStatRow("Times grabbed", 0, nMNRemixVSStatsValueLedgeGrabs);
    MNRemixVSStatsRow up = mnRemixVSStatsPagesStatRow("Up Specials used", 0, nMNRemixVSStatsValueUpSpecial);
    MNRemixVSStatsRow neutral = mnRemixVSStatsPagesStatRow("Neutral Specials used", 0, nMNRemixVSStatsValueNeutralSpecial);
    MNRemixVSStatsRow down = mnRemixVSStatsPagesStatRow("Down Specials used", 0, nMNRemixVSStatsValueDownSpecial);

    mnRemixVSStatsPagesDrawHeader(cursor, "Z-Cancel Stats");
    mnRemixVSStatsPagesDrawUnderline(cursor, MNVSSTATS_PAGES_UNDERLINE_ZCANCEL);
    mnRemixVSStatsPagesDrawRow(cursor, &zcancel_percent);
    mnRemixVSStatsPagesDrawRow(cursor, &zcancel_success);
    mnRemixVSStatsPagesDrawRow(cursor, &zcancel_miss);

    mnRemixVSStatsPagesDrawGap(cursor);
    mnRemixVSStatsPagesDrawHeader(cursor, "Tech Stats");
    mnRemixVSStatsPagesDrawUnderline(cursor, MNVSSTATS_PAGES_UNDERLINE_TECH);
    mnRemixVSStatsPagesDrawRow(cursor, &tech_percent);
    mnRemixVSStatsPagesDrawRow(cursor, &tech_success);
    mnRemixVSStatsPagesDrawRow(cursor, &tech_miss);

    mnRemixVSStatsPagesDrawGap(cursor);
    mnRemixVSStatsPagesDrawHeader(cursor, "Ledge Stats");
    mnRemixVSStatsPagesDrawUnderline(cursor, MNVSSTATS_PAGES_UNDERLINE_LEDGE);
    mnRemixVSStatsPagesDrawRow(cursor, &grabbed);

    mnRemixVSStatsPagesDrawGap(cursor);
    mnRemixVSStatsPagesDrawHeader(cursor, "Special Move Stats");
    mnRemixVSStatsPagesDrawUnderline(cursor, MNVSSTATS_PAGES_UNDERLINE_SPECIAL);
    mnRemixVSStatsPagesDrawRow(cursor, &up);
    mnRemixVSStatsPagesDrawRow(cursor, &neutral);
    mnRemixVSStatsPagesDrawRow(cursor, &down);
}

/** Number of ports with a fighter in the match. */
static s32 mnRemixVSStatsPagesCountActivePorts(void)
{
    s32 port;
    s32 count = 0;

    for (port = 0; port < MN_REMIX_VS_STATS_PORT_COUNT; port++)
    {
        if (mnRemixVSStatsPagesIsPortActive(port))
        {
            count++;
        }
    }
    return count;
}

/**
 * Stripe state at the start of page 2. VsStats.asm never resets stripe_on, so page 2 continues the alternation where the
 * rows with values of page 1 left it: the damage rows, the three totals and, with the combo meter on, the combo rows.
 */
static sb32 mnRemixVSStatsPagesPageTwoStartsStriped(void)
{
    s32 active = mnRemixVSStatsPagesCountActivePorts();
    s32 rows_with_values = active + 3;

    if (gmRemixToggle(combo_meter) != FALSE)
    {
        rows_with_values += active + 2;
    }
    return (rows_with_values & 1) != 0;
}

void mnRemixVSStatsPagesDraw(s32 page)
{
    MNRemixVSStatsCursor cursor;

    cursor.y = MNVSSTATS_PAGES_FIRST_Y;
    cursor.text_slot = MNVSSTATS_PAGES_TEXT_SLOT_FIRST;
    cursor.rect_slot = MNVSSTATS_PAGES_RECT_SLOT_FIRST;
    cursor.stripe_on = FALSE;

    remix_hud_rect_put(MNVSSTATS_PAGES_RECT_SLOT_BACKGROUND, MNVSSTATS_PAGES_BACKGROUND_X, MNVSSTATS_PAGES_BACKGROUND_Y,
                       MNVSSTATS_PAGES_BACKGROUND_WIDTH, MNVSSTATS_PAGES_BACKGROUND_HEIGHT, MNVSSTATS_PAGES_BACKGROUND_RGBA);
    mnRemixVSStatsPagesDrawPortHeader(&cursor);
    if (page == 0)
    {
        mnRemixVSStatsPagesDrawPageOne(&cursor);
        return;
    }
    cursor.stripe_on = mnRemixVSStatsPagesPageTwoStartsStriped();
    mnRemixVSStatsPagesDrawPageTwo(&cursor);
}

#endif
