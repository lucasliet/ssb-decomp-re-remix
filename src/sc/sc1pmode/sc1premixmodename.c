/**
 * sc1premixmodename.c - names of the Remix 1P modes, see sc1premixmodename.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixmodename.h"

#include <stddef.h>

extern int strcmp(const char *a, const char *b);

typedef struct SC1PRemixModeName
{
    const char *name;
    int mode;
} SC1PRemixModeName;

static const SC1PRemixModeName dSC1PRemixModeNames[] =
{
    { "standard", SC1PREMIX_MODE_STANDARD },
    { "bonus3", SC1PREMIX_MODE_BONUS3 },
    { "multiman", SC1PREMIX_MODE_MULTIMAN },
    { "cruel", SC1PREMIX_MODE_CRUEL },
    { "remix1p", SC1PREMIX_MODE_REMIX_1P },
    { "allstar", SC1PREMIX_MODE_ALLSTAR },
    { "hrc", SC1PREMIX_MODE_HRC }
};

int sc1PRemixModeFromName(const char *name)
{
    unsigned int i;

    if (name == NULL)
    {
        return -1;
    }
    for (i = 0; i < (sizeof(dSC1PRemixModeNames) / sizeof(dSC1PRemixModeNames[0])); i++)
    {
        if (strcmp(name, dSC1PRemixModeNames[i].name) == 0)
        {
            return dSC1PRemixModeNames[i].mode;
        }
    }
    return -1;
}

#endif
