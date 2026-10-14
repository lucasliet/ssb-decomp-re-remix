#ifndef _GRREMIXSTAGE_H_
#define _GRREMIXSTAGE_H_

/*
 * Smash Remix stage tables (.toolchain/pkgs/smashremix-src/src/Stages.asm).
 *
 * Remix keeps every per-stage attribute in a table indexed by stage id and
 * grown past the vanilla 0x00..0x28 range up to GRREMIX_KIND_MAX. The tables
 * live in the Remix RAM image; these readers expose them to the port. The
 * port's gkind is the Remix stage id.
 */

#include <ssb_types.h>
#include <gr/grtypes.h>

#define GRREMIX_KIND_START 0x29 /* first custom stage (Deku Tree) */
#define GRREMIX_KIND_MAX 0xD7 /* Stages.MAX_STAGE_ID */

/* Stages.function_table entries that are not a hazard routine of the Remix build. */
#define GRREMIX_FUNC_CLONE 0x801056F8 /* Stages.function.CLONE: a "jr ra"; the stage has no hazard code */

typedef enum GRRemixStageClass
{
    nGRRemixStageClassBattle = 0,
    nGRRemixStageClassRTTF = 1,
    nGRRemixStageClassBTP = 2,
    nGRRemixStageClassBTT = 3

} GRRemixStageClass;

/** TRUE when gkind is a Remix-only stage id (0x29..GRREMIX_KIND_MAX). */
sb32 grRemixIsCustomKind(s32 gkind);

/** Map file id and header offset of the stage (Stages.stage_file_table, 8 bytes per stage). */
GRFileInfo grRemixGetGroundFileInfo(s32 gkind);

/** Stages.function_table entry: a vanilla ground maker address, GRREMIX_FUNC_CLONE or a Remix hazard routine. */
u32 grRemixGetGroundFunction(s32 gkind);

/** Stages.class_table entry (GRRemixStageClass). */
s32 grRemixGetStageClass(s32 gkind);

/** Stages.default_music_track_table entry (track + 1; 0 = none). */
s32 grRemixGetDefaultMusicTrack(s32 gkind);

/** Stages.background_table entry: the stage id whose background this stage shows. */
s32 grRemixGetBackgroundStage(s32 gkind);

/**
 * BGM id the stage plays (BGM.asm alternate_music_): the Stages.default_music_track_table override of default_bgm, then
 * the stage's Occasional/Rare/Rare2 alternates by chance (65/16/10/9 percent).
 */
u32 grRemixGetStageBGM(s32 gkind, u32 default_bgm);

#define GRREMIX_NAME_MAX 48

/** Offset of the stage icon Sprite inside the Remix stage icon file (Stages.icon_offset_table; every icon is in file index 0). */
u32 grRemixGetStageIconOffset(s32 gkind);

/** Display name of the stage (Stages.string_table), upper-cased and reduced to the CSS font charset (A-Z ' . % space). */
void grRemixGetStageName(s32 gkind, char *out_name);

/** Preview model scale (Stages.zoom_table). */
f32 grRemixGetPreviewZoom(s32 gkind);

/** Preview model translation (Stages.position_table); FALSE when the stage has no entry. */
sb32 grRemixGetPreviewPosition(s32 gkind, f32 *out_x, f32 *out_y, f32 *out_z);

#define GRREMIX_SELECT_ENTRIES_MAX 90 /* Stages.NUM_ICONS * Stages.NUM_PAGES of stage_table_normal */

/**
 * Stages of the VS stage select in Remix order (Stages.stage_table_normal, the Random slots dropped).
 * Writes at most max_count ids to out_gkinds and returns how many were written.
 */
s32 grRemixSelectGetStages(s32 *out_gkinds, s32 max_count);

/** Runs the ground maker of the stage: the vanilla maker for the shared ones, nothing for clones and unported hazards. */
void grRemixMakeGround(void);

#endif
