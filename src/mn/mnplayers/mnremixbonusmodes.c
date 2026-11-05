/**
 * mnremixbonusmodes.c - Remix 1P modes on the Bonus 2 character select, see mnremixbonusmodes.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <reloc_data.h>
#include <string.h>
#include <sys/controller.h>
#include <sys/taskman.h>
#include <sys/utils.h>
#include <sys/objman.h>
#include "if/ifremixsprite.h"
#include "mn/mnplayers/mnplayers1pbonus.h"
#include "mn/mnplayers/mnremixbonusmodes.h"
#include "mn/mnplayers/mnremixbonusmodesrules.h"
#include "mn/mnplayers/mnremixcss.h"
#include "mn/mnplayers/mnremixcssbonus.h"
#include "mn/mnplayers/mnremixcostumes.h"
#include "mn/mnplayers/mnremixcssrandom.h"
#include "mn/mnplayers/mnremixcssrules.h"
#include "mn/mnplayers/mnremixvariants.h"
#include "remix/remix_css_data.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1pbonusremix.h"
#include "sc/sc1pmode/sc1pbonusremixrules.h"
#include "sc/sc1pmode/sc1premixbonus3.h"
#include "sc/sc1pmode/sc1premixbonus3rules.h"
#include "sc/sc1pmode/sc1premixhrcrecord.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixmultiman.h"

/* HUD text slots of the Bonus 2 character select (remix_hud_text.h slot map): 96 is "High Scores Disabled". */
#define MNREMIX_BONUSMODES_TEXT_SLOT_MODE_LABEL 97
#define MNREMIX_BONUSMODES_TEXT_SLOT_MODE_NAME 98
#define MNREMIX_BONUSMODES_TEXT_SLOT_STAGE_LABEL 99
#define MNREMIX_BONUSMODES_TEXT_SLOT_SET_STAGE 100

/* GObj link and display list link of the labels camera of the screen (mnPlayers1PBonusMakeLabels). */
#define MNREMIX_BONUSMODES_GOBJ_LINK 23
#define MNREMIX_BONUSMODES_DL_LINK 34

/* Record number and label, the standard best task count layout of the US build with the colours of the digits (mnPlayers1PBonusMakeBestTaskCount). */
#define MNREMIX_BONUSMODES_RECORD_LABEL_X 235.0F
#define MNREMIX_BONUSMODES_RECORD_LABEL_Y 195.0F
#define MNREMIX_BONUSMODES_RECORD_NUMBER_X 225.0F
#define MNREMIX_BONUSMODES_RECORD_NUMBER_Y 194.0F

/* Eight roll attempts is enough in practice, the guard only keeps a pool of locked fighters from looping forever. */
#define MNREMIX_BONUSMODES_ROLL_ATTEMPTS 256

static u32 sMNRemixBonusModesDigitColors[6] = { 0x00, 0x00, 0x00, 0x7E, 0x7C, 0x77 };

static GObj *sMNRemixBonusModesArrows[2];
static s32 sMNRemixBonusModesBlinkTimer;

intptr_t mnRemixBonusModesGetHeaderOffset(void)
{
    return mnRemixBonusModesHeaderOffset(sc1PRemixModeGet());
}

static u32 mnRemixBonusModesGetRecord(s32 mode, s32 fkind)
{
    if (mode == SC1PREMIX_MODE_HRC)
    {
        return (u32)sc1PRemixHrcRecordGet(fkind);
    }
    return sc1PRemixMultimanRecordGet(fkind);
}

static void mnRemixBonusModesStyleSprite(SObj *sobj, u32 red, u32 green, u32 blue)
{
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->sprite.red = red;
    sobj->sprite.green = green;
    sobj->sprite.blue = blue;
}

/**
 * Puts the decimal point of the Home-Run Contest feet: the tenths digit moves right and a period, the colon image cut to five rows,
 * goes between it and the units (draw_hrc_high_score_decimal_).
 *
 * @param gobj Record GObj; its first sprite is the label and the digits follow, the tenths first.
 * @param digits_file Loaded digits file.
 */
