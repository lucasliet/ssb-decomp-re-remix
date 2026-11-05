#ifndef _ITRENDER_H_
#define _ITRENDER_H_

#include "ittypes.h"

// Display item hitboxes and hurtboxes
void itDisplayHitCollisions(GObj* item_gobj);

#ifdef REGION_REMIX
// Model routine drawn under the hitboxes by Hitbox.asm render_hitbox_plus_item_ (hitbox+model)
typedef enum ITDisplayRemixModel
{
    nITDisplayRemixModelOPA,                // gcDrawDObjTreeForGObj, itDisplayOPAProcDisplay (RAM 0x80171D20)
    nITDisplayRemixModelXLU,                // gcDrawDObjTreeDLLinksForGObj, itDisplayXLUProcDisplay (RAM 0x80171DDC)
    nITDisplayRemixModelColAnimOPA,         // itDisplayColAnimOPA (RAM 0x80171FF0)
    nITDisplayRemixModelColAnimXLU,         // itDisplayColAnimXLU (RAM 0x801722F0)
    nITDisplayRemixModelMonsterTexEdge,     // gcDrawDObjTreeForGObj after the tex edge render mode (RAM 0x8018346C, 0x8017E5CC)
    nITDisplayRemixModelMonsterXLU          // gcDrawDObjTreeForGObj after the XLU render mode (RAM 0x801835C8, 0x8017E228)

} ITDisplayRemixModel;

// Display item hitboxes and hurtboxes, with the model under them in hitbox+model
void itDisplayRemixHitCollisions(GObj* item_gobj, ITDisplayRemixModel model);
#endif

// Display map collision diamond
void itDisplayMapCollisions(GObj* item_gobj);

// Check whether item should be rendered
sb32 itDisplayCheckItemVisible(ITStruct* ip);

// Render item with opacity flag
void itDisplayOPAProcDisplay(GObj* item_gobj);

// Render item with transparency flag
void itDisplayXLUProcDisplay(GObj* item_gobj);

// Display item's ColAnim with opacity flag
void itDisplayColAnimOPA(GObj* item_gobj);

// Render item with ColAnim and opacity flag
void itDisplayColAnimOPAProcDisplay(GObj* item_gobj);

// Display item's ColAnim with transparency flag
void itDisplayColAnimXLU(GObj* item_gobj);

// Render item's ColAnim with transparency flag
void itDisplayColAnimXLUProcDisplay(GObj* item_gobj);

#endif
