/**
 * sc1premix1pdata.c - Remix 1P path data read from the Remix RAM image, see sc1premix1pdata.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "sc/sc1pmode/sc1premix1pdata.h"

#define SC1PREMIX_1P_STAGE_STRIDE 0x10
#define SC1PREMIX_1P_COMPUTER_STRIDE 22
#define SC1PREMIX_1P_POOL_STRIDE 0x18
#define SC1PREMIX_1P_DUO_STRIDE 0x10
#define SC1PREMIX_1P_KIRBY_STRIDE 0xC

/*
 * The three per-slot tables are `insert`ed binaries that the symbol file leaves unlabelled: they follow the 18 stage lines of
 * MATCH_SETTINGS_PART1 one after the other, 14 words each.
 */
#define SC1PREMIX_1P_ANNOUNCER_CALLS (REMIX_SYM_SinglePlayerModes__MATCH_SETTINGS_PART1 + (SC1PREMIX_1P_STAGE_COUNT * SC1PREMIX_1P_STAGE_STRIDE))
#define SC1PREMIX_1P_MODEL_SCALES (SC1PREMIX_1P_ANNOUNCER_CALLS + (SC1PREMIX_1P_PATH_COUNT * 4))
#define SC1PREMIX_1P_NAME_TEXTURES (SC1PREMIX_1P_MODEL_SCALES + (SC1PREMIX_1P_PATH_COUNT * 4))

static void sc1PRemix1PDataReadStageAt(u32 addr, SC1PGameStage *stage)
{
    stage->screenflash_alpha = remix_ram_image_read_byte(addr + 0x0);
    stage->gkind = remix_ram_image_read_byte(addr + 0x1);
    stage->item_toggles = remix_ram_image_read_word(addr + 0x4);
    stage->opponent_count = remix_ram_image_read_byte(addr + 0x8);
    stage->fkind[0] = remix_ram_image_read_byte(addr + 0x9);
    stage->fkind[1] = remix_ram_image_read_byte(addr + 0xA);
    stage->opponent_behavior = remix_ram_image_read_byte(addr + 0xB);
    stage->ally_count = remix_ram_image_read_byte(addr + 0xC);
    stage->ally_behavior = remix_ram_image_read_byte(addr + 0xD);
}

void sc1PRemix1PDataReadStage(s32 index, SC1PGameStage *stage)
{
    sc1PRemix1PDataReadStageAt(REMIX_SYM_SinglePlayerModes__MATCH_SETTINGS_PART1 + (index * SC1PREMIX_1P_STAGE_STRIDE), stage);
}

void sc1PRemix1PDataReadAllstarStage(s32 index, SC1PGameStage *stage)
{
    sc1PRemix1PDataReadStageAt(REMIX_SYM_SinglePlayerModes__ALLSTAR_MATCH_SETTINGS_PART1 + (index * SC1PREMIX_1P_STAGE_STRIDE), stage);
}

static void sc1PRemix1PDataReadComputerAt(u32 addr, SC1PGameComputer *computer)
{
    s32 i;

    computer->is_team_attack = remix_ram_image_read_byte(addr + 0);
    computer->item_appearance_rate = remix_ram_image_read_byte(addr + 1);

    for (i = 0; i < nSC1PGameDifficultyEnumCount; i++)
    {
        computer->enemy_level[i] = remix_ram_image_read_byte(addr + 2 + i);
        computer->enemy_handicap[i] = remix_ram_image_read_byte(addr + 7 + i);
        computer->ally_level[i] = remix_ram_image_read_byte(addr + 12 + i);
        computer->ally_handicap[i] = remix_ram_image_read_byte(addr + 17 + i);
    }
}

void sc1PRemix1PDataReadComputer(s32 index, SC1PGameComputer *computer)
{
    sc1PRemix1PDataReadComputerAt(REMIX_SYM_SinglePlayerModes__MATCH_SETTINGS_PART2 + (index * SC1PREMIX_1P_COMPUTER_STRIDE), computer);
}

void sc1PRemix1PDataReadAllstarComputer(s32 index, SC1PGameComputer *computer)
{
    sc1PRemix1PDataReadComputerAt(REMIX_SYM_SinglePlayerModes__ALLSTAR_MATCH_SETTINGS_PART2 + (index * SC1PREMIX_1P_COMPUTER_STRIDE), computer);
}

