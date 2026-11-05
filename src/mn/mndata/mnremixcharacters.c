/**
 * mnremixcharacters.c - Remix data read by the Character Data screen (src/CharacterDataScreen.asm,
 * src/ResultsScreen.asm) for the 18 extended fighters.
 *
 * Everything is read at run time from the Remix RAM image (port/remix/remix_ram_image.h): the emblem offsets
 * (ResultsScreen.series_logo_*_table), the special and jab action tables (special_action_pointers,
 * jab_action_pointers) and the special-move name sprites (special_attack_offsets). Action rows are 3 words:
 * 0x20000 + action id (0xA2C2A is the "random" filler), frames (0x29A waits for the animation end) and the
 * ftMainSetStatus flags. Action ids are port statuses (docs/remix/fighters.md).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <stdlib.h>
#include <ssb_types.h>
#include <ft/ftdef.h>
#include <mn/menu.h>
#include <port_log.h>
#include "fighter_registry.h"
#include "mn/mndata/mnremixcharacters.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define MNREMIX_CHARACTERS_WORD_SIZE 4
#define MNREMIX_CHARACTERS_MOTION_ROW_SIZE 0xC
#define MNREMIX_CHARACTERS_TRACKS_NUM 8
#define MNREMIX_CHARACTERS_SPECIAL_KINDS_NUM 3
#define MNREMIX_CHARACTERS_SPECIAL_NAME_ENTRY_SIZE (MNREMIX_CHARACTERS_SPECIAL_KINDS_NUM * MNREMIX_CHARACTERS_WORD_SIZE)
#define MNREMIX_CHARACTERS_VANILLA_FIGHTERS_NUM 12
#define MNREMIX_CHARACTERS_VANILLA_SPECIAL_TABLES 0x801340B0U
#define MNREMIX_CHARACTERS_VANILLA_SPECIAL_STRIDE 0x120U
#define MNREMIX_CHARACTERS_VANILLA_JAB_TABLES 0x80135DA8U
#define MNREMIX_CHARACTERS_VANILLA_JAB_STRIDE 0x60U
#define MNREMIX_CHARACTERS_OFFSET_FLAG_MASK 0xFF000000U
#define MNREMIX_CHARACTERS_FORCED_MOTIONS_ENV "SSB64_REMIX_CHARDATA_MOTIONS"
#define MNREMIX_CHARACTERS_FORCED_MOTIONS_MAX 64

static s32 sMNRemixCharactersForcedMotionKinds[MNREMIX_CHARACTERS_FORCED_MOTIONS_MAX];
static s32 sMNRemixCharactersForcedMotionsNum;
static s32 sMNRemixCharactersForcedMotionNext;
static sb32 sMNRemixCharactersIsForcedMotionsParsed = FALSE;

/** TRUE for the Remix fighters whose name uses the tall tag (extend_name_border_check: Dr. Mario, Ganondorf, Banjo, Young Link). */
sb32 mnRemixCharactersHasTallNameTag(s32 fkind)
{
    return (fkind == REMIX_ID_DRM) || (fkind == REMIX_ID_GND) || (fkind == REMIX_ID_BANJO) || (fkind == REMIX_ID_YLINK);
}

static u32 mnRemixCharactersReadTableWord(u32 table, s32 index)
{
    return remix_ram_image_read_word(table + (index * MNREMIX_CHARACTERS_WORD_SIZE));
}

/**
 * Offsets of the series emblem models of `fkind` in the emblem models file. The asm of the Data screen swaps only the
 * DObjDesc offset (ResultsScreen.series_logo_offset_table) and leaves the MObjSub and MatAnimJoint offsets at 0 for the
 * 18 fighters; the port reads the whole triple of the results screen (series_logo_zoom_table is the MObjSub offset,
 * series_logo_color_table the MatAnimJoint offset) so the model gets the textures and the colour animation of its series.
 * Returns FALSE when the fighter has no emblem.
 */
sb32 mnRemixCharactersGetEmblem(s32 fkind, MNRemixCharactersEmblem *emblem)
{
    u32 dobjdesc = mnRemixCharactersReadTableWord(REMIX_SYM_ResultsScreen__series_logo_offset_table, fkind);
    u32 mobjsub = mnRemixCharactersReadTableWord(REMIX_SYM_ResultsScreen__series_logo_zoom_table, fkind);
    u32 matanim = mnRemixCharactersReadTableWord(REMIX_SYM_ResultsScreen__series_logo_color_table, fkind);

    if (dobjdesc == 0)
    {
        return FALSE;
    }
    if ((dobjdesc | mobjsub | matanim) & MNREMIX_CHARACTERS_OFFSET_FLAG_MASK)
    {
        port_log("[mnremixcharacters] fkind %d emblem offsets 0x%08X 0x%08X 0x%08X name another file; the emblem is skipped\n",
                 (int)fkind, dobjdesc, mobjsub, matanim);
        return FALSE;
    }
    emblem->dobjdesc = dobjdesc;
    emblem->mobjsub = mobjsub;
    emblem->matanim = matanim;

    return TRUE;
}

