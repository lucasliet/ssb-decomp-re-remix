#include <ft/fighter.h>
#include "ft/ftremix/ftremixhooks.h"

#ifdef PORT
#include <enhancements/enhancements.h>
#endif

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x8013F2A0
void ftCommonKneeBendProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    fp->status_vars.common.kneebend.anim_frame += DObjGetStruct(fighter_gobj)->anim_speed;

    if 
    (
        (fp->status_vars.common.kneebend.input_source == FTCOMMON_KNEEBEND_INPUT_TYPE_BUTTON)&&
        (fp->status_vars.common.kneebend.anim_frame <= FTCOMMON_KNEEBEND_SHORTHOP_FRAMES)    &&
        (fp->input.pl.button_release & (R_CBUTTONS | L_CBUTTONS | D_CBUTTONS | U_CBUTTONS))
    )
    {
        fp->status_vars.common.kneebend.is_shorthop = TRUE;
    }
#ifdef REGION_REMIX
    if (ftRemixSizeJumpSquatLength(fp, attr->kneebend_anim_length) <= fp->status_vars.common.kneebend.anim_frame)
#else
    if (attr->kneebend_anim_length <= fp->status_vars.common.kneebend.anim_frame)
#endif 
    {
        ftCommonJumpSetStatus(fighter_gobj);
    }
}

// 0x8013F334
void ftCommonKneeBendProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftCommonSpecialHiCheckInterruptCommon(fighter_gobj) == FALSE)
    {
        if (ftCommonAttackHi4CheckInterruptKneeBend(fighter_gobj) == FALSE)
        {
            if (fp->status_vars.common.kneebend.jump_force < fp->input.pl.stick_range.y)
            {
                fp->status_vars.common.kneebend.jump_force = fp->input.pl.stick_range.y;
            }
        }
    }
}

// 0x8013F3A0
void ftCommonKneeBendSetStatusParam(GObj *fighter_gobj, s32 status_id, s32 input_source)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    fp->status_vars.common.kneebend.jump_force = fp->input.pl.stick_range.y;
    fp->status_vars.common.kneebend.anim_frame = 0.0F;
    fp->status_vars.common.kneebend.input_source = input_source;
    fp->status_vars.common.kneebend.is_shorthop = FALSE;

    fp->is_special_interrupt = TRUE;
}

// 0x8013F408
void ftCommonKneeBendSetStatus(GObj *fighter_gobj, s32 input_source)
{
    ftCommonKneeBendSetStatusParam(fighter_gobj, nFTCommonStatusKneeBend, input_source);
}

// 0x8013F42C
void ftCommonGuardKneeBendSetStatus(GObj *fighter_gobj, s32 input_source)
{
    ftCommonKneeBendSetStatusParam(fighter_gobj, nFTCommonStatusGuardKneeBend, input_source);
}

// 0x8013F450
sb32 ftCommonKneeBendCheckButtonTap(FTStruct *fp)
{
    if (fp->input.pl.button_tap & (R_CBUTTONS | L_CBUTTONS | D_CBUTTONS | U_CBUTTONS))
    {
#ifdef REGION_REMIX
        return ftRemixTauntButtonAllowsJump(fp, fp->input.pl.button_tap);
#else
        return TRUE;
#endif
    }
    else return FALSE;
}

// 0x8013F474
s32 ftCommonKneeBendGetInputTypeCommon(FTStruct *fp)
{
#ifdef PORT
    sb32 tap_jump_disabled = port_enhancement_tap_jump_disabled(fp->player);
#else
    sb32 tap_jump_disabled = FALSE;
#endif
#ifdef REGION_REMIX
    tap_jump_disabled = tap_jump_disabled || ftRemixDpadControlDisablesStickJump(fp);
#endif

    if (!tap_jump_disabled && (fp->input.pl.stick_range.y >= FTCOMMON_KNEEBEND_STICK_RANGE_MIN) && (fp->tap_stick_y <= FTCOMMON_KNEEBEND_BUFFER_TICS_MAX))
    {
        return FTCOMMON_KNEEBEND_INPUT_TYPE_STICK;
    }
    else if (ftCommonKneeBendCheckButtonTap(fp) != FALSE)
    {
        return FTCOMMON_KNEEBEND_INPUT_TYPE_BUTTON;
    }
    else return FTCOMMON_KNEEBEND_INPUT_TYPE_NONE;
}

// 0x8013F4D0
sb32 ftCommonKneeBendCheckInterruptCommon(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 input_source;

    if (ftHammerCheckHoldHammer(fighter_gobj) != FALSE)
    {
        return ftCommonHammerKneeBendCheckInterruptCommon(fighter_gobj);
    }
    input_source = ftCommonKneeBendGetInputTypeCommon(fp);

    if (input_source != FTCOMMON_KNEEBEND_INPUT_TYPE_NONE)
    {
        ftCommonKneeBendSetStatus(fighter_gobj, input_source);

        return TRUE;
    }
    else return FALSE;
}

// 0x8013F53C
s32 ftCommonKneeBendGetInputTypeRun(FTStruct *fp)
{
#ifdef PORT
    sb32 tap_jump_disabled = port_enhancement_tap_jump_disabled(fp->player);
#else
    sb32 tap_jump_disabled = FALSE;
#endif
#ifdef REGION_REMIX
    tap_jump_disabled = ftRemixDpadControlHandlesRunStickUp(fp) || tap_jump_disabled;
#endif

    if (!tap_jump_disabled && (fp->input.pl.stick_range.y > FTCOMMON_KNEEBEND_RUN_STICK_RANGE_MIN) && (fp->tap_stick_y <= FTCOMMON_KNEEBEND_BUFFER_TICS_MAX))
    {
        return FTCOMMON_KNEEBEND_INPUT_TYPE_STICK;
    }
    else if (ftCommonKneeBendCheckButtonTap(fp) != FALSE)
    {
        return FTCOMMON_KNEEBEND_INPUT_TYPE_BUTTON;
    }
    else return FTCOMMON_KNEEBEND_INPUT_TYPE_NONE;
}

// 0x8013F598
sb32 ftCommonKneeBendCheckInterruptRun(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 input_source;

    if (ftHammerCheckHoldHammer(fighter_gobj) != FALSE)
    {
        return ftCommonHammerKneeBendCheckInterruptCommon(fighter_gobj);
    }
    input_source = ftCommonKneeBendGetInputTypeRun(fp);

    if (input_source != FTCOMMON_KNEEBEND_INPUT_TYPE_NONE)
    {
        ftCommonKneeBendSetStatus(fighter_gobj, input_source);

        return TRUE;
    }
    else return FALSE;
}

// 0x8013F604
sb32 ftCommonGuardKneeBendCheckInterruptGuard(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 input_source = ftCommonKneeBendGetInputTypeCommon(fp);

    if ((input_source != FTCOMMON_KNEEBEND_INPUT_TYPE_NONE) && (fp->input.pl.button_hold & fp->input.button_mask_z))
    {
        ftCommonGuardKneeBendSetStatus(fighter_gobj, input_source);

        return TRUE;
    }
    else return FALSE;
}