static void sc1PRemix1PDataReadFighterAt(u32 addr, SC1PRemix1PFighter *fighter)
{
    s32 i;

    fighter->fkind = remix_ram_image_read_byte(addr + 0x4);

    for (i = 0; i < SC1PREMIX_1P_STAGE_OPTIONS; i++)
    {
        fighter->stages[i] = remix_ram_image_read_byte(addr + 0x5 + i);
    }
    fighter->name_texture = remix_ram_image_read_word(addr + 0x8);
    fighter->announcer_call = remix_ram_image_read_word(addr + 0xC);
    fighter->model_scale = remix_ram_image_read_word(addr + 0x10);
    fighter->progress_icon = remix_ram_image_read_word(addr + 0x14);
}

void sc1PRemix1PDataReadFighter(s32 index, SC1PRemix1PFighter *fighter)
{
    sc1PRemix1PDataReadFighterAt(REMIX_SYM_SinglePlayerModes__match_pool + (index * SC1PREMIX_1P_POOL_STRIDE), fighter);
}

void sc1PRemix1PDataReadDuo(s32 index, SC1PRemix1PDuo *duo)
{
    u32 addr = REMIX_SYM_SinglePlayerModes__duo_pool + (index * SC1PREMIX_1P_DUO_STRIDE);
    u32 main_addr = remix_ram_image_read_word(addr + 0x0);
    SC1PRemix1PFighter main_fighter;

    sc1PRemix1PDataReadFighterAt(main_addr, &main_fighter);

    duo->main_fkind = main_fighter.fkind;
    duo->main_pool_index = (s32)(main_addr - REMIX_SYM_SinglePlayerModes__match_pool) / SC1PREMIX_1P_POOL_STRIDE;
    duo->main_stages[0] = main_fighter.stages[0];
    duo->main_stages[1] = main_fighter.stages[1];
    duo->main_stages[2] = main_fighter.stages[2];
    duo->partner_fkind = remix_ram_image_read_byte(addr + 0x4);
    duo->loads_first = remix_ram_image_read_byte(addr + 0x5);
    duo->announcer_call = remix_ram_image_read_half(addr + 0x6);
    duo->progress_icon = remix_ram_image_read_word(addr + 0x8);
    duo->name_texture = remix_ram_image_read_word(addr + 0xC);
}

void sc1PRemix1PDataReadSlotCard(s32 index, SC1PRemix1PSlotCard *card)
{
    card->title_fkind = remix_ram_image_read_word(REMIX_SYM_SinglePlayerModes__title_card_1_struct + (index * 4));
    card->name_texture = remix_ram_image_read_word(SC1PREMIX_1P_NAME_TEXTURES + (index * 4));
    card->announcer_call = remix_ram_image_read_word(SC1PREMIX_1P_ANNOUNCER_CALLS + (index * 4));
    card->model_scale = remix_ram_image_read_word(SC1PREMIX_1P_MODEL_SCALES + (index * 4));
    card->progress_icon = remix_ram_image_read_word(REMIX_SYM_SinglePlayerModes__progress_icon_struct + (index * 4));
}

void sc1PRemix1PDataReadKirby(s32 index, s32 *power, s32 *hat)
{
    u32 addr = REMIX_SYM_SinglePlayerModes__remix_kirby_pool + (index * SC1PREMIX_1P_KIRBY_STRIDE);

    *power = remix_ram_image_read_word(addr + 0x0);
    *hat = remix_ram_image_read_word(addr + 0x4);
}

s32 sc1PRemix1PDataFindFighter(s32 fkind)
{
    SC1PRemix1PFighter fighter;
    s32 i;

    for (i = 0; i < SC1PREMIX_1P_POOL_COUNT; i++)
    {
        sc1PRemix1PDataReadFighter(i, &fighter);

        if (fighter.fkind == fkind)
        {
            return i;
        }
    }
    return -1;
}

s32 sc1PRemix1PDataRandomSlot(s32 pick)
{
    u32 first_slot = REMIX_SYM_SinglePlayerModes__match_slots - (SC1PREMIX_1P_RANDOM_SLOTS - 1);

    if ((pick < 0) || (pick >= SC1PREMIX_1P_RANDOM_SLOTS))
    {
        return -1;
    }
    return remix_ram_image_read_byte(first_slot + pick) / SC1PREMIX_1P_STAGE_STRIDE;
}

#endif
