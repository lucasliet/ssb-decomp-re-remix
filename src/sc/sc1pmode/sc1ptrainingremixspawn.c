/**
 * sc1ptrainingremixspawn.c - custom spawn of the Training Z menu, see sc1ptrainingremixspawn.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftdef.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "sc/sc1pmode/sc1ptrainingremixspawn.h"

#define SC1PTRAININGREMIXSPAWN_PORT_COUNT 4
#define SC1PTRAININGREMIXSPAWN_NO_LEDGE (-1)

typedef struct SC1PTrainingRemixSpawn
{
    sb32 is_custom;
    f32 x;
    f32 y;
    s32 lr;
    s32 cliff_id;

} SC1PTrainingRemixSpawn;

/* Training.asm struct.port_N defaults: position 0,0, facing left (FACE_LEFT), clipping_id -1. */
static SC1PTrainingRemixSpawn sSC1PTrainingRemixSpawns[SC1PTRAININGREMIXSPAWN_PORT_COUNT] =
{
    { FALSE, 0.0F, 0.0F, -1, SC1PTRAININGREMIXSPAWN_NO_LEDGE },
    { FALSE, 0.0F, 0.0F, -1, SC1PTRAININGREMIXSPAWN_NO_LEDGE },
    { FALSE, 0.0F, 0.0F, -1, SC1PTRAININGREMIXSPAWN_NO_LEDGE },
    { FALSE, 0.0F, 0.0F, -1, SC1PTRAININGREMIXSPAWN_NO_LEDGE }
};

static sb32 sc1PTrainingRemixSpawnIsValidPort(s32 port)
{
    return ((port >= 0) && (port < SC1PTRAININGREMIXSPAWN_PORT_COUNT)) ? TRUE : FALSE;
}

void sc1PTrainingRemixSpawnSave(s32 port)
{
    SC1PTrainingRemixSpawn *spawn;
    GObj *fighter_gobj;
    FTStruct *fp;
    DObj *top_joint;

    if (sc1PTrainingRemixSpawnIsValidPort(port) == FALSE)
    {
        return;
    }
    fighter_gobj = gSCManagerBattleState->players[port].fighter_gobj;

    if (fighter_gobj == NULL)
    {
        return;
    }
    fp = ftGetStruct(fighter_gobj);
    top_joint = fp->joints[nFTPartsJointTopN];
    spawn = &sSC1PTrainingRemixSpawns[port];

    spawn->x = top_joint->translate.vec.f.x;
    spawn->y = top_joint->translate.vec.f.y;
    spawn->lr = fp->lr;
    spawn->cliff_id = (fp->status_id == nFTCommonStatusCliffWait) ? fp->coll_data.cliff_id : SC1PTRAININGREMIXSPAWN_NO_LEDGE;
    spawn->is_custom = TRUE;
}

sb32 sc1PTrainingRemixSpawnIsCustom(s32 port)
{
    return (sc1PTrainingRemixSpawnIsValidPort(port) != FALSE) ? sSC1PTrainingRemixSpawns[port].is_custom : FALSE;
}

void sc1PTrainingRemixSpawnClear(s32 port)
{
    if (sc1PTrainingRemixSpawnIsValidPort(port) != FALSE)
    {
        sSC1PTrainingRemixSpawns[port].is_custom = FALSE;
    }
}

void sc1PTrainingRemixSpawnSelect(s32 port)
{
    if (sc1PTrainingRemixSpawnIsValidPort(port) != FALSE)
    {
        sSC1PTrainingRemixSpawns[port].is_custom = TRUE;
    }
}

sb32 sc1PTrainingRemixSpawnApplyDesc(s32 port, FTDesc *desc)
{
    SC1PTrainingRemixSpawn *spawn;

    if (sc1PTrainingRemixSpawnIsCustom(port) == FALSE)
    {
        return FALSE;
    }
    spawn = &sSC1PTrainingRemixSpawns[port];

    desc->pos.x = spawn->x;
    desc->pos.y = spawn->y;
    desc->lr = spawn->lr;

    return TRUE;
}

void sc1PTrainingRemixSpawnEnterLedge(s32 port, GObj *fighter_gobj)
{
    SC1PTrainingRemixSpawn *spawn;
    FTStruct *fp;

    if (sc1PTrainingRemixSpawnIsCustom(port) == FALSE)
    {
        return;
    }
    spawn = &sSC1PTrainingRemixSpawns[port];

    if (spawn->cliff_id < 0)
    {
        return;
    }
    fp = ftGetStruct(fighter_gobj);
    fp->coll_data.cliff_id = spawn->cliff_id;

    ftCommonCliffCatchSetStatus(fighter_gobj);
}

#endif
