//#include "AAMath/aamath.h"
#include "game_main.h"

static void GameUpdate(game_memory *memory, game_input *input)
{
    Assert(sizeof(game_state) <= memory->permanentMemorySize);

    game_state *gameState = (game_state *)memory->permanentMemory;

    if(!memory->isInitialised)
    {
        memory->isInitialised = true;

        // TODO: Initialise game state
    }
}
