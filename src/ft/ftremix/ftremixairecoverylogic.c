/*
 * ftremixairecoverylogic.c - Character.recovery_logic dispatch of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm
 * custom_recovery_logic). The routines are Remix code with no port symbol for the shared ones, so the entry of a fighter is
 * matched with the entry of the fighter that registers it (clones copy the registering fighter's routine). The pure decisions
 * live in ftremixairecoverylogicrules.c, ftremixairecoverylogicsharedrules.c and ftremixairecoverylogicfighterrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <port_log.h>
#include <sys/taskman.h>
#include <sys/utils.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixairecoverylogic.h"
#include "ft/ftremix/ftremixairecoverylogicrules.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"

#define FTREMIX_AI_RLOGIC_TABLE_ENTRY_BYTES 4
#define FTREMIX_AI_RLOGIC_LOG_FKIND_MAX 64
#define FTREMIX_AI_RLOGIC_N64_PASSIVE_WORD 0x0ADC

/* Fighter that registers a recovery routine; its clones share the entry (Ness and Yoshi are two files with one body). */
typedef struct FTRemixAiRecoveryLogicOwner
{
    s32 fkind;
    s32 kind;

} FTRemixAiRecoveryLogicOwner;

static const FTRemixAiRecoveryLogicOwner sFTRemixAiRecoveryLogicOwners[] =
{
    { REMIX_ID_MARIO, FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO },
    { REMIX_ID_PIKACHU, FTREMIX_AI_RECOVERY_LOGIC_KIND_PIKACHU },
    { REMIX_ID_JIGGLY, FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF },
    { REMIX_ID_YOSHI, FTREMIX_AI_RECOVERY_LOGIC_KIND_YOSHI },
    { REMIX_ID_NESS, FTREMIX_AI_RECOVERY_LOGIC_KIND_YOSHI },
    { REMIX_ID_CAPTAIN, FTREMIX_AI_RECOVERY_LOGIC_KIND_CAPTAIN },
    { REMIX_ID_LINK, FTREMIX_AI_RECOVERY_LOGIC_KIND_LINK },
    { REMIX_ID_FOX, FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX },
    { REMIX_ID_BANJO, FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO },
    { REMIX_ID_WARIO, FTREMIX_AI_RECOVERY_LOGIC_KIND_WARIO },
    { REMIX_ID_MARTH, FTREMIX_AI_RECOVERY_LOGIC_KIND_MARTH },
    { REMIX_ID_CRASH, FTREMIX_AI_RECOVERY_LOGIC_KIND_CRASH },
    { REMIX_ID_DEDEDE, FTREMIX_AI_RECOVERY_LOGIC_KIND_DEDEDE },
    { REMIX_ID_FALCO, FTREMIX_AI_RECOVERY_LOGIC_KIND_FALCO },
    { REMIX_ID_SHEIK, FTREMIX_AI_RECOVERY_LOGIC_KIND_SHEIK },
    { REMIX_ID_MARINA, FTREMIX_AI_RECOVERY_LOGIC_KIND_MARINA },
    { REMIX_ID_SONIC, FTREMIX_AI_RECOVERY_LOGIC_KIND_SONIC },
    { REMIX_ID_PEACH, FTREMIX_AI_RECOVERY_LOGIC_KIND_PEACH }
};

static u64 sFTRemixAiRecoveryLogicLoggedFkinds;

/** Routine address that Character.recovery_logic holds for a fighter kind (0 when it has none or is outside the image). */
static u32 ftRemixAiRecoveryLogicGetTableEntry(s32 fkind)
{
    return remix_ram_image_read_word(REMIX_SYM_Character__recovery_logic__table +
                                     (fkind * FTREMIX_AI_RLOGIC_TABLE_ENTRY_BYTES));
}

/** Routine kind of a fighter kind: the kind of the owner that has the same table entry, NONE when nobody does. */
static s32 ftRemixAiRecoveryLogicGetKind(s32 fkind)
{
    u32 routine_addr = ftRemixAiRecoveryLogicGetTableEntry(fkind);
    s32 i;

    if (routine_addr == 0)
    {
        return FTREMIX_AI_RECOVERY_LOGIC_KIND_NONE;
    }
    for (i = 0; i < ARRAY_COUNT(sFTRemixAiRecoveryLogicOwners); i++)
    {
        if (ftRemixAiRecoveryLogicGetTableEntry(sFTRemixAiRecoveryLogicOwners[i].fkind) == routine_addr)
        {
            return sFTRemixAiRecoveryLogicOwners[i].kind;
        }
    }
    return FTREMIX_AI_RECOVERY_LOGIC_KIND_NONE;
}

