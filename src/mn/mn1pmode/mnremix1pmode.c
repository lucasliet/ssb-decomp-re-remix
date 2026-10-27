/**
 * mnremix1pmode.c - Remix 1P menu pages (.toolchain/pkgs/smashremix-src/src/SinglePlayerMenus.asm), see mnremix1pmode.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/menu.h>
#include <sc/scene.h>
#include <sys/audio.h>
#include <reloc_data.h>
#include "mn/mn1pmode/mn1pmode.h"
#include "mn/mn1pmode/mnremix1pmode.h"
#include "sc/sc1pmode/sc1premixmode.h"

#define MNREMIX_1PMODE_BIG_TAB_PIECES 16
#define MNREMIX_1PMODE_PAGE_1_LAST 5
#define MNREMIX_1PMODE_PAGE_2_LAST 4

s32 gMNRemix1PModePage = MNREMIX_1PMODE_PAGE_1;

/* Rows in asm order, tab positions and text positions are the float constants of the tables. */
static const MNRemix1PModeRow dMNRemix1PModePage1[] =
{
    { FALSE, nSCKind1PGamePlayers, SC1PREMIX_MODE_STANDARD, MNREMIX_1PMODE_PAGE_1, 124.0F, 22.0F, 161.0F, 26.0F, 0x00002A28 },
    { FALSE, nSCKindPlayers1PTraining, SC1PREMIX_MODE_STANDARD, MNREMIX_1PMODE_PAGE_1, 99.0F, 57.0F, 107.0F, 60.0F, 0x00005AC8 },
    { TRUE, nSCKind1PBonus1Players, SC1PREMIX_MODE_STANDARD, MNREMIX_1PMODE_PAGE_1, 78.0F, 96.0F, 97.0F, 97.0F, 0x00005F28 },
    { TRUE, nSCKind1PBonus2Players, SC1PREMIX_MODE_STANDARD, MNREMIX_1PMODE_PAGE_1, 65.0F, 118.0F, 86.0F, 119.0F, 0x00006388 },
    { TRUE, nSCKind1PBonus2Players, SC1PREMIX_MODE_BONUS3, MNREMIX_1PMODE_PAGE_1, 56.0F, 140.0F, 75.0F, 141.0F, 0x000070A8 },
    { FALSE, nSCKind1PMode, SC1PREMIX_MODE_STANDARD, MNREMIX_1PMODE_PAGE_2, 36.0F, 168.0F, 50.0F, 171.0F, 0x00007FB8 }
};

static const MNRemix1PModeRow dMNRemix1PModePage2[] =
{
    { FALSE, nSCKind1PGamePlayers, SC1PREMIX_MODE_REMIX_1P, MNREMIX_1PMODE_PAGE_2, 124.0F, 22.0F, 160.0F, 25.0F, 0x00007838 },
    { FALSE, nSCKind1PGamePlayers, SC1PREMIX_MODE_ALLSTAR, MNREMIX_1PMODE_PAGE_2, 99.0F, 57.0F, 111.0F, 60.0F, 0x00008738 },
    { TRUE, nSCKind1PBonus2Players, SC1PREMIX_MODE_MULTIMAN, MNREMIX_1PMODE_PAGE_2, 78.0F, 96.0F, 97.0F, 97.0F, 0x000067E8 },
    { TRUE, nSCKind1PBonus2Players, SC1PREMIX_MODE_CRUEL, MNREMIX_1PMODE_PAGE_2, 65.0F, 118.0F, 86.0F, 119.0F, 0x00006C48 },
    { TRUE, nSCKind1PBonus2Players, SC1PREMIX_MODE_HRC, MNREMIX_1PMODE_PAGE_2, 56.0F, 140.0F, 75.0F, 141.0F, 0x00008B88 }
};

s32 mnRemix1PModeGetLastOption(void)
{
    return (gMNRemix1PModePage == MNREMIX_1PMODE_PAGE_1) ? MNREMIX_1PMODE_PAGE_1_LAST : MNREMIX_1PMODE_PAGE_2_LAST;
}

const MNRemix1PModeRow* mnRemix1PModeGetRow(s32 option)
{
    return (gMNRemix1PModePage == MNREMIX_1PMODE_PAGE_1) ? &dMNRemix1PModePage1[option] : &dMNRemix1PModePage2[option];
}

static s32 mnRemix1PModeGetColorKind(const MNRemix1PModeRow *row)
{
    return (row->is_small != FALSE) ? nMN1PModeOptionBonus1Practice : nMN1PModeOption1PGame;
}

