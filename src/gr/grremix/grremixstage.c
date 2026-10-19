/**
 * grremixstage.c - Remix stage tables and ground dispatch. See grremixstage.h.
 */

#ifdef PORT

#include "grremixstage.h"
#include "grremixhazards.h"
#include "grremixrandommusic.h"
#include <gr/ground.h>
#include <sc/scene.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include <port_log.h>
#include <sys/utils.h>

#define GRREMIX_ALT_MUSIC_CHANCE_MAIN 65
#define GRREMIX_ALT_MUSIC_CHANCE_OCCASIONAL 16
#define GRREMIX_ALT_MUSIC_CHANCE_RARE 10
#define GRREMIX_ALT_MUSIC_NONE 0xFFFF

/* GameEnd.asm is_salty_runback in the Remix RAM image; missing from remix_symbols.h, so the address comes from remix.sym. */
#define GRREMIX_ADDR_SALTY_RUNBACK 0x8048B174

#define GRREMIX_FUNC_PEACHS_CASTLE 0x8010B4AC
#define GRREMIX_FUNC_SECTOR_Z 0x80107FCC
#define GRREMIX_FUNC_CONGO_JUNGLE 0x80109FB4
#define GRREMIX_FUNC_PLANET_ZEBES 0x80108448
#define GRREMIX_FUNC_HYRULE_CASTLE 0x8010AB20
#define GRREMIX_FUNC_YOSHIS_ISLAND 0x80108C80
#define GRREMIX_FUNC_DREAM_LAND 0x801066D4
#define GRREMIX_FUNC_SAFFRON_CITY 0x8010B2EC
#define GRREMIX_FUNC_MUSHROOM_KINGDOM 0x80109C0C

/* Words of the image are native u32; a 4-aligned read is direct. */
static u32 grRemixReadWord(u32 addr)
{
    u32 *word = remix_ram_image_resolve(addr);

    if (word == NULL)
    {
        return 0;
    }
    return *word;
}

static u32 grRemixReadHalf(u32 addr)
{
    u32 word = grRemixReadWord(addr & ~3u);

    return (word >> ((2 - (addr & 2)) * 8)) & 0xFFFF;
}

static u32 grRemixReadByte(u32 addr)
{
    u32 word = grRemixReadWord(addr & ~3u);

    return (word >> ((3 - (addr & 3)) * 8)) & 0xFF;
}

sb32 grRemixIsCustomKind(s32 gkind)
{
    return (gkind >= GRREMIX_KIND_START) && (gkind <= GRREMIX_KIND_MAX);
}

GRFileInfo grRemixGetGroundFileInfo(s32 gkind)
{
    GRFileInfo info;
    u32 entry = REMIX_SYM_Stages__stage_file_table + (gkind * 8);

    info.file_id = grRemixReadWord(entry);
    info.offset = grRemixReadWord(entry + 4);

    return info;
}

u32 grRemixGetGroundFunction(s32 gkind)
{
    return grRemixReadWord(REMIX_SYM_Stages__function_table + (gkind * 4));
}

s32 grRemixGetStageClass(s32 gkind)
{
    return grRemixReadByte(REMIX_SYM_Stages__class_table + gkind);
}

s32 grRemixGetDefaultMusicTrack(s32 gkind)
{
    return grRemixReadHalf(REMIX_SYM_Stages__default_music_track_table + (gkind * 2));
}

s32 grRemixGetBackgroundStage(s32 gkind)
{
    return grRemixReadByte(REMIX_SYM_Stages__background_table + gkind);
}

u32 grRemixGetStageIconOffset(s32 gkind)
{
    return grRemixReadWord(REMIX_SYM_Stages__icon_offset_table + (gkind * 4)) & 0x00FFFFFF;
}

void grRemixGetStageName(s32 gkind, char *out_name)
{
    u32 text = grRemixReadWord(REMIX_SYM_Stages__string_table + (gkind * 4));
    s32 i;
    s32 count = 0;

    for (i = 0; (text != 0) && (i < 255) && (count < (GRREMIX_NAME_MAX - 1)); i++)
    {
        s32 c = grRemixReadByte(text + i);

        if (c == 0)
        {
            break;
        }
        if ((c >= 'a') && (c <= 'z'))
        {
            c -= 'a' - 'A';
        }
        if (((c >= 'A') && (c <= 'Z')) || (c == '\'') || (c == '.') || (c == '%'))
        {
            out_name[count++] = c;
        }
        else if (c == ' ')
        {
            out_name[count++] = ' ';
        }
    }
    out_name[count] = '\0';
}

