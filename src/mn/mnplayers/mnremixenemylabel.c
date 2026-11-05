/**
 * mnremixenemylabel.c - labels of the Single Player Enemy option on the 1P character select, see mnremixenemylabel.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <reloc_data.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include "if/ifremixsprite.h"
#include "mn/mnplayers/mnremixenemylabel.h"
#include "remix/remix_hud_text.h"
#include "sc/sc1pmode/sc1premixenemy.h"
#include "sc/sc1pmode/sc1premixenemyrules.h"

#define ENEMY_LABEL_GOBJ_LINK 23
#define ENEMY_LABEL_DL_LINK 34
#define ENEMY_LABEL_X 108.0F
#define ENEMY_LABEL_TEXT_Y 130.0F
#define ENEMY_LABEL_PRESS_Y 140.0F
#define ENEMY_LABEL_TEXT_SCALE 0.5F
#define ENEMY_LABEL_TEXT_COLOR 0xFFFFFFFF
#define ENEMY_LABEL_TEXT_SLOT_ENEMY 120
#define ENEMY_LABEL_TEXT_SLOT_PRESS 121
/* The asm colour words end in 00; the sprite alpha is kept opaque. */
#define ENEMY_LABEL_PALETTE 0xFFFFFFFF

typedef struct EnemyLabelPort
{
    u32 sprite_offset;
    u32 tint_rgba;
} EnemyLabelPort;

static const EnemyLabelPort sEnemyLabelPorts[SC1PREMIX_ENEMY_PORT_COUNT] =
{
    { llMNPlayersCommon1PTextSprite, 0x8C1616FF },
    { llMNPlayersCommon2PTextSprite, 0x5C8EF2FF },
    { llMNPlayersCommon3PTextSprite, 0xBFB41DFF },
    { llMNPlayersCommon4PTextSprite, 0x249643FF }
};

static GObj *sEnemyLabelPortGObjs[SC1PREMIX_ENEMY_PORT_COUNT];
static s32 sEnemyLabelBlinkTimer;

void mnRemixEnemyLabelMake(void *common_file)
{
    s32 port;

    sEnemyLabelBlinkTimer = 0;

    for (port = 0; port < SC1PREMIX_ENEMY_PORT_COUNT; port++)
    {
        sEnemyLabelPortGObjs[port] = ifRemixSpriteMakeFromFile(common_file, sEnemyLabelPorts[port].sprite_offset, ENEMY_LABEL_GOBJ_LINK,
            ENEMY_LABEL_DL_LINK, ENEMY_LABEL_X, ENEMY_LABEL_TEXT_Y, sEnemyLabelPorts[port].tint_rgba, ENEMY_LABEL_PALETTE, 1.0F);

        if (sEnemyLabelPortGObjs[port] != NULL)
        {
            sEnemyLabelPortGObjs[port]->flags = GOBJ_FLAG_HIDDEN;
        }
    }
}

void mnRemixEnemyLabelUpdate(void)
{
    s32 enemy_port = sc1PRemixEnemyPortGet();
    sb32 is_text_visible;
    s32 port;

    is_text_visible = (sc1PRemixEnemyTextIsVisible(enemy_port, sc1PRemixEnemyBlinkIsOn(sEnemyLabelBlinkTimer)) != 0) ? TRUE : FALSE;
    sEnemyLabelBlinkTimer = sc1PRemixEnemyBlinkNext(sEnemyLabelBlinkTimer);

    if (is_text_visible != FALSE)
    {
        remix_hud_text_put(ENEMY_LABEL_TEXT_SLOT_ENEMY, "Enemy Player", ENEMY_LABEL_X, ENEMY_LABEL_TEXT_Y, ENEMY_LABEL_TEXT_COLOR,
                           ENEMY_LABEL_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
        remix_hud_text_put(ENEMY_LABEL_TEXT_SLOT_PRESS, "Press Z", ENEMY_LABEL_X, ENEMY_LABEL_PRESS_Y, ENEMY_LABEL_TEXT_COLOR,
                           ENEMY_LABEL_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
    }
    for (port = 0; port < SC1PREMIX_ENEMY_PORT_COUNT; port++)
    {
        if (sEnemyLabelPortGObjs[port] != NULL)
        {
            sEnemyLabelPortGObjs[port]->flags = (sc1PRemixEnemyPortLabelIsVisible(enemy_port, port + 1) != 0) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;
        }
    }
}

#endif
