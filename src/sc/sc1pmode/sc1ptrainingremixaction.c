/**
 * sc1ptrainingremixaction.c - Show Action of the Remix Training, see sc1ptrainingremixaction.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/controller.h>
#include <sys/objman.h>
#include "gm/gmremixgroupdisplay.h"
#include "if/ifremixsprite.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "sc/sc1pmode/sc1ptrainingleftoverrules.h"
#include "sc/sc1pmode/sc1ptrainingremixaction.h"
#include "sc/sc1pmode/sc1ptrainingremixmenu.h"

#define SC1PTRAININGREMIXACTION_PORT_COUNT 4
#define SC1PTRAININGREMIXACTION_NAME_LENGTH 40
#define SC1PTRAININGREMIXACTION_WORD_SIZE 4

/* Training.asm setup_ 1634-1637: the L glyph and the two legend words, in the custom pause group (0x15). */
#define SC1PTRAININGREMIXACTION_GROUP_LEGEND 0x15
#define SC1PTRAININGREMIXACTION_DL_LINK 24
#define SC1PTRAININGREMIXACTION_TINT 0x848484FF
#define SC1PTRAININGREMIXACTION_PALETTE 0x303030FF
#define SC1PTRAININGREMIXACTION_LEGEND_SCALE 0.875F
#define SC1PTRAININGREMIXACTION_LEGEND_X 20.0F
#define SC1PTRAININGREMIXACTION_LEGEND_GLYPH_Y 16.0F
#define SC1PTRAININGREMIXACTION_LEGEND_SHOW_Y 27.25F
#define SC1PTRAININGREMIXACTION_LEGEND_ACTION_Y 38.5F
#define SC1PTRAININGREMIXACTION_SLOT_LEGEND_SHOW 113
#define SC1PTRAININGREMIXACTION_SLOT_LEGEND_ACTION 114

/* create_action_strings_ 1485-1553: rows of 26 pixels per port, the action name at y 55 and the frame count at y 66, scale 0.75. */
#define SC1PTRAININGREMIXACTION_SLOT_ACTION_FIRST 105
#define SC1PTRAININGREMIXACTION_SLOT_FRAME_FIRST 109
#define SC1PTRAININGREMIXACTION_ROW_X 20.0F
#define SC1PTRAININGREMIXACTION_ROW_STEP 26.0F
#define SC1PTRAININGREMIXACTION_ACTION_Y 55.0F
#define SC1PTRAININGREMIXACTION_FRAME_Y 66.0F
#define SC1PTRAININGREMIXACTION_ROW_SCALE 0.75F
#define SC1PTRAININGREMIXACTION_TEXT_COLOR 0xFFFFFFFF

typedef struct SC1PTrainingRemixActionPort
{
    s32 prev_action;
    s32 prev_hitlag;
    s32 frame_count;

} SC1PTrainingRemixActionPort;

static SC1PTrainingRemixActionPort sSC1PTrainingRemixActionPorts[SC1PTRAININGREMIXACTION_PORT_COUNT];
static sb32 sSC1PTrainingRemixActionIsShown;

void sc1PTrainingRemixActionStartScene(void)
{
    s32 port;

    for (port = 0; port < SC1PTRAININGREMIXACTION_PORT_COUNT; port++)
    {
        sSC1PTrainingRemixActionPorts[port].prev_action = 0;
        sSC1PTrainingRemixActionPorts[port].prev_hitlag = 0;
        sSC1PTrainingRemixActionPorts[port].frame_count = 0;
    }
}

void sc1PTrainingRemixActionMakeLegend(void)
{
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_L, SC1PTRAININGREMIXACTION_GROUP_LEGEND, SC1PTRAININGREMIXACTION_DL_LINK,
                      SC1PTRAININGREMIXACTION_LEGEND_X, SC1PTRAININGREMIXACTION_LEGEND_GLYPH_Y, SC1PTRAININGREMIXACTION_TINT,
                      SC1PTRAININGREMIXACTION_PALETTE, SC1PTRAININGREMIXACTION_LEGEND_SCALE);
    gmRemixGroupDisplaySet(SC1PTRAININGREMIXACTION_GROUP_LEGEND, 1);
}

void sc1PTrainingRemixActionResetFrameCount(s32 player)
{
    if (gSCManagerSceneData.scene_curr != nSCKind1PTrainingMode)
    {
        return;
    }
    if ((player < 0) || (player >= SC1PTRAININGREMIXACTION_PORT_COUNT))
    {
        return;
    }
    sSC1PTrainingRemixActionPorts[player].prev_action = -1;
}

/** update_frame_counts_ for one fighter. */
static void sc1PTrainingRemixActionCountFighter(FTStruct *fp)
{
    SC1PTrainingRemixActionPort *port;

    if (fp->player >= SC1PTRAININGREMIXACTION_PORT_COUNT)
    {
        return;
    }
    port = &sSC1PTrainingRemixActionPorts[fp->player];
    port->frame_count = sc1PTrainingLeftoverRulesNextFrameCount(port->prev_action, fp->status_id, port->prev_hitlag, fp->hitlag_tics,
                                                                port->frame_count);
    port->prev_action = fp->status_id;
    port->prev_hitlag = fp->hitlag_tics;
}

void sc1PTrainingRemixActionCountFrame(void)
{
    GObj *fighter_gobj;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        sc1PTrainingRemixActionCountFighter(ftGetStruct(fighter_gobj));
    }
}

