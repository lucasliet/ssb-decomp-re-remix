/**
 * sc1premixallstarrest.c - series icons of the All-Star Rest Area, see sc1premixallstarrest.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <lb/library.h>
#include <port_log.h>
#include <sys/taskman.h>
#include "gm/gmremixtoggle.h"
#include "if/ifremixsprite.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "sc/sc1pmode/sc1premix1prules.h"
#include "sc/sc1pmode/sc1premixallstar.h"
#include "sc/sc1pmode/sc1premixallstarrest.h"
#include "sc/sc1pmode/sc1premixallstarrestrules.h"

/* Render.draw_texture_ args of the asm: room 0x18, group 0, white tint, no palette, scale 1. */
#define SC1PREMIX_ALLSTAR_REST_ICON_FILE 0xD8B
#define SC1PREMIX_ALLSTAR_REST_ICON_GROUP 0x0B
#define SC1PREMIX_ALLSTAR_REST_ICON_DL_LINK 23
#define SC1PREMIX_ALLSTAR_REST_ICON_TINT 0xFFFFFFFF
#define SC1PREMIX_ALLSTAR_REST_ICON_PALETTE 0x00000000
#define SC1PREMIX_ALLSTAR_REST_ICON_SCALE 1.0F

static void sc1PRemixAllstarRestMakeIcon(void *icon_file, s32 index, s32 fkind)
{
    u32 sprite_offset = remix_ram_image_read_word(REMIX_SYM_SinglePlayerModes__icon_offset_table + (fkind * sizeof(u32)));
    f32 x;
    f32 y;

    sc1PRemixAllstarRestIconPosition(index, gmRemixToggle(dragon_king_hud), &x, &y);

    ifRemixSpriteMakeFromFile(icon_file, sprite_offset, SC1PREMIX_ALLSTAR_REST_ICON_GROUP, SC1PREMIX_ALLSTAR_REST_ICON_DL_LINK, x, y,
                              SC1PREMIX_ALLSTAR_REST_ICON_TINT, SC1PREMIX_ALLSTAR_REST_ICON_PALETTE, SC1PREMIX_ALLSTAR_REST_ICON_SCALE);
}

void sc1PRemixAllstarRestMakeIcons(void)
{
    s32 progress = sc1PRemixAllstarGetProgress();
    s32 count = sc1PRemixAllstarRestIconCount(SC1PREMIX_ALLSTAR_FIGHTER_COUNT, progress);
    void *icon_file;
    s32 index;

    if ((sc1PRemixAllstarIsRestArea() == FALSE) || (count == 0))
    {
        return;
    }
    icon_file = lbRelocGetExternHeapFile(SC1PREMIX_ALLSTAR_REST_ICON_FILE, syTaskmanMalloc(lbRelocGetFileSize(SC1PREMIX_ALLSTAR_REST_ICON_FILE), 0x10));

    for (index = 0; index < count; index++)
    {
        sc1PRemixAllstarRestMakeIcon(icon_file, index, sc1PRemixAllstarGetFkind(progress + index));
    }
}

#endif
