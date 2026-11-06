/**
 * mnremixvsstamina.c - VS menu and CSS parts of the Remix Stamina rule (Stamina.asm vs_menu and vs_css).
 *
 * The rule order of the Rule button is Time, Stock, Stamina, Time Team, Stock Team, Stamina Team. The HP value is shown
 * in the Time/Stock button and scrolled there and in the CSS; gGMRemixStaminaHP keeps it (1..999).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/menu.h>
#include <reloc_data.h>
#include <sys/controller.h>
#include "gm/gmremixpanel.h"
#include "gm/gmremixstamina.h"
#include "mn/mnplayers/mnplayersvs.h"
#include "mn/mnvsmode/mnvsmode.h"
#include "mn/mnvsmode/mnremixvsstamina.h"
#include "mn/mnvsmode/mnremixvsstaminarules.h"

#define MNREMIX_VSSTAMINA_RULE_OFFSET 0x00006D28
#define MNREMIX_VSSTAMINA_LABEL_OFFSET 0x00006B28
#define MNREMIX_VSSTAMINA_HP_LABEL_OFFSET 0x00006E28
#define MNREMIX_VSSTAMINA_TEAM_SCALE 0.875F

static SObj* mnRemixVSStaminaMakeSObj(GObj *gobj, Sprite *sprite, f32 x, f32 y, SYColorRGB color)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, sprite);

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    sobj->sprite.red = color.r;
    sobj->sprite.green = color.g;
    sobj->sprite.blue = color.b;
    sobj->pos.x = x;
    sobj->pos.y = y;

    return sobj;
}

static void mnRemixVSStaminaSetScale(SObj *sobj, f32 scale)
{
    sobj->sprite.scalex = scale;
    sobj->sprite.scaley = scale;
}

sb32 mnRemixVSStaminaIsRule(s32 rule)
{
    if ((rule == nMNVSModeRuleStamina) || (rule == nMNVSModeRuleStaminaTeam))
    {
        return TRUE;
    }
    return FALSE;
}

s32 mnRemixVSStaminaGetRule(sb32 is_team)
{
    if (is_team != FALSE)
    {
        return nMNVSModeRuleStaminaTeam;
    }
    return nMNVSModeRuleStamina;
}

void mnRemixVSStaminaSaveRule(s32 rule)
{
    gGMRemixStaminaRule = mnRemixVSStaminaIsRule(rule);
}

/** TRUE when any controller holds a button of the given mask (Joypad.check_buttons_all_ with Joypad.HELD). */
static sb32 mnRemixVSStaminaIsButtonHeld(u16 button_mask)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        if (gSYControllerDevices[port].button_hold & button_mask)
        {
            return TRUE;
        }
    }
    return FALSE;
}

s32 mnRemixVSStaminaStepHP(s32 hp, sb32 is_increase)
{
    u16 button_mask = (is_increase != FALSE) ? (R_JPAD | R_TRIG | R_CBUTTONS) : (L_JPAD | L_TRIG | L_CBUTTONS);
    sb32 is_stick = (mnRemixVSStaminaIsButtonHeld(button_mask) == FALSE) ? TRUE : FALSE;

    return mnRemixVSStaminaRulesMenuStep(hp, is_increase, is_stick);
}

sb32 mnRemixVSStaminaScrollHP(sb32 is_increase)
{
    s32 hp = mnRemixVSStaminaStepHP(gGMRemixStaminaHP, is_increase);
    sb32 is_limit = (hp == ((is_increase != FALSE) ? GMREMIX_STAMINA_HP_MAX : GMREMIX_STAMINA_HP_MIN)) ? TRUE : FALSE;

    gGMRemixStaminaHP = hp;

    return is_limit;
}

void mnRemixVSStaminaRestrictDamage(void)
{
    s32 total_hp = gmRemixStaminaGetTotalHP();
    s32 port;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        if (gmRemixPanel(DAMAGE, port) > total_hp)
        {
            remix_panel_set(REMIX_PANEL_DAMAGE, port, total_hp);
        }
    }
}

void mnRemixVSStaminaScrollHPCSS(sb32 is_increase)
{
    gGMRemixStaminaHP = mnRemixVSStaminaRulesCssStep(gGMRemixStaminaHP, is_increase);

    mnRemixVSStaminaRestrictDamage();
}

void mnRemixVSStaminaMakeRuleValue(GObj *rule_value_gobj, void *file, s32 rule)
{
    SYColorRGB white = { 0xFF, 0xFF, 0xFF };
    SObj *sobj;
    Sprite *rule_sprite = lbRelocGetFileData(Sprite*, file, MNREMIX_VSSTAMINA_RULE_OFFSET);

    if (rule == nMNVSModeRuleStamina)
    {
        mnRemixVSStaminaMakeSObj(rule_value_gobj, rule_sprite, 176.0F, 78.0F, white);
        return;
    }
    sobj = mnRemixVSStaminaMakeSObj(rule_value_gobj, rule_sprite, 165.0F, 78.0F, white);
    mnRemixVSStaminaSetScale(sobj, MNREMIX_VSSTAMINA_TEAM_SCALE);

    sobj = mnRemixVSStaminaMakeSObj(rule_value_gobj, lbRelocGetFileData(Sprite*, file, llMNVSModeTeamTextSprite), 224.0F, 78.0F, white);
    mnRemixVSStaminaSetScale(sobj, MNREMIX_VSSTAMINA_TEAM_SCALE);
}

void mnRemixVSStaminaMakeTimeStockLabels(GObj *time_stock_button_gobj, void *file)
{
    SYColorRGB black = { 0x00, 0x00, 0x00 };

    mnRemixVSStaminaMakeSObj(time_stock_button_gobj, lbRelocGetFileData(Sprite*, file, MNREMIX_VSSTAMINA_LABEL_OFFSET), 106.0F, 114.0F, black);
    mnRemixVSStaminaMakeSObj(time_stock_button_gobj, lbRelocGetFileData(Sprite*, file, MNREMIX_VSSTAMINA_HP_LABEL_OFFSET), 212.0F, 120.0F, black);
}

void mnRemixVSStaminaMakeHPValue(GObj *time_stock_value_gobj, s32 hp, u32 *colors)
{
    s32 x;

    if (hp < 10)
    {
        x = 197;
    }
    else if (hp < 100)
    {
        x = 205;
    }
    else x = 209;

    mnVSModeMakeNumber(time_stock_value_gobj, hp, (f32)x, 116.0F, colors, 3, FALSE);
}

void mnRemixVSStaminaMakeCSSValue(GObj *rule_selector_gobj, s32 hp, u32 *colors)
{
    f32 x = (hp < 10) ? 210.0F : 214.0F;

    mnPlayersVSMakeGameRuleNumber(rule_selector_gobj, hp, x, 23.0F, colors, 3, FALSE);
}

#endif