/** update_actions_ 2181-2203: RAM address of the action name string of a fighter, 0 when its character has no action table. */
static u32 sc1PTrainingRemixActionGetNameAddress(const FTStruct *fp)
{
    s32 action = fp->status_id;
    u32 table;

    if (action < 0)
    {
        return 0;
    }
    if (sc1PTrainingLeftoverRulesIsSharedAction(action) != 0)
    {
        return remix_ram_image_read_word(REMIX_SYM_Action__shared_action_string_table + (action * SC1PTRAININGREMIXACTION_WORD_SIZE));
    }
    table = remix_ram_image_read_word(REMIX_SYM_Character__action_string__table + (fp->fkind * SC1PTRAININGREMIXACTION_WORD_SIZE));

    if (table == 0)
    {
        return 0;
    }
    return remix_ram_image_read_word(table + (sc1PTrainingLeftoverRulesGetUniqueIndex(action) * SC1PTRAININGREMIXACTION_WORD_SIZE));
}

/** Copies a printable ASCII string of the image; FALSE for a null pointer, an empty string or a byte that is not text. */
static sb32 sc1PTrainingRemixActionReadName(u32 address, char *name)
{
    s32 index;

    if (address == 0)
    {
        return FALSE;
    }
    for (index = 0; index < SC1PTRAININGREMIXACTION_NAME_LENGTH; index++)
    {
        u32 character = remix_ram_image_read_byte(address + index);

        if (character == 0)
        {
            break;
        }
        if ((character < 0x20) || (character > 0x7E))
        {
            return FALSE;
        }
        name[index] = (char)character;
    }
    name[index] = '\0';

    return (index > 0) ? TRUE : FALSE;
}

static void sc1PTrainingRemixActionPutRow(s32 port)
{
    GObj *fighter_gobj = gSCManagerBattleState->players[port].fighter_gobj;
    f32 row_offset = SC1PTRAININGREMIXACTION_ROW_STEP * port;
    char name[SC1PTRAININGREMIXACTION_NAME_LENGTH + 1];

    if (fighter_gobj == NULL)
    {
        return;
    }
    if (sc1PTrainingRemixActionReadName(sc1PTrainingRemixActionGetNameAddress(ftGetStruct(fighter_gobj)), name) != FALSE)
    {
        remix_hud_text_put(SC1PTRAININGREMIXACTION_SLOT_ACTION_FIRST + port, name, SC1PTRAININGREMIXACTION_ROW_X,
                           SC1PTRAININGREMIXACTION_ACTION_Y + row_offset, SC1PTRAININGREMIXACTION_TEXT_COLOR,
                           SC1PTRAININGREMIXACTION_ROW_SCALE, REMIX_HUD_ALIGN_LEFT);
    }
    remix_hud_text_putf(SC1PTRAININGREMIXACTION_SLOT_FRAME_FIRST + port, SC1PTRAININGREMIXACTION_ROW_X,
                        SC1PTRAININGREMIXACTION_FRAME_Y + row_offset, SC1PTRAININGREMIXACTION_TEXT_COLOR,
                        SC1PTRAININGREMIXACTION_ROW_SCALE, REMIX_HUD_ALIGN_LEFT, "%d",
                        (int)sSC1PTrainingRemixActionPorts[port].frame_count);
}

/** Joypad.check_buttons_all_ with PRESSED: any of the four controllers. */
static sb32 sc1PTrainingRemixActionIsLTapped(void)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        if (gSYControllerDevices[port].button_tap & L_TRIG)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void sc1PTrainingRemixActionPutLegend(void)
{
    remix_hud_text_put(SC1PTRAININGREMIXACTION_SLOT_LEGEND_SHOW, "Show", SC1PTRAININGREMIXACTION_LEGEND_X,
                       SC1PTRAININGREMIXACTION_LEGEND_SHOW_Y, SC1PTRAININGREMIXACTION_TEXT_COLOR,
                       SC1PTRAININGREMIXACTION_LEGEND_SCALE, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_put(SC1PTRAININGREMIXACTION_SLOT_LEGEND_ACTION, "Action", SC1PTRAININGREMIXACTION_LEGEND_X,
                       SC1PTRAININGREMIXACTION_LEGEND_ACTION_Y, SC1PTRAININGREMIXACTION_TEXT_COLOR,
                       SC1PTRAININGREMIXACTION_LEGEND_SCALE, REMIX_HUD_ALIGN_LEFT);
}

void sc1PTrainingRemixActionUpdate(void)
{
    sb32 is_paused = (gSCManagerBattleState->game_status == nSCBattleGameStatusPause) ? TRUE : FALSE;
    s32 port;

    if ((is_paused != FALSE) && (sc1PTrainingRemixActionIsLTapped() != FALSE))
    {
        sSC1PTrainingRemixActionIsShown = (sSC1PTrainingRemixActionIsShown == FALSE) ? TRUE : FALSE;
    }
    gmRemixGroupDisplaySet(SC1PTRAININGREMIXACTION_GROUP_LEGEND, (is_paused != FALSE) ? 0 : 1);

    if (is_paused != FALSE)
    {
        sc1PTrainingRemixActionPutLegend();
    }
    /* DEVIATION(overlay text): the asm draws the rows in room 0x39 under the Z menu panel; the overlay text would cross the menu rows. */
    if ((sSC1PTrainingRemixActionIsShown == FALSE) || (sc1PTrainingRemixMenuIsOpen() != FALSE))
    {
        return;
    }
    for (port = 0; port < SC1PTRAININGREMIXACTION_PORT_COUNT; port++)
    {
        sc1PTrainingRemixActionPutRow(port);
    }
}

#endif