/** Writes the "random" filler row (status 0xA2C2A, 1 frame) that ends an action sequence. */
void mnRemixCharactersMakeNullMotion(MNCharactersMotion *motion)
{
    motion->status_id = FTSTATUS_CHARACTERS_NULL;
    motion->anim_length = 1;
    motion->flags = FTSTATUS_PRESERVE_NONE;
}

static sb32 mnRemixCharactersReadMotionRow(u32 table, s32 row, MNCharactersMotion *motion)
{
    u32 row_addr = table + (row * MNREMIX_CHARACTERS_MOTION_ROW_SIZE);
    u32 status_id = remix_ram_image_read_word(row_addr);

    if (status_id == 0)
    {
        return FALSE;
    }
    motion->status_id = status_id;
    motion->anim_length = remix_ram_image_read_word(row_addr + MNREMIX_CHARACTERS_WORD_SIZE);
    motion->flags = remix_ram_image_read_word(row_addr + (MNREMIX_CHARACTERS_WORD_SIZE * 2));

    return TRUE;
}

/** Index of the vanilla fighter whose table sits at `table` (tables of the original overlay), or -1 for a table of the image. */
static s32 mnRemixCharactersGetVanillaTableIndex(u32 table, u32 first_table, u32 stride)
{
    if ((table < first_table) || (table >= (first_table + (MNREMIX_CHARACTERS_VANILLA_FIGHTERS_NUM * stride))))
    {
        return -1;
    }
    return (table - first_table) / stride;
}

/**
 * Row `track` of the special move `motion_kind` (SpecialHi, SpecialN, SpecialLw) of `fkind`. Five fighters point at a
 * table of the vanilla overlay (Falco at Fox's, Ganondorf at Falcon's...): then `*vanilla_fkind` is that fighter and
 * `motion` is left untouched, the caller reads its own copy of the vanilla table. Returns FALSE for no row.
 */
sb32 mnRemixCharactersGetSpecialMotion(MNCharactersMotion *motion, s32 fkind, s32 motion_kind, s32 track, s32 *vanilla_fkind)
{
    u32 table;

    *vanilla_fkind = -1;

    if ((motion_kind < 0) || (motion_kind >= MNREMIX_CHARACTERS_SPECIAL_KINDS_NUM) || (track < 0) || (track >= MNREMIX_CHARACTERS_TRACKS_NUM))
    {
        return FALSE;
    }
    table = mnRemixCharactersReadTableWord(REMIX_SYM_CharacterDataScreen__special_action_pointers, fkind);

    if (table == 0)
    {
        return FALSE;
    }
    *vanilla_fkind = mnRemixCharactersGetVanillaTableIndex(table, MNREMIX_CHARACTERS_VANILLA_SPECIAL_TABLES, MNREMIX_CHARACTERS_VANILLA_SPECIAL_STRIDE);

    if (*vanilla_fkind >= 0)
    {
        return TRUE;
    }
    return mnRemixCharactersReadMotionRow(table, (motion_kind * MNREMIX_CHARACTERS_TRACKS_NUM) + track, motion);
}

/** Row `track` of the jab combo of `fkind` (jab_action_pointers); `vanilla_fkind` works as in mnRemixCharactersGetSpecialMotion. */
sb32 mnRemixCharactersGetJabMotion(MNCharactersMotion *motion, s32 fkind, s32 track, s32 *vanilla_fkind)
{
    u32 table;

    *vanilla_fkind = -1;

    if ((track < 0) || (track >= MNREMIX_CHARACTERS_TRACKS_NUM))
    {
        return FALSE;
    }
    table = mnRemixCharactersReadTableWord(REMIX_SYM_CharacterDataScreen__jab_action_pointers, fkind);

    if (table == 0)
    {
        return FALSE;
    }
    *vanilla_fkind = mnRemixCharactersGetVanillaTableIndex(table, MNREMIX_CHARACTERS_VANILLA_JAB_TABLES, MNREMIX_CHARACTERS_VANILLA_JAB_STRIDE);

    if (*vanilla_fkind >= 0)
    {
        return TRUE;
    }
    return mnRemixCharactersReadMotionRow(table, track, motion);
}

