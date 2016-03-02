#ifndef PLATFORM_H
#define PLATFORM_H

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

struct game_controller
{
    b32 isConnected;
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

            // NOTE: New buttons must be added above here
            game_button_state btnCount;
        };
    };
};

enum game_input_mouse_button
{
    mouse_button_left,
    mouse_button_middle,
    mouse_button_right,
    mouse_button_ext0,
    mouse_button_ext1,

    mouse_button_count,
};

struct game_mouse
{
    game_button_state buttons[mouse_button_count];
    r32 x, y;
};

struct game_input
{
    r32 deltaTime;
    game_controller controllers[5];
    game_mouse mouse;
};

inline game_controller *GetController(game_input *input, u32 idx)
{
    Assert(idx < ArrayCount(input->controllers));
    return &input->controllers[idx];
}

struct game_memory
{
    u64 permanentMemorySize;
    void *permanentMemory;
    u64 transientMemorySize;
    void *transientMemory;
};

struct game_render_commands
{
    u32 width;
    u32 height;

    mat4 projection;
    mat4 view;

    vec3 lightInvDir;

    u32 maxPushBufferSize;
    u32 pushBufferSize;
    u8 *pushBufferBase;
};

#define PLATFORM_ALLOCATE_MEMORY(name) void *name(memsize size)
#define PLATFORM_DEALLOCATE_MEMORY(name) void name(void *memory)

static void GameUpdate(game_memory *memory, game_input *input, game_render_commands *renderCommands);

#endif

