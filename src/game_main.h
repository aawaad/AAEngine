#ifndef GAME_MAIN_H
#define GAME_MAIN_H

/*
#if AAENGINE_DEBUG
#define Assert(x) if(!(x)) {*(int *)0 = 0;}
#else
#define Assert(x)
#endif
*/

#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))
#define Swap(a, b) {decltype(a) tmp = (a); (a) = (b); (b) = tmp;}

#define Kilobytes(n) ((n) * 1024LL)
#define Megabytes(n) (Kilobytes(n) * 1024LL)
#define Gigabytes(n) (Megabytes(n) * 1024LL)
#define Terabytes(n) (Gigabytes(n) * 1024LL)

struct game_button_state
{
    s32 halfTransitionCount;
    b32 endedDown;
};

struct game_controller_input
{
    b32 isAnalog;

    vec2 leftStickAvg,
       rightStickAvg;

    r32 leftTrigger;
    r32 rightTrigger;
    
    union
    {
        game_button_state buttons[14];
        struct
        {
            game_button_state moveUp;
            game_button_state moveDown;
            game_button_state moveLeft;
            game_button_state moveRight;
            game_button_state start;
            game_button_state back;
            game_button_state leftThumb;
            game_button_state rightThumb;
            game_button_state leftShoulder;
            game_button_state rightShoulder;
            game_button_state actionUp;
            game_button_state actionDown;
            game_button_state actionLeft;
            game_button_state actionRight;
            // TODO: Do something about size mismatch of this struct and the array ^
        };
    };
};

struct game_input
{
    r32 deltaTime;
    game_controller_input controllers[5];
};

struct game_memory
{
    b32 isInitialised;
    u64 permanentMemorySize;
    void *permanentMemory;
    u64 transientMemorySize;
    void *transientMemory;
};

struct game_state
{
};

static void GameUpdate(game_memory *memory, game_input *input);

#endif

