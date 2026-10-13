/**
 * ftremixfields.c - maps N64 FTStruct offsets used by Remix routines onto the
 * port's FTStruct. See ftremixfields.h.
 */

#ifdef PORT

#include "ftremixfields.h"
#include <port_log.h>

#define FTREMIX_N64_PASSIVE_VARS_START 0x0ADC
#define FTREMIX_N64_PASSIVE_VARS_END 0x0B14
#define FTREMIX_N64_STATUS_VARS_START 0x0B18
#define FTREMIX_N64_STATUS_VARS_END 0x0B4C

static void ftRemixFighterWordFail(FTStruct *fp, u32 n64_offset)
{
    port_log("ftRemixFighterWord: N64 FTStruct offset 0x%04X (fkind 0x%X) is outside the mapped scratch "
             "ranges 0x%04X-0x%04X and 0x%04X-0x%04X; map it to a named field\n",
             n64_offset, fp->fkind, FTREMIX_N64_PASSIVE_VARS_START, FTREMIX_N64_PASSIVE_VARS_END - 4,
             FTREMIX_N64_STATUS_VARS_START, FTREMIX_N64_STATUS_VARS_END - 4);
    port_exit_process(1);
}

static sb32 ftRemixIsWordInRange(u32 n64_offset, u32 start, u32 end, size_t port_size)
{
    return (n64_offset >= start) && ((n64_offset + sizeof(s32)) <= end) && ((n64_offset - start + sizeof(s32)) <= port_size);
}

s32 *ftRemixFighterWord(FTStruct *fp, u32 n64_offset)
{
    if ((n64_offset & 3) != 0)
    {
        ftRemixFighterWordFail(fp, n64_offset);
    }
    if (ftRemixIsWordInRange(n64_offset, FTREMIX_N64_PASSIVE_VARS_START, FTREMIX_N64_PASSIVE_VARS_END, sizeof(fp->passive_vars)))
    {
        return (s32 *)((u8 *)&fp->passive_vars + (n64_offset - FTREMIX_N64_PASSIVE_VARS_START));
    }
    if (ftRemixIsWordInRange(n64_offset, FTREMIX_N64_STATUS_VARS_START, FTREMIX_N64_STATUS_VARS_END, sizeof(fp->status_vars)))
    {
        return (s32 *)((u8 *)&fp->status_vars + (n64_offset - FTREMIX_N64_STATUS_VARS_START));
    }
    ftRemixFighterWordFail(fp, n64_offset);

    return NULL;
}

f32 *ftRemixFighterFloat(FTStruct *fp, u32 n64_offset)
{
    return (f32 *)ftRemixFighterWord(fp, n64_offset);
}

s8 *ftRemixFighterKirbyHat(FTStruct *fp)
{
    return &fp->modelpart_status[2].modelpart_id_base;
}

#endif
