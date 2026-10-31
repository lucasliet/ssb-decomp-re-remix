/*
 * ftremixdkmode.c - DK Mode and Lanky Mode head and arm scale of Smash Remix (DKMode.asm _resize_head).
 *
 * The setting is DKMode.dk_mode_enabled: 0 off, 1 DK Mode, 2 Lanky Mode. It is written only by the 12cb portrait
 * easter egg (DKMode.asm _check_portraits); no entry of the Remix toggle menu drives it. Each frame, every rendered fighter has
 * the joints of its Character.fighter_DK_mode.table entry scaled: the first joint takes the mode scale, every later joint takes
 * the arm scale. The table holds one 4-byte entry per fighter kind, a zero-terminated list of bone indices, and an empty entry
 * falls back to the table base (the default list).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixdkmode.h"
#include <ft/fighter.h>
#include "remix/remix_debug.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define FTREMIX_DK_MODE_OFF 0
#define FTREMIX_DK_MODE_DK 1
#define FTREMIX_DK_MODE_LANKY 2

/* DKMode.asm HEAD_SIZE_MULTIPLIER 0x3FC0 and LANKY_SIZE_MULTIPLIER 0x3F00, as the upper halfword of a float32 */
static const f32 sFTRemixDKModeHeadScaleDK = 1.5F;
static const f32 sFTRemixDKModeHeadScaleLanky = 0.5F;
/* DKMode.asm ARM_SIZE_MULTIPLIER 0x3FC0 */
static const f32 sFTRemixDKModeArmScale = 1.5F;

static s32 sFTRemixDKModeSetting;
static sb32 sFTRemixDKModeIsEnvRead;

/**
 * DKMode.dk_mode_enabled. The asm keeps it in mutable Remix RAM, which the read-only RAM image cannot hold, so the port keeps it
 * here. Until a setter is called it comes from SSB64_DK_MODE (0, 1 or 2), the test shortcut for the 12cb portrait easter egg.
 *
 * @return FTREMIX_DK_MODE_OFF, FTREMIX_DK_MODE_DK or FTREMIX_DK_MODE_LANKY.
 */
static s32 ftRemixDKModeReadSetting(void)
{
    if (sFTRemixDKModeIsEnvRead == FALSE)
    {
        sFTRemixDKModeIsEnvRead = TRUE;
        sFTRemixDKModeSetting = remix_debug_env_int("SSB64_DK_MODE", FTREMIX_DK_MODE_OFF);
    }
    return sFTRemixDKModeSetting;
}

void ftRemixDKModeSetMode(s32 mode)
{
    sFTRemixDKModeIsEnvRead = TRUE;
    sFTRemixDKModeSetting = mode;
}

/**
 * TRUE for the fighters DKMode.asm leaves alone: Master Hand (REMIX_ID_BOSS = 0x0C in the roster) and Sandbag.
 */
static sb32 ftRemixDKModeIsSkipped(s32 fkind)
{
    return (fkind == REMIX_ID_BOSS) || (fkind == REMIX_ID_SANDBAG);
}

/**
 * Returns the address of the bone index list of fkind: its own table entry, or the table base when that entry is empty
 * (DKMode.asm _loop_initial, beqzl t6 falls back to the first entry of the table).
 */
static u32 ftRemixDKModeBoneListAddr(s32 fkind)
{
    u32 table_addr = REMIX_SYM_Character__fighter_DK_mode__table;
    u32 entry_addr = table_addr + (fkind * 4);

    if (remix_ram_image_read_word(entry_addr) == 0)
    {
        return table_addr;
    }
    return entry_addr;
}

/**
 * Sets the x, y and z scale of the joint DObj at fp->joints[nFTPartsJointCommonStart + bone]. A bone index outside the
 * joints array or a joint that is not loaded is skipped, where the asm would dereference it.
 */
static void ftRemixDKModeSetJointScale(FTStruct *fp, u32 bone, f32 scale)
{
    u32 joint_index = nFTPartsJointCommonStart + bone;
    DObj *joint;

    if (joint_index >= ARRAY_COUNT(fp->joints))
    {
        return;
    }
    joint = fp->joints[joint_index];
    if (joint == NULL)
    {
        return;
    }
    joint->scale.vec.f.x = scale;
    joint->scale.vec.f.y = scale;
    joint->scale.vec.f.z = scale;
}

/**
 * Walks the zero-terminated bone index list at list_addr: the first joint gets first_scale, every later joint gets the arm
 * scale (DKMode.asm _loop_start, lui at, ARM_SIZE_MULTIPLIER after the first store).
 */
static void ftRemixDKModeScaleBones(FTStruct *fp, u32 list_addr, f32 first_scale)
{
    f32 scale = first_scale;
    u32 offset;
    u32 bone;

    for (offset = 0; ; offset++)
    {
        bone = remix_ram_image_read_byte(list_addr + offset);
        if (bone == 0)
        {
            return;
        }
        ftRemixDKModeSetJointScale(fp, bone, scale);
        scale = sFTRemixDKModeArmScale;
    }
}

/**
 * Head and arm scale of DK Mode or Lanky Mode for one fighter (DKMode.asm _resize_head, RAM 0x800F296C, hooked in
 * ftDisplayMainProcDisplay right after fp = ftGetStruct(fighter_gobj)). Does nothing when the setting is off or the fighter
 * is Master Hand or Sandbag.
 */
void ftRemixDKModeResizeHead(FTStruct *fp)
{
    s32 mode = ftRemixDKModeReadSetting();
    f32 head_scale;

    if (mode == FTREMIX_DK_MODE_OFF)
    {
        return;
    }
    if (ftRemixDKModeIsSkipped(fp->fkind))
    {
        return;
    }
    head_scale = (mode == FTREMIX_DK_MODE_LANKY) ? sFTRemixDKModeHeadScaleLanky : sFTRemixDKModeHeadScaleDK;
    ftRemixDKModeScaleBones(fp, ftRemixDKModeBoneListAddr(fp->fkind), head_scale);
}

#endif