/** Logs the first run of each fighter kind, once, so a match shows which routines fired. */
static void ftRemixAiRecoveryLogicLogFirstRun(FTStruct *fp, s32 kind, u32 routine_addr)
{
    u64 kind_bit;

    if ((fp->fkind < 0) || (fp->fkind >= FTREMIX_AI_RLOGIC_LOG_FKIND_MAX))
    {
        return;
    }
    kind_bit = (u64)1 << fp->fkind;
    if ((sFTRemixAiRecoveryLogicLoggedFkinds & kind_bit) != 0)
    {
        return;
    }
    sFTRemixAiRecoveryLogicLoggedFkinds |= kind_bit;
    port_log("ftRemixAiRecoveryLogic: fkind 0x%X level %d routine 0x%08X kind %d %s\n", fp->fkind, fp->level, routine_addr, kind,
             (kind != FTREMIX_AI_RECOVERY_LOGIC_KIND_NONE) ? "first run" : "not ported");
}

/** Fills the state the pure routines read (the N64 offsets are in ftremixairecoverylogicrules.h). */
static void ftRemixAiRecoveryLogicReadState(FTStruct *fp, s32 kind, FTRemixAiRecoveryLogicState *state)
{
    FTComputer *com = &fp->computer;

    state->level_ten = ftRemixAiIsLevelTen(fp->level) ? 1 : 0;
    state->action_id = fp->status_id;
    state->status_frame = fp->status_total_tics;
    state->lr = fp->lr;
    state->jumps_used = fp->jumps_used;
    state->jumps_max = fp->attr->jumps_max;
    state->has_item = (fp->item_gobj != NULL) ? 1 : 0;
    state->jump_command_active = (com->p_command == ftComputerGetInputScript(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_JUMP)) ? 1 : 0;
    state->word_adc = *ftRemixFighterWord(fp, FTREMIX_AI_RLOGIC_N64_PASSIVE_WORD);
    state->minions_full = (kind == FTREMIX_AI_RECOVERY_LOGIC_KIND_DEDEDE) ? ftRemixDededeDSPFieldMinionsFull(fp) : 0;
    state->frame_is_odd = (int)(dSYTaskmanUpdateCount & 1);
    state->buttons = fp->input.cp.button_inputs;
    state->x = fp->coll_data.p_translate->x;
    state->y = fp->coll_data.p_translate->y;
    state->vel_x = fp->physics.vel_air.x;
    state->vel_y = fp->physics.vel_air.y;
    state->air_speed_max_x = fp->attr->air_speed_max_x;
    state->air_friction = fp->attr->air_friction;
    state->cliff_catch_y = fp->attr->cliffcatch_coll.y;
    state->left_x = com->cliff_left_pos.x;
    state->left_y = com->cliff_left_pos.y;
    state->right_x = com->cliff_right_pos.x;
    state->right_y = com->cliff_right_pos.y;
    state->target_x = com->target_pos.x;
    state->target_y = com->target_pos.y;
}

/** Applies a routine's result in the asm order: target, stick, buttons, then the command. */
static void ftRemixAiRecoveryLogicApplyResult(FTStruct *fp, const FTRemixAiRecoveryLogicResult *result)
{
    if (result->sets_target != 0)
    {
        fp->computer.target_pos.x = result->target_x;
        fp->computer.target_pos.y = result->target_y;
    }
    if (result->sets_stick_x != 0)
    {
        fp->input.cp.stick_range.x = result->stick_x;
    }
    if (result->sets_stick_y != 0)
    {
        fp->input.cp.stick_range.y = result->stick_y;
    }
    if (result->sets_buttons != 0)
    {
        fp->input.cp.button_inputs = (u16)result->buttons;
    }
    if (result->command != FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE)
    {
        ftComputerSetCommandImmediate(fp, result->command);
    }
}

void ftRemixAiRecoveryLogicRunFighter(FTStruct *fp)
{
    FTRemixAiRecoveryLogicState state;
    FTRemixAiRecoveryLogicResult result;
    u32 routine_addr = ftRemixAiRecoveryLogicGetTableEntry(fp->fkind);
    s32 kind;

    if (routine_addr == 0)
    {
        return;
    }
    kind = ftRemixAiRecoveryLogicGetKind(fp->fkind);
    ftRemixAiRecoveryLogicLogFirstRun(fp, kind, routine_addr);

    if (kind == FTREMIX_AI_RECOVERY_LOGIC_KIND_NONE)
    {
        return;
    }
    ftRemixAiRecoveryLogicReadState(fp, kind, &state);
    ftRemixAiRecoveryLogicRun(kind, &state, &result);

    if ((result.roll_modulus > 0) && (syUtilsRandIntRange(result.roll_modulus) != 0))
    {
        return;
    }
    ftRemixAiRecoveryLogicApplyResult(fp, &result);
}

#endif