static void mnRemixBonusModesMakeDecimal(GObj *gobj, void *digits_file)
{
    SObj *label = SObjGetStruct(gobj);
    SObj *tenths = label->next;
    SObj *last_digit = label;
    SObj *point;

    if (tenths == NULL)
    {
        return;
    }
    tenths->pos.x += MNREMIX_BONUSMODES_DECIMAL_SHIFT;

    while (last_digit->next != NULL)
    {
        last_digit = last_digit->next;
    }
    point = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, digits_file, llIFCommonDigitsColonSprite));
    mnPlayers1PBonusSetDigitColors(point, sMNRemixBonusModesDigitColors);
    point->pos.x = MNREMIX_BONUSMODES_DECIMAL_X;
    point->pos.y = last_digit->pos.y + MNREMIX_BONUSMODES_DECIMAL_Y_OFFSET;
    point->sprite.bmheight = MNREMIX_BONUSMODES_DECIMAL_ROWS;
}

sb32 mnRemixBonusModesMakeRecord(GObj *gobj, void **files, s32 fkind)
{
    s32 mode = sc1PRemixModeGet();
    s32 counter_offset = mnRemixBonusModesCounterOffset(mode);
    SObj *label;

    if (counter_offset == 0)
    {
        return FALSE;
    }
    label = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, files[MNREMIX_BONUSMODES_FILE_1P_MODE], counter_offset));
    label->pos.x = MNREMIX_BONUSMODES_RECORD_LABEL_X;
    label->pos.y = MNREMIX_BONUSMODES_RECORD_LABEL_Y;
    mnRemixBonusModesStyleSprite(label, 0x7E, 0x7C, 0x77);

    mnPlayers1PBonusMakeNumber(gobj, (s32)mnRemixBonusModesGetRecord(mode, fkind), MNREMIX_BONUSMODES_RECORD_NUMBER_X,
                               MNREMIX_BONUSMODES_RECORD_NUMBER_Y, sMNRemixBonusModesDigitColors, mnRemixBonusModesRecordDigits(mode), TRUE);

    if (mnRemixBonusModesHasDecimal(mode) != 0)
    {
        mnRemixBonusModesMakeDecimal(gobj, files[MNREMIX_BONUSMODES_FILE_DIGITS]);
    }
    return TRUE;
}

sb32 mnRemixBonusModesIsBonus3(void)
{
    return sc1PRemixModeIs(SC1PREMIX_MODE_BONUS3);
}

u32 mnRemixBonusModesGetBonus3Time(s32 fkind)
{
    return sc1PRemixBonus3RecordGet(fkind);
}

sb32 mnRemixBonusModesShowsTime(s32 fkind, sb32 is_complete)
{
    return (mnRemixBonusModesRulesShowsTime(sc1PRemixModeGet(), (fkind == REMIX_ID_RANDOM) ? 1 : 0, (is_complete != FALSE) ? 1 : 0) != 0) ? TRUE : FALSE;
}

sb32 mnRemixBonusModesShowsTotal(sb32 is_all_complete)
{
    return (mnRemixBonusModesRulesShowsTotal(sc1PRemixModeGet(), (is_all_complete != FALSE) ? 1 : 0) != 0) ? TRUE : FALSE;
}

/** Sum of the record of every fighter that has a row in the save (total_kos_or_feet_._loop). */
static u32 mnRemixBonusModesSumAllRecords(s32 mode)
{
    unsigned int records[GMCOMMON_FIGHTERS_SAVE_NUM];
    s32 fkind;

    for (fkind = 0; fkind < GMCOMMON_FIGHTERS_SAVE_NUM; fkind++)
    {
        records[fkind] = mnRemixBonusModesGetRecord(mode, fkind);
    }
    return mnRemixBonusModesSumRecords(records, GMCOMMON_FIGHTERS_SAVE_NUM);
}