/**
 * File and offset of the name sprite of the special move `motion_kind` of `fkind` (special_attack_offsets: 3 words per
 * fighter, USP, NSP, DSP, with the top-byte flag of the extended images file 0x10F6). Returns FALSE when there is none.
 */
sb32 mnRemixCharactersGetSpecialNameRef(s32 fkind, s32 motion_kind, MNRemixCharDataSpriteRef *ref)
{
    u32 entry = REMIX_SYM_CharacterDataScreen__special_attack_offsets + (fkind * MNREMIX_CHARACTERS_SPECIAL_NAME_ENTRY_SIZE);
    u32 value;

    if ((motion_kind < 0) || (motion_kind >= MNREMIX_CHARACTERS_SPECIAL_KINDS_NUM))
    {
        return FALSE;
    }
    value = mnRemixCharactersReadTableWord(entry, motion_kind);

    if (value == 0)
    {
        return FALSE;
    }
    mnRemixCharDataMakeRef(value, 0, MNREMIX_CHARDATA_IMAGES_FILE_INDEX, ref);

    return TRUE;
}

/**
 * TRUE when ftMainSetStatus can run the status of `motion` on the synth fighter `fkind`: a special status needs an entry
 * in the fighter's action array and an opening status needs the opening table of the fighter. The "random" filler is
 * not a status and is never playable.
 */
sb32 mnRemixCharactersIsMotionPlayable(s32 fkind, const MNCharactersMotion *motion)
{
    s32 status_id = motion->status_id;

    if (status_id == FTSTATUS_CHARACTERS_NULL)
    {
        return FALSE;
    }
    if (status_id >= FTSTAT_CHARDATA_START)
    {
        status_id -= FTSTAT_CHARDATA_START;
    }
    if (status_id >= FTSTAT_OPENING1_START)
    {
        return (port_fighter_opening_descs(fkind) != NULL) ? TRUE : FALSE;
    }
    if (status_id >= FTSTAT_OPENING2_START)
    {
        return TRUE;
    }
    if (status_id >= nFTCommonStatusSpecialStart)
    {
        return ((status_id - nFTCommonStatusSpecialStart) < port_fighter_special_descs_count(fkind)) ? TRUE : FALSE;
    }
    return TRUE;
}

static void mnRemixCharactersParseForcedMotions(void)
{
    const char *list = getenv(MNREMIX_CHARACTERS_FORCED_MOTIONS_ENV);
    s32 value = 0;
    sb32 has_digits = FALSE;

    sMNRemixCharactersIsForcedMotionsParsed = TRUE;

    if (list == NULL)
    {
        return;
    }
    do
    {
        if ((*list >= '0') && (*list <= '9'))
        {
            if (value < nMNCharactersMotionKindEnumCount)
            {
                value = (value * 10) + (*list - '0');
            }
            has_digits = TRUE;
        }
        else if ((*list == ',') || (*list == '\0'))
        {
            if ((has_digits != FALSE) && (value < nMNCharactersMotionKindEnumCount) && (sMNRemixCharactersForcedMotionsNum < MNREMIX_CHARACTERS_FORCED_MOTIONS_MAX))
            {
                sMNRemixCharactersForcedMotionKinds[sMNRemixCharactersForcedMotionsNum++] = value;
            }
            else if (has_digits != FALSE)
            {
                port_log("[mnremixcharacters] %s: motion kind %d is out of range 0..%d or the list is full; ignored\n",
                         MNREMIX_CHARACTERS_FORCED_MOTIONS_ENV, (int)value, (int)(nMNCharactersMotionKindEnumCount - 1));
            }
            value = 0;
            has_digits = FALSE;
        }
    }
    while (*list++ != '\0');
}

/**
 * Test override: SSB64_REMIX_CHARDATA_MOTIONS=<k1,k2,...> (decimal MNCharactersMotionKind values) replaces the random
 * motion pick of the Character Data screen with the list, cycling. Writes the next kind and returns TRUE while the
 * variable holds at least one valid kind; FALSE (the random pick runs) otherwise.
 */
sb32 mnRemixCharactersNextForcedMotionKind(s32 *motion_kind)
{
    if (sMNRemixCharactersIsForcedMotionsParsed == FALSE)
    {
        mnRemixCharactersParseForcedMotions();
    }
    if (sMNRemixCharactersForcedMotionsNum == 0)
    {
        return FALSE;
    }
    *motion_kind = sMNRemixCharactersForcedMotionKinds[sMNRemixCharactersForcedMotionNext];
    sMNRemixCharactersForcedMotionNext = (sMNRemixCharactersForcedMotionNext + 1) % sMNRemixCharactersForcedMotionsNum;

    return TRUE;
}

#endif
