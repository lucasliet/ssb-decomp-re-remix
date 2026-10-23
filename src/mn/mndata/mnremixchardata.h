#ifndef _MNREMIXCHARDATA_H_
#define _MNREMIXCHARDATA_H_

#include <ssb_types.h>

#ifdef REGION_REMIX

#define MNREMIX_CHARDATA_PAGE_COUNT 30
#define MNREMIX_CHARDATA_BIOS_FILE_ID 0x10F5
#define MNREMIX_CHARDATA_IMAGES_FILE_ID 0x10F6
#define MNREMIX_CHARDATA_BIOS_FILE_INDEX 4
#define MNREMIX_CHARDATA_IMAGES_FILE_INDEX 5

enum MNRemixCharDataSpriteKind
{
    MNREMIX_CHARDATA_SPRITE_BIO,
    MNREMIX_CHARDATA_SPRITE_WORKS,
    MNREMIX_CHARDATA_SPRITE_NAME
};

typedef struct MNRemixCharDataSpriteRef
{
    s32 file_index;
    intptr_t offset;
} MNRemixCharDataSpriteRef;

extern s32 mnRemixCharDataGetFighterKind(s32 page);
extern s32 mnRemixCharDataGetPage(s32 fkind);
extern sb32 mnRemixCharDataIsExtended(s32 fkind);
extern sb32 mnRemixCharDataGetSpriteRef(s32 fkind, s32 sprite_kind, MNRemixCharDataSpriteRef *ref);
extern f32 mnRemixCharDataGetNameX(s32 fkind);
extern f32 mnRemixCharDataGetNameY(s32 fkind);

#endif

#endif