sb32 mnRemixBonusModesMakeTotal(GObj *gobj, SObj *label, void **files)
{
    s32 mode = sc1PRemixModeGet();
    s32 digits = mnRemixBonusModesTotalDigits(mode);

    if (digits == 0)
    {
        return FALSE;
    }
    label->sprite.width = MNREMIX_BONUSMODES_TOTAL_LABEL_WIDTH;
    label->pos.x = MNREMIX_BONUSMODES_TOTAL_LABEL_X;

    mnPlayers1PBonusMakeNumber(gobj, (s32)mnRemixBonusModesSumAllRecords(mode), MNREMIX_BONUSMODES_TOTAL_NUMBER_X,
                               MNREMIX_BONUSMODES_TOTAL_NUMBER_Y, sMNRemixBonusModesDigitColors, digits, TRUE);
    return TRUE;
}

/** Rolls a fighter for the Random token, skipping the locked ones and the last six rolls of the player (get_random_char_id_). */
static s32 mnRemixBonusModesRollFighterKind(s32 player)
{
    s32 attempt;
    s32 fkind;

    for (attempt = 0; attempt < MNREMIX_BONUSMODES_ROLL_ATTEMPTS; attempt++)
    {
        fkind = mnRemixRandomPickFighterKind();

        if ((fkind != nFTKindNull) && (mnPlayers1PBonusCheckFighterLocked(fkind) == FALSE) && (mnRemixCssRandomKeepRoll(player, fkind) != FALSE))
        {
            return fkind;
        }
    }
    return nFTKindMario;
}

void mnRemixBonusModesResolveRandom(s32 player, s32 *fkind, s32 *costume)
{
    if (*fkind != REMIX_ID_RANDOM)
    {
        return;
    }
    *fkind = mnRemixBonusModesRollFighterKind(player);
    *costume = mnRemixCostumeGetRandomLegal(*fkind);
}

