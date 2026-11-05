/**
 * ifremixdragonkinghud.c - Dragon King HUD glue and timer placement, see ifremixdragonkinghud.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixdragonkinghud.h"
#include "if/ifremixdragonkinghudrules.h"
#include "gm/gmremixpanel.h"
#include "gm/gmremixtoggle.h"
#include <ft/fighter.h>
#include <sc/scene.h>
#include <sc/scmanager.h>

#define IFREMIX_DKHUD_PORT_LAST 3

sb32 ifRemixDragonKingHudIsOn(void)
{
    return ifRemixDragonKingHudRulesIsOn(gmRemixToggle(dragon_king_hud), gSCManagerBattleState->gkind);
}

f32 ifRemixDragonKingHudPickY(f32 vanilla_y, f32 dragon_king_y)
{
    return (ifRemixDragonKingHudIsOn() != FALSE) ? dragon_king_y : vanilla_y;
}

s32 ifRemixDragonKingHudPickIntY(s32 vanilla_y, s32 dragon_king_y)
{
    return (ifRemixDragonKingHudIsOn() != FALSE) ? dragon_king_y : vanilla_y;
}

s32 ifRemixDragonKingHudTrainingLabelOffsetY(void)
{
    return (ifRemixDragonKingHudIsOn() != FALSE) ? IFREMIX_DKHUD_TRAINING_LABEL_OFFSET_Y : 0;
}

s32 ifRemixDragonKingHudDamageY(s32 vanilla_y, sb32 is_one_player_game)
{
    return ifRemixDragonKingHudRulesDamageY(gmRemixToggle(dragon_king_hud), gSCManagerBattleState->gkind, vanilla_y, is_one_player_game);
}

intptr_t ifRemixDragonKingHudDigitOffset(s32 image_id)
{
    return ifRemixDragonKingHudRulesDigitOffset(image_id);
}

s32 ifRemixDragonKingHudDigitWidth(s32 image_id)
{
    return ifRemixDragonKingHudRulesDigitWidth(image_id);
}

void ifRemixDragonKingHudApplyDamageColor(s32 player, u8 *red, u8 *green, u8 *blue)
{
    IFRemixDragonKingHudColorInput input;
    unsigned char rgb[3];

    if (ifRemixDragonKingHudIsOn() == FALSE)
    {
        return;
    }
    input.port = player;
    input.is_vs_teams_battle = (gSCManagerSceneData.scene_curr == nSCKindVSBattle) && (gSCManagerBattleState->is_team_battle != FALSE);
    input.team = gSCManagerBattleState->players[player].team;
    input.is_cpu = (gSCManagerBattleState->players[player].pkind != nFTPlayerKindMan);

    ifRemixDragonKingHudRulesColor(&input, rgb);

    *red = rgb[0];
    *green = rgb[1];
    *blue = rgb[2];
}

f32 ifRemixTimerGetDigitsY(void)
{
    return ifRemixDragonKingHudRulesTimerY(ifRemixDragonKingHudIsOn(), gmRemixPanel(INPUT_DISPLAY, IFREMIX_DKHUD_PORT_LAST));
}

#endif
