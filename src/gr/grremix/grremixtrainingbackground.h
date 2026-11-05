#ifndef _GRREMIXTRAININGBACKGROUND_H_
#define _GRREMIXTRAININGBACKGROUND_H_

/*
 * Training wallpaper stage of the Remix build (Stages.asm training_id_fix_ and random_fix_3_). In Training Mode the
 * wallpaper tables are indexed with the parent stage of Stages.background_table instead of the stage id, in the match
 * (sc1ptrainingmode.c) and in the stage select preview (mnmaps.c).
 */

#include <ssb_types.h>

/**
 * Index for the Training wallpaper, heap offset and fog tables of a stage.
 *
 * @param gkind Stage id.
 * @return Stages.background_table entry of the stage, or gkind itself when it is outside the Remix stage ids (the RANDOM
 * cell).
 */
s32 grRemixTrainingBackgroundGkind(s32 gkind);

#endif
