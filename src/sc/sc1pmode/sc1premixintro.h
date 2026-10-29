#ifndef _SC1PREMIXINTRO_H_
#define _SC1PREMIXINTRO_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Per-fighter data of the 1P title card (SinglePlayer.asm get_name_texture_, get_fgm_, get_name_delay_ and the singleplayer_vs_preview
 * fixes), read from the Remix RAM image. The vanilla card indexes 12-entry tables by fighter kind; these cover every Remix fighter.
 */

/** Fighter kind (0 to 11) whose camera and pose the card of this fighter borrows (Character.singleplayer_vs_preview). */
extern s32 sc1PRemixIntroPreviewKind(s32 fkind);

/** Offset of the fighter name sprite in its file (name_texture_table, already past the file header). */
extern intptr_t sc1PRemixIntroNameOffset(s32 fkind);

/** TRUE when the name sprite of this fighter is in the card file (Metal Mario, Giant DK, Master Hand), FALSE for the names file. */
extern sb32 sc1PRemixIntroNameIsInCardFile(s32 fkind);

/** Announcer call (FGM id) that says the name of the fighter. */
extern u32 sc1PRemixIntroAnnounceCall(s32 fkind);

/** Tics the announcer waits between the fighter name and "VS". */
extern u32 sc1PRemixIntroNameDelay(s32 fkind);

#endif

#endif
