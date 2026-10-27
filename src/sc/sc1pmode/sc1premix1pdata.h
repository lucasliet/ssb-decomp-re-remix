#ifndef _SC1PREMIX1PDATA_H_
#define _SC1PREMIX1PDATA_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sc/sctypes.h>
#include "sc/sc1pmode/sc1premix1prules.h"

/*
 * Data of the Remix 1P path (SinglePlayerModes.asm), read from the Remix RAM image: the stage table, the pool of fighters it picks
 * from, the duos and the Kirby powers. Nothing here is stored in the repository, the values come from the user's Remix build.
 */

#define SC1PREMIX_1P_STAGE_COUNT 18
#define SC1PREMIX_1P_PATH_COUNT 14
#define SC1PREMIX_1P_POOL_COUNT 18
#define SC1PREMIX_1P_DUO_COUNT 13
#define SC1PREMIX_1P_KIRBY_POOL_COUNT 22
#define SC1PREMIX_1P_RANDOM_SLOTS 6

/** One fighter of the pool (a `match_pool` entry): its stage options and what the stage title card shows. */
typedef struct SC1PRemix1PFighter
{
    s32 fkind;
    s32 stages[SC1PREMIX_1P_STAGE_OPTIONS];
    u32 name_texture;
    u32 announcer_call;
    u32 model_scale;
    u32 progress_icon;

} SC1PRemix1PFighter;

/** One duo (a `duo_pool` entry): the main fighter's pool entry, its partner and what the title card shows. */
typedef struct SC1PRemix1PDuo
{
    s32 main_fkind;
    s32 main_stages[SC1PREMIX_1P_STAGE_OPTIONS];
    s32 main_pool_index;
    s32 partner_fkind;
    s32 loads_first;
    u32 announcer_call;
    u32 progress_icon;
    u32 name_texture;

} SC1PRemix1PDuo;

/** What the title card of a path slot shows (the per-slot tables `title_card_1_struct`, `announcer_calls`, `model_scale`, `name_textures` and `progress_icon_struct`). */
typedef struct SC1PRemix1PSlotCard
{
    u32 title_fkind;
    u32 name_texture;
    u32 announcer_call;
    u32 model_scale;
    u32 progress_icon;

} SC1PRemix1PSlotCard;

/** Reads a line of the Remix stage table. @param index 0 to SC1PREMIX_1P_STAGE_COUNT - 1. */
extern void sc1PRemix1PDataReadStage(s32 index, SC1PGameStage *stage);

/** Reads a line of the Remix CPU settings (MATCH_SETTINGS_PART2), which stands in for dSC1PGameComputerDesc. @param index 0 to SC1PREMIX_1P_STAGE_COUNT - 1. */
extern void sc1PRemix1PDataReadComputer(s32 index, SC1PGameComputer *computer);

/** Reads a fighter of the pool. @param index 0 to SC1PREMIX_1P_POOL_COUNT - 1. */
extern void sc1PRemix1PDataReadFighter(s32 index, SC1PRemix1PFighter *fighter);

/** Reads a duo. @param index 0 to SC1PREMIX_1P_DUO_COUNT - 1. */
extern void sc1PRemix1PDataReadDuo(s32 index, SC1PRemix1PDuo *duo);

/** Reads what the title card of a path slot shows. @param index 0 to SC1PREMIX_1P_PATH_COUNT - 1. */
extern void sc1PRemix1PDataReadSlotCard(s32 index, SC1PRemix1PSlotCard *card);

/** Reads a Kirby power and hat of the pool. @param index 0 to SC1PREMIX_1P_KIRBY_POOL_COUNT - 1. */
extern void sc1PRemix1PDataReadKirby(s32 index, s32 *power, s32 *hat);

/** Pool index of the fighter with this kind, or -1. */
extern s32 sc1PRemix1PDataFindFighter(s32 fkind);

/** Slot (stage table line) that the random pick number takes: the `match_slots` table, in the Remix build the lines 0, 1, 2, 5, 6 and 9. @param pick 0 to 5. */
extern s32 sc1PRemix1PDataRandomSlot(s32 pick);

#endif

#endif