f32 grRemixGetPreviewZoom(s32 gkind)
{
    union
    {
        u32 word;
        f32 value;

    } zoom;

    zoom.word = grRemixReadWord(REMIX_SYM_Stages__zoom_table + (gkind * 4));

    return (zoom.value > 0.0F) ? zoom.value : 0.5F;
}

sb32 grRemixGetPreviewPosition(s32 gkind, f32 *out_x, f32 *out_y, f32 *out_z)
{
    u32 array = grRemixReadWord(REMIX_SYM_Stages__position_table + (gkind * 4));
    union
    {
        u32 word;
        f32 value;

    } x, y, z;

    if (array == 0)
    {
        return FALSE;
    }
    x.word = grRemixReadWord(array);
    y.word = grRemixReadWord(array + 4);
    z.word = grRemixReadWord(array + 8);

    *out_x = x.value;
    *out_y = y.value;
    *out_z = z.value;

    return TRUE;
}

s32 grRemixSelectGetStages(s32 *out_gkinds, s32 max_count)
{
    s32 index;
    s32 count = 0;

    for (index = 0; (index < GRREMIX_SELECT_ENTRIES_MAX) && (count < max_count); index++)
    {
        s32 gkind = grRemixReadByte(REMIX_SYM_Stages__stage_table_normal + index);

        if (gkind <= GRREMIX_KIND_MAX)
        {
            out_gkinds[count++] = gkind;
        }
    }
    return count;
}

static u32 grRemixGetAlternateStageBGM(s32 gkind, u32 default_bgm)
{
    s32 override_track = grRemixGetDefaultMusicTrack(gkind);
    s32 roll;
    s32 slot;
    u32 alternate;

    if (override_track != 0)
    {
        default_bgm = override_track - 1;
    }
    roll = syUtilsRandIntRange(100);

    if (roll < GRREMIX_ALT_MUSIC_CHANCE_MAIN)
    {
        return default_bgm;
    }
    if (roll < (GRREMIX_ALT_MUSIC_CHANCE_MAIN + GRREMIX_ALT_MUSIC_CHANCE_OCCASIONAL))
    {
        slot = 0;
    }
    else if (roll < (GRREMIX_ALT_MUSIC_CHANCE_MAIN + GRREMIX_ALT_MUSIC_CHANCE_OCCASIONAL + GRREMIX_ALT_MUSIC_CHANCE_RARE))
    {
        slot = 1;
    }
    else
    {
        slot = 2;
    }
    alternate = grRemixReadHalf(REMIX_SYM_Stages__alternate_music_table + (gkind * 8) + (slot * 2));

    return (alternate == GRREMIX_ALT_MUSIC_NONE) ? default_bgm : alternate;
}

u32 grRemixGetStageBGM(s32 gkind, u32 default_bgm)
{
    sb32 salty_runback = (grRemixReadWord(GRREMIX_ADDR_SALTY_RUNBACK) != 0);

    return grRemixRandomMusicPick(grRemixGetAlternateStageBGM(gkind, default_bgm), salty_runback);
}

void grRemixMakeGround(void)
{
    s32 gkind = gSCManagerBattleState->gkind;
    u32 function = grRemixGetGroundFunction(gkind);

    switch (function)
    {
    case GRREMIX_FUNC_PEACHS_CASTLE:
        grCastleMakeGround();
        break;

    case GRREMIX_FUNC_SECTOR_Z:
        grSectorMakeGround();
        break;

    case GRREMIX_FUNC_CONGO_JUNGLE:
        grJungleMakeGround();
        break;

    case GRREMIX_FUNC_PLANET_ZEBES:
        grZebesMakeGround();
        break;

    case GRREMIX_FUNC_HYRULE_CASTLE:
        grHyruleMakeGround();
        break;

    case GRREMIX_FUNC_YOSHIS_ISLAND:
        grYosterMakeGround();
        break;

    case GRREMIX_FUNC_DREAM_LAND:
        grPupupuMakeGround();
        break;

    case GRREMIX_FUNC_SAFFRON_CITY:
        grYamabukiMakeGround();
        break;

    case GRREMIX_FUNC_MUSHROOM_KINGDOM:
        grInishieMakeGround();
        break;

    case GRREMIX_FUNC_CLONE:
        break;

    default:
        if (grRemixHazardsMakeGround(gkind, function) == FALSE)
        {
            port_log("[remix-stage] gkind=0x%02X hazard routine 0x%08X is not ported", gkind, function);
        }
        break;
    }
}

#endif