void mnRemix1PModeSetButtonColors(GObj *gobj, s32 status, s32 option)
{
    mn1PModeSetOptionSpriteColors(gobj, status, mnRemix1PModeGetColorKind(mnRemix1PModeGetRow(option)));
}

static void mnRemix1PModeMakeSmallTab(GObj *gobj, const MNRemix1PModeRow *row, void *mn1p_file)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, mn1p_file, llMN1POptionTabSprite));

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->pos.x = row->box_x;
    sobj->pos.y = row->box_y;
}

GObj* mnRemix1PModeMakeButton(s32 option, void *common_file, void *mn1p_file, sb32 is_highlight)
{
    const MNRemix1PModeRow *row = mnRemix1PModeGetRow(option);
    GObj *gobj = gcMakeGObjSPAfter(0, NULL, 4, GOBJ_PRIORITY_DEFAULT);
    SObj *text_sobj;

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 2, GOBJ_PRIORITY_DEFAULT, ~0);

    if (row->is_small != FALSE)
    {
        mnRemix1PModeMakeSmallTab(gobj, row, mn1p_file);
    }
    else mn1PModeMakeOptionTab(gobj, row->box_x, row->box_y, MNREMIX_1PMODE_BIG_TAB_PIECES);

    mnRemix1PModeSetButtonColors(gobj, (is_highlight != FALSE) ? nMNOptionTabStatusHighlight : nMNOptionTabStatusNot, option);

    text_sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, mn1p_file, row->text_offset));
    text_sobj->sprite.attr &= ~SP_FASTCOPY;
    text_sobj->sprite.attr |= SP_TRANSPARENT;
    text_sobj->sprite.red = 0x00;
    text_sobj->sprite.green = 0x00;
    text_sobj->sprite.blue = 0x00;
    text_sobj->pos.x = row->text_x;
    text_sobj->pos.y = row->text_y;

    return gobj;
}

/* SinglePlayerMenus.asm set_cursor_index_: the order of the modes is the order of the second page, Bonus 3 is the fifth button of the first. */
static s32 mnRemix1PModeGetModeOption(s32 mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_REMIX_1P:
        return 0;

    case SC1PREMIX_MODE_ALLSTAR:
        return 1;

    case SC1PREMIX_MODE_MULTIMAN:
        return 2;

    case SC1PREMIX_MODE_CRUEL:
        return 3;

    default:
        return 4;
    }
}

s32 mnRemix1PModeGetStartOption(s32 prev_scene, s32 vanilla_option)
{
    if (prev_scene == nSCKindModeSelect)
    {
        gMNRemix1PModePage = MNREMIX_1PMODE_PAGE_1;
        return vanilla_option;
    }
    if (prev_scene == nSCKind1PMode)
    {
        return (gMNRemix1PModePage == MNREMIX_1PMODE_PAGE_1) ? MNREMIX_1PMODE_OPTION_REMIX_MODES : 0;
    }
    if (gSC1PRemixMode != SC1PREMIX_MODE_STANDARD)
    {
        return mnRemix1PModeGetModeOption(gSC1PRemixMode);
    }
    return vanilla_option;
}

extern int port_enhancement_classic_coop(void);
extern void port_classic_coop_set_context(int);

/* Classic Co-op serves the 1P game with the VS character select so a second player can join (see mn1PModeFuncRun). */
static s32 mnRemix1PModeGetScene(const MNRemix1PModeRow *row)
{
    if ((row->scene != nSCKind1PGamePlayers) || (row->mode != SC1PREMIX_MODE_STANDARD))
    {
        return row->scene;
    }
    if (port_enhancement_classic_coop() == FALSE)
    {
        port_classic_coop_set_context(0);
        return row->scene;
    }
    gSCManagerTransferBattleState.is_reset_players = TRUE;
    port_classic_coop_set_context(1);
    return nSCKindPlayersVS;
}

void mnRemix1PModeSelect(s32 option, s32 player)
{
    const MNRemix1PModeRow *row = mnRemix1PModeGetRow(option);

    gSC1PRemixMode = row->mode;
    gMNRemix1PModePage = row->page;

    gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
    gSCManagerSceneData.scene_curr = mnRemix1PModeGetScene(row);
    gSCManagerSceneData.player = player - 1;
}

void mnRemix1PModeBack(void)
{
    gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;

    if (gMNRemix1PModePage != MNREMIX_1PMODE_PAGE_1)
    {
        gMNRemix1PModePage = MNREMIX_1PMODE_PAGE_1;
        gSCManagerSceneData.scene_curr = nSCKind1PMode;
        return;
    }
    gSCManagerSceneData.scene_curr = nSCKindModeSelect;
}

#endif
