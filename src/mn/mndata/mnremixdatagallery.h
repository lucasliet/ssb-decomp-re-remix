#ifndef _MNREMIXDATAGALLERY_H_
#define _MNREMIXDATAGALLERY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

/*
 * Gallery button of the Data screen (Gallery.asm data_menu.add_button_, cursor_): a fourth tab under Sound Test whose text image is
 * the Gallery label that Remix appended to the Data menu file.
 */

/** Cursor index of the Gallery button (Gallery.asm `lli t1, 0x0003`), after Characters, VS Record and Sound Test. */
#define MNREMIX_DATA_OPTION_GALLERY 3

/**
 * Creates the Gallery tab and its text.
 *
 * @param data_file Data menu file (the Sound Test text sprite the label reuses and the Gallery label), sMNDataFiles[1].
 * @param is_highlight TRUE when the cursor starts on the button.
 * @return The button object.
 */
extern GObj* mnRemixDataGalleryMakeButton(void *data_file, sb32 is_highlight);

#endif

#endif
