#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixjabrules.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_BANJO_JAB_3_STATUS 0xDE
#define FTREMIX_DEDEDE_JAB_3_STATUS 0xFE
#define FTREMIX_BOWSER_JAB_3_STATUS 0xE9
#define FTREMIX_MEWTWO_RAPID_JAB_STATUS_START 0xE7

#define FTREMIX_RAPID_JAB_INPUTS_MIN_DEFAULT 4
#define FTREMIX_RAPID_JAB_INPUTS_MIN_LINK 5
#define FTREMIX_RAPID_JAB_INPUTS_MIN_CAPTAIN 6

const int dFTRemixJabRapidStyleFkinds[FTREMIX_RAPID_JAB_STYLE_COUNT] =
{
    REMIX_ID_FOX,
    REMIX_ID_LINK,
    REMIX_ID_KIRBY,
    REMIX_ID_JIGGLY,
    REMIX_ID_CAPTAIN
};

int ftRemixJabRulesIsJab3Gate(unsigned int gate_word)
{
    return gate_word == FTREMIX_JAB_3_GATE_ENABLED;
}

int ftRemixJabRulesIsRapidGate(unsigned int gate_word)
{
    return gate_word == FTREMIX_RAPID_JAB_GATE_ENABLED;
}

int ftRemixJabRulesFollowupKind(unsigned int timer_word)
{
    if (timer_word == FTREMIX_JAB_3_TIMER_DISABLED)
    {
        return nFTRemixJabFollowupKeep;
    }
    if (timer_word >= FTREMIX_JAB_CUSTOM_ROUTINE_MIN)
    {
        return nFTRemixJabFollowupLong;
    }
    return nFTRemixJabFollowupDefault;
}

static int ftRemixJabRulesCustomJab3Status(int fkind)
{
    switch (fkind)
    {
    case REMIX_ID_BANJO:
    case REMIX_ID_NBANJO:
        return FTREMIX_BANJO_JAB_3_STATUS;

    case REMIX_ID_DEDEDE:
    case REMIX_ID_NDEDEDE:
        return FTREMIX_DEDEDE_JAB_3_STATUS;

    case REMIX_ID_BOWSER:
    case REMIX_ID_NBOWSER:
    case REMIX_ID_GBOWSER:
        return FTREMIX_BOWSER_JAB_3_STATUS;

    default:
        return FTREMIX_JAB_STATUS_NONE;
    }
}

int ftRemixJabRulesJab3Status(int fkind, unsigned int action_word)
{
    if (action_word == FTREMIX_JAB_3_ACTION_DISABLED)
    {
        return FTREMIX_JAB_STATUS_NONE;
    }
    if (action_word >= FTREMIX_JAB_CUSTOM_ROUTINE_MIN)
    {
        return ftRemixJabRulesCustomJab3Status(fkind);
    }
    return FTREMIX_JAB_SPECIAL_STATUS_START;
}

int ftRemixJabRulesRapidStyle(unsigned int word, const unsigned int *vanilla_words)
{
    int index;

    if (word == 0)
    {
        return nFTRemixJabRapidStyleNone;
    }
    for (index = 0; index < FTREMIX_RAPID_JAB_STYLE_COUNT; index++)
    {
        if (vanilla_words[index] == word)
        {
            return index + 1;
        }
    }
    return nFTRemixJabRapidStyleNone;
}

static int ftRemixJabRulesIsMewtwo(int fkind)
{
    return (fkind == REMIX_ID_MTWO) || (fkind == REMIX_ID_NMTWO);
}

static int ftRemixJabRulesStyleStatus(int phase, int style)
{
    int status = FTREMIX_JAB_SPECIAL_STATUS_START + phase;

    switch (style)
    {
    case nFTRemixJabRapidStyleFox:
    case nFTRemixJabRapidStyleKirby:
    case nFTRemixJabRapidStylePurin:
        return status;

    case nFTRemixJabRapidStyleLink:
    case nFTRemixJabRapidStyleCaptain:
        return status + 1;

    default:
        return FTREMIX_JAB_STATUS_NONE;
    }
}

int ftRemixJabRulesRapidStatus(int phase, int fkind, unsigned int word, const unsigned int *vanilla_words)
{
    if (ftRemixJabRulesIsMewtwo(fkind) && (word >= FTREMIX_JAB_CUSTOM_ROUTINE_MIN))
    {
        return FTREMIX_MEWTWO_RAPID_JAB_STATUS_START + phase;
    }
    return ftRemixJabRulesStyleStatus(phase, ftRemixJabRulesRapidStyle(word, vanilla_words));
}

int ftRemixJabRulesRapidInputsMin(int style)
{
    switch (style)
    {
    case nFTRemixJabRapidStyleFox:
    case nFTRemixJabRapidStyleKirby:
    case nFTRemixJabRapidStylePurin:
        return FTREMIX_RAPID_JAB_INPUTS_MIN_DEFAULT;

    case nFTRemixJabRapidStyleLink:
        return FTREMIX_RAPID_JAB_INPUTS_MIN_LINK;

    case nFTRemixJabRapidStyleCaptain:
        return FTREMIX_RAPID_JAB_INPUTS_MIN_CAPTAIN;

    default:
        return 0;
    }
}

int ftRemixJabRulesStartsRapidJabAtJab2(int fkind)
{
    return ftRemixJabRulesIsMewtwo(fkind);
}

int ftRemixJabRulesRapidJabRestartsJab1(int fkind)
{
    return fkind == REMIX_ID_SLIPPY;
}

#endif
