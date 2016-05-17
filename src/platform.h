#ifndef PLATFORM_H
#define PLATFORM_H

#if AAENGINE_DEBUG
#define Assert(x) if(!(x)) {*(int *)0 = 0;}
#else
#define Assert(x)
#endif

/*
typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef float r32;
typedef int32_t b32;
*/

typedef size_t memsize;
typedef uintptr_t uptr;

#define global_variable static
#define local_persist static

#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))
#define Swap(a, b) {decltype(a) tmp = (a); (a) = (b); (b) = tmp;}

#define Kilobytes(n) ((n) * 1024LL)
#define Megabytes(n) (Kilobytes(n) * 1024LL)
#define Gigabytes(n) (Megabytes(n) * 1024LL)
#define Terabytes(n) (Gigabytes(n) * 1024LL)

struct game_button_state
{
    s32 HalfTransitionCount;
    b32 EndedDown;
};

struct game_controller
{
    b32 IsConnected;
    b32 IsAnalog;

    vec2 LeftStickAvg,
         RightStickAvg;

    r32 LeftTrigger;
    r32 RightTrigger;
    
    union
    {
        game_button_state Buttons[14];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;
            game_button_state Start;
            game_button_state Back;
            game_button_state LeftThumb;
            game_button_state RightThumb;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;

            // NOTE: New buttons must be added above here
            game_button_state BtnCount;
        };
    };
};

enum game_input_mouse_button
{
    MouseButton_Left,
    MouseButton_Middle,
    MouseButton_Right,
    MouseButton_Ext0,
    MouseButton_Ext1,

    MouseButton_Count,
};

struct game_mouse
{
    game_button_state Buttons[MouseButton_Count];
    r32 X, Y;
};

struct game_input
{
    r32 DeltaTime;
    game_controller Controllers[5];
    game_mouse Mouse;
};

inline game_controller *GetController(game_input *Input, u32 Idx)
{
    Assert(Idx < ArrayCount(Input->Controllers));
    return &Input->Controllers[Idx];
}

//
// NOTE: Platform API
//

#define PLATFORM_FREE_FILE(name) void name(void *File)
typedef PLATFORM_FREE_FILE(platform_free_file);

#define PLATFORM_READ_ENTIRE_FILE(name) void *name(WCHAR *Filename)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

typedef void platform_add_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);
typedef void platform_complete_all_work(platform_work_queue *queue);

#define PLATFORM_ALLOCATE_MEMORY(name) void *name(memsize Size)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_DEALLOCATE_MEMORY(name) void name(void *Memory)
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

#define PLATFORM_FULL_PATH_FROM_RELATIVE_WCHAR(name) WCHAR *name(WCHAR *AbsPath, const WCHAR *RelPath, size_t MaxLen)
typedef PLATFORM_FULL_PATH_FROM_RELATIVE_WCHAR(platform_full_path_from_relative_wchar);

struct platform_api
{
    platform_free_file *FreeFile;
    platform_read_entire_file *ReadEntireFile;

    platform_add_entry *AddEntry;
    platform_complete_all_work *CompleteAllWork;

    platform_allocate_memory *AllocateMemory;
    platform_deallocate_memory *DeallocateMemory;

    platform_full_path_from_relative_wchar *FullPathFromRelativeWchar;
};

//
//
//

struct game_memory
{
    u64 PermanentMemorySize;
    void *PermanentMemory;
    u64 TransientMemorySize;
    void *TransientMemory;

    platform_api PlatformAPI;
};

struct game_render_commands
{
    u32 Width;
    u32 Height;

    mat4 Projection;
    mat4 View;

    vec3 LightInvDir;

    u32 MaxPushBufferSize;
    u32 PushBufferSize;
    u8 *PushBufferBase;
};

// NOTE: https://en.wikipedia.org/wiki/Win32_Thread_Information_Block
//       Current thread ID is 0x24 (on 32-bit), but we're 64-bit so x2 -> 0x48.
inline u32 GetThreadID()
{
    /*
    u8 *ThreadLocal = (u8 *)__readgsqword(0x30);
    u32 ThreadID = *(u32 *)(ThreadLocal + 0x48);
    */
    u32 ThreadID = (u32)__readgsqword(0x48);
    return ThreadID;
}

static void GameUpdate(game_memory *Memory, s64 ElapsedTime, game_input *Input, game_render_commands *RenderCommands);

#endif