sb32 mnRemixBonusModesIsBonusBookendFighter(s32 fkind)
{
    s32 index;

    for (index = 0; index < REMIX_CSS_NUM_BONUS_CHARS; index++)
    {
        if (kRemixCssBonusIds[index] == fkind)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static sb32 mnRemixBonusModesIsBookendCell(s32 fkind)
{
    return ((fkind == REMIX_ID_RANDOM) || (mnRemixBonusModesIsBonusBookendFighter(fkind) != FALSE)) ? TRUE : FALSE;
}

f32 mnRemixBonusModesGetCellX(s32 fkind, s32 portrait)
{
    if (fkind == REMIX_ID_RANDOM)
    {
        return (f32)MNREMIX_CSS_RANDOM_BOOKEND_X;
    }
    return (mnRemixBonusModesIsBookendCell(fkind) != FALSE) ? (f32)MNREMIX_CSS_BOOKEND_X : mnRemixCssCellX(portrait);
}

f32 mnRemixBonusModesGetCellY(s32 fkind, s32 portrait)
{
    return (mnRemixBonusModesIsBookendCell(fkind) != FALSE) ? (f32)MNREMIX_CSS_BOOKEND_Y : mnRemixCssCellY(portrait);
}

f32 mnRemixBonusModesGetCellWidth(s32 fkind)
{
    return (mnRemixBonusModesIsBookendCell(fkind) != FALSE) ? (f32)MNREMIX_CSS_BOOKEND_WIDTH : (f32)REMIX_CSS_PORTRAIT_WIDTH;
}

f32 mnRemixBonusModesGetCellHeight(s32 fkind)
{
    return (mnRemixBonusModesIsBookendCell(fkind) != FALSE) ? (f32)MNREMIX_CSS_BOOKEND_HEIGHT : (f32)REMIX_CSS_PORTRAIT_HEIGHT;
}

sb32 mnRemixBonusModesUpdateScroll(s32 player, s32 held_player, s32 fkind, u16 held_buttons)
{
    if (held_player < 0)
    {
        mnRemixCssBonusScroll(player, player, 0, FALSE);
        return FALSE;
    }
    return mnRemixCssBonusScroll(held_player, player, held_buttons, (mnRemixBonusModesIsBonusBookendFighter(fkind) != FALSE) ? TRUE : FALSE);
}

void mnRemixBonusModesUpdateHint(s32 token, sb32 is_token_held, s32 fkind, void **files)
{
    sb32 is_visible = ((is_token_held != FALSE) && (mnRemixBonusModesIsBonusBookendFighter(fkind) != FALSE)) ? TRUE : FALSE;

    mnRemixCssBonusUpdateHint(token, is_visible, files[MNREMIX_BONUSMODES_FILE_COMMON]);
}

sb32 mnRemixBonusModesRandomBookendContains(s32 pos_x, s32 pos_y)
{
    return (mnRemixCssRandomBookendContains(pos_x, pos_y) != 0) ? TRUE : FALSE;
}

/** The outline of the Option box of the 1P game, stretched to the panel (Bonus.asm setup_, "sideways U"). */
static void mnRemixBonusModesMakeOutline(void *mode_file)
{
    GObj *gobj = ifRemixSpriteMakeFromFile(mode_file, llMNPlayers1PModeOptionOutlineSprite, MNREMIX_BONUSMODES_GOBJ_LINK, MNREMIX_BONUSMODES_DL_LINK,
                                           MNREMIX_BONUSMODES_OUTLINE_X, MNREMIX_BONUSMODES_OUTLINE_Y, MNREMIX_BONUSMODES_BAR_RGBA, MNREMIX_BONUSMODES_OPTION_PALETTE, 1.0F);
    SObj *sobj;

    if (gobj == NULL)
    {
        return;
    }
    sobj = SObjGetStruct(gobj);
    sobj->cms = G_TX_WRAP;
    sobj->cmt = G_TX_MIRROR;
    sobj->masks = 0;
    sobj->maskt = MNREMIX_BONUSMODES_OUTLINE_MASKT;
    sobj->lrs = MNREMIX_BONUSMODES_OUTLINE_LRS;
    sobj->lrt = MNREMIX_BONUSMODES_OUTLINE_LRT;
}

typedef struct MNRemixBonusModesStageRow
{
    GObj *arrows[2];
    GObj *icons[2];
    GObj *set_stage_button;
    void *icon_file;

} MNRemixBonusModesStageRow;

static MNRemixBonusModesStageRow sMNRemixBonusModesStageRow;

/** The STAGE row exists on the Bonus 1 and 2 screens only (Bonus.asm setup_ and arrow_routine_ hide it for the other modes). */
static sb32 mnRemixBonusModesHasStageRow(void)
{
    return (sc1PRemixModeGet() == SC1PREMIX_MODE_STANDARD) ? TRUE : FALSE;
}

/** Makes the one or two stock icons of the picked stage again (update_stage_and_icons_). */
static void mnRemixBonusModesMakeStageIcons(void)
{
    s32 first;
    s32 second;
    s32 count;
    s32 index;
    s32 fkinds[2];

    for (index = 0; index < 2; index++)
    {
        if (sMNRemixBonusModesStageRow.icons[index] != NULL)
        {
            gcEjectGObj(sMNRemixBonusModesStageRow.icons[index]);
            sMNRemixBonusModesStageRow.icons[index] = NULL;
        }
    }
    sc1PBonusRemixGetStageIcons(sc1PBonusRemixGetStageIndex(), &first, &second);
    count = (second != 0) ? 2 : 1;
    fkinds[0] = first;
    fkinds[1] = second;

    for (index = 0; index < count; index++)
    {
        sMNRemixBonusModesStageRow.icons[index] = ifRemixSpriteMakeFromFile(sMNRemixBonusModesStageRow.icon_file, sc1PBonusRemixGetIconOffset(fkinds[index]),
                                                                           MNREMIX_BONUSMODES_GOBJ_LINK, MNREMIX_BONUSMODES_DL_LINK,
                                                                           sc1PBonusRemixIconX(count, index), SC1PBONUS_REMIX_ICON_Y, 0xFFFFFFFFu, 0x00000000u, 1.0F);
    }
}

static void mnRemixBonusModesMakeStageRow(void **files)
{
    sMNRemixBonusModesStageRow.icon_file = lbRelocGetExternHeapFile(SC1PBONUS_REMIX_ICON_FILE_ID, syTaskmanMalloc(lbRelocGetFileSize(SC1PBONUS_REMIX_ICON_FILE_ID), 0x10));
    sMNRemixBonusModesStageRow.arrows[0] = ifRemixSpriteMakeFromFile(files[MNREMIX_BONUSMODES_FILE_COMMON], llMNPlayersCommonArrowLSprite,
                                                                    MNREMIX_BONUSMODES_GOBJ_LINK, MNREMIX_BONUSMODES_DL_LINK,
                                                                    SC1PBONUS_REMIX_ARROW_LEFT_X, SC1PBONUS_REMIX_ARROW_Y, 0xFFFFFFFFu, 0x000000FFu, 1.0F);
    sMNRemixBonusModesStageRow.arrows[1] = ifRemixSpriteMakeFromFile(files[MNREMIX_BONUSMODES_FILE_COMMON], llMNPlayersCommonArrowRSprite,
                                                                    MNREMIX_BONUSMODES_GOBJ_LINK, MNREMIX_BONUSMODES_DL_LINK,
                                                                    SC1PBONUS_REMIX_ARROW_RIGHT_X, SC1PBONUS_REMIX_ARROW_Y, 0xFFFFFFFFu, 0x000000FFu, 1.0F);
    sMNRemixBonusModesStageRow.set_stage_button = ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_L, MNREMIX_BONUSMODES_GOBJ_LINK, MNREMIX_BONUSMODES_DL_LINK,
                                                                   SC1PBONUS_REMIX_SET_STAGE_BUTTON_X, SC1PBONUS_REMIX_SET_STAGE_BUTTON_Y, 0x848484FFu, 0x303030FFu, 1.0F);
    mnRemixBonusModesMakeStageIcons();
}

void mnRemixBonusModesToggleMake(void **files)
{
    sMNRemixBonusModesArrows[0] = NULL;
    sMNRemixBonusModesArrows[1] = NULL;
    sMNRemixBonusModesBlinkTimer = 0;
    memset(&sMNRemixBonusModesStageRow, 0, sizeof(sMNRemixBonusModesStageRow));

    if ((sc1PRemixModeIs(SC1PREMIX_MODE_BONUS3) == FALSE) && (mnRemixBonusModesHasStageRow() == FALSE))
    {
        return;
    }
    ifRemixSpriteMakeFromFile(files[MNREMIX_BONUSMODES_FILE_1P_MODE], llMNPlayers1PModeOptionTextSprite, MNREMIX_BONUSMODES_GOBJ_LINK,
                              MNREMIX_BONUSMODES_DL_LINK, MNREMIX_BONUSMODES_OPTION_X, MNREMIX_BONUSMODES_OPTION_Y,
                              MNREMIX_BONUSMODES_OPTION_TINT, MNREMIX_BONUSMODES_OPTION_PALETTE, 1.0F);
    mnRemixBonusModesMakeOutline(files[MNREMIX_BONUSMODES_FILE_1P_MODE]);
    sMNRemixBonusModesArrows[0] = ifRemixSpriteMakeFromFile(files[MNREMIX_BONUSMODES_FILE_COMMON], llMNPlayersCommonArrowLSprite,
                                                            MNREMIX_BONUSMODES_GOBJ_LINK, MNREMIX_BONUSMODES_DL_LINK,
                                                            MNREMIX_BONUSMODES_ARROW_LEFT_X, MNREMIX_BONUSMODES_ARROW_Y, 0xFFFFFFFFu, 0x000000FFu, 1.0F);
    sMNRemixBonusModesArrows[1] = ifRemixSpriteMakeFromFile(files[MNREMIX_BONUSMODES_FILE_COMMON], llMNPlayersCommonArrowRSprite,
                                                            MNREMIX_BONUSMODES_GOBJ_LINK, MNREMIX_BONUSMODES_DL_LINK,
                                                            MNREMIX_BONUSMODES_ARROW_RIGHT_X, MNREMIX_BONUSMODES_ARROW_Y, 0xFFFFFFFFu, 0x000000FFu, 1.0F);
    if (mnRemixBonusModesHasStageRow() != FALSE)
    {
        mnRemixBonusModesMakeStageRow(files);
    }
}

static void mnRemixBonusModesSetFlags(GObj *gobj, sb32 is_visible)
{
    if (gobj != NULL)
    {
        gobj->flags = (is_visible != FALSE) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;
    }
}

/** Shows the STAGE row, its arrows and icons while the choice is Remix, and "L: Set Stage" while a token is held over a fighter (arrow_routine_, check_input_). */
static void mnRemixBonusModesUpdateStageRow(sb32 is_blink_on, sb32 is_token_held, s32 fkind)
{
    sb32 is_remix = sc1PBonusRemixIsRemixChoice();
    sb32 is_set_stage = (sc1PBonusRemixRulesSetStageIsVisible(1, (is_remix != FALSE) ? 1 : 0, (is_token_held != FALSE) ? 1 : 0, (fkind != nFTKindNull) ? 1 : 0) != 0) ? TRUE : FALSE;
    s32 index;

    for (index = 0; index < 2; index++)
    {
        mnRemixBonusModesSetFlags(sMNRemixBonusModesStageRow.arrows[index], ((is_remix != FALSE) && (is_blink_on != FALSE)) ? TRUE : FALSE);
        mnRemixBonusModesSetFlags(sMNRemixBonusModesStageRow.icons[index], is_remix);
    }
    mnRemixBonusModesSetFlags(sMNRemixBonusModesStageRow.set_stage_button, is_set_stage);

    if (is_remix != FALSE)
    {
        remix_hud_text_put(MNREMIX_BONUSMODES_TEXT_SLOT_STAGE_LABEL, "STAGE:", SC1PBONUS_REMIX_STAGE_LABEL_X, SC1PBONUS_REMIX_STAGE_LABEL_Y,
                           MNREMIX_BONUSMODES_MODE_LABEL_RGBA, MNREMIX_BONUSMODES_MODE_SCALE, REMIX_HUD_ALIGN_RIGHT);
    }
    if (is_set_stage != FALSE)
    {
        remix_hud_text_put(MNREMIX_BONUSMODES_TEXT_SLOT_SET_STAGE, ": Set Stage", SC1PBONUS_REMIX_SET_STAGE_TEXT_X, SC1PBONUS_REMIX_SET_STAGE_TEXT_Y,
                           0xFFFFFFFFu, SC1PBONUS_REMIX_SET_STAGE_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
    }
}

void mnRemixBonusModesToggleUpdate(sb32 is_token_held, s32 fkind)
{
    s32 course;
    s32 is_right;
    sb32 is_blink_on;

    if (sMNRemixBonusModesArrows[0] == NULL)
    {
        return;
    }
    course = sc1PRemixBonus3CourseGet();
    sMNRemixBonusModesBlinkTimer = mnRemixBonusModesToggleBlinkNext(sMNRemixBonusModesBlinkTimer);
    is_blink_on = (mnRemixBonusModesToggleBlinkIsOn(sMNRemixBonusModesBlinkTimer) != 0) ? TRUE : FALSE;

    for (is_right = 0; is_right < 2; is_right++)
    {
        mnRemixBonusModesSetFlags(sMNRemixBonusModesArrows[is_right],
                                  ((mnRemixBonusModesToggleArrowExists(course, is_right) != 0) && (is_blink_on != FALSE)) ? TRUE : FALSE);
    }
    remix_hud_rect_put(MNREMIX_BONUSMODES_BAR_SLOT, MNREMIX_BONUSMODES_BAR_X, MNREMIX_BONUSMODES_BAR_Y, MNREMIX_BONUSMODES_BAR_WIDTH,
                       MNREMIX_BONUSMODES_BAR_HEIGHT, MNREMIX_BONUSMODES_BAR_RGBA);
    remix_hud_text_put(MNREMIX_BONUSMODES_TEXT_SLOT_MODE_LABEL, "MODE:", MNREMIX_BONUSMODES_MODE_LABEL_X, MNREMIX_BONUSMODES_MODE_Y,
                       MNREMIX_BONUSMODES_MODE_LABEL_RGBA, MNREMIX_BONUSMODES_MODE_SCALE, REMIX_HUD_ALIGN_RIGHT);
    remix_hud_text_put(MNREMIX_BONUSMODES_TEXT_SLOT_MODE_NAME, (course == SC1PREMIX_BONUS3_COURSE_REMIX) ? "REMIX" : "NORMAL",
                       MNREMIX_BONUSMODES_MODE_NAME_X, MNREMIX_BONUSMODES_MODE_Y,
                       (course == SC1PREMIX_BONUS3_COURSE_REMIX) ? MNREMIX_BONUSMODES_MODE_REMIX_RGBA : MNREMIX_BONUSMODES_MODE_NORMAL_RGBA,
                       MNREMIX_BONUSMODES_MODE_SCALE, REMIX_HUD_ALIGN_CENTER);

    if (sMNRemixBonusModesStageRow.icon_file != NULL)
    {
        mnRemixBonusModesUpdateStageRow(is_blink_on, is_token_held, fkind);
    }
}

/** Press on an arrow of the MODE row or, with the Remix choice, of the STAGE row. */
static sb32 mnRemixBonusModesPressStageArrow(s32 pos_x, s32 pos_y)
{
    s32 is_right;

    if ((sMNRemixBonusModesStageRow.icon_file == NULL) || (sc1PBonusRemixIsRemixChoice() == FALSE))
    {
        return FALSE;
    }
    for (is_right = 0; is_right < 2; is_right++)
    {
        SObj *arrow = SObjGetStruct(sMNRemixBonusModesStageRow.arrows[is_right]);

        if (mnRemixBonusModesToggleArrowHit(pos_x, pos_y, (s32)arrow->pos.x, (s32)arrow->pos.y, arrow->sprite.width, arrow->sprite.height, is_right) == 0)
        {
            continue;
        }
        sc1PBonusRemixSetStageIndex(sc1PBonusRemixStageStep(sc1PBonusRemixGetStageIndex(), (is_right != 0) ? 1 : -1, SC1PBONUS_REMIX_STAGE_COUNT));
        mnRemixBonusModesMakeStageIcons();

        return TRUE;
    }
    return FALSE;
}

sb32 mnRemixBonusModesTogglePress(s32 pos_x, s32 pos_y)
{
    s32 course;
    s32 is_right;

    if (sMNRemixBonusModesArrows[0] == NULL)
    {
        return FALSE;
    }
    course = sc1PRemixBonus3CourseGet();

    for (is_right = 0; is_right < 2; is_right++)
    {
        SObj *arrow = SObjGetStruct(sMNRemixBonusModesArrows[is_right]);

        if ((mnRemixBonusModesToggleArrowExists(course, is_right) == 0) ||
            (mnRemixBonusModesToggleArrowHit(pos_x, pos_y, (s32)arrow->pos.x, (s32)arrow->pos.y, arrow->sprite.width, arrow->sprite.height, is_right) == 0))
        {
            continue;
        }
        sc1PRemixBonus3SetCourse(mnRemixBonusModesToggleStep(course, (is_right != 0) ? 1 : -1));
        return TRUE;
    }
    return mnRemixBonusModesPressStageArrow(pos_x, pos_y);
}

sb32 mnRemixBonusModesIsManualStart(void)
{
    return ((mnRemixBonusModesHasStageRow() != FALSE) && (sc1PBonusRemixIsRemixChoice() != FALSE)) ? TRUE : FALSE;
}

sb32 mnRemixBonusModesOnButtonRelease(u16 released_buttons, sb32 is_token_held, s32 fkind)
{
    s32 stage_index;

    if ((sMNRemixBonusModesStageRow.icon_file == NULL) || ((released_buttons & L_TRIG) == 0) ||
        (sc1PBonusRemixRulesSetStageIsVisible(1, (sc1PBonusRemixIsRemixChoice() != FALSE) ? 1 : 0, (is_token_held != FALSE) ? 1 : 0, (fkind != nFTKindNull) ? 1 : 0) == 0))
    {
        return FALSE;
    }
    if (fkind == REMIX_ID_RANDOM)
    {
        stage_index = syUtilsRandIntRange(SC1PBONUS_REMIX_STAGE_COUNT);
    }
    else stage_index = sc1PBonusRemixFindStageIndex(FALSE, sc1PBonusRemixGetDefaultStageId(FALSE, fkind));

    if (stage_index < 0)
    {
        return FALSE;
    }
    sc1PBonusRemixSetStageIndex(stage_index);
    mnRemixBonusModesMakeStageIcons();

    return TRUE;
}

#endif
