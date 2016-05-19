#include <windows.h>
#define GLEW_STATIC
#include "glew\glew.h"
#include <gl\gl.h>
#include <stdio.h>
#include <xinput.h>
#include <stdint.h>
#include <dsound.h>
#include <mmintrin.h>

#include "AAMath/aamath.h"
using namespace aam;

// NOTE: Unity build (single compilation unit)!
#include "game.cpp"

global_variable b32 GlobalRunning;
global_variable WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};
global_variable RECT GlobalWindowRect;
global_variable LPDIRECTSOUNDBUFFER GlobalSoundBuffer;

static void ToggleFullscreen(HWND window)
{
    // NOTE: See https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if(style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO mi = {sizeof(mi)};
    
        if(GetWindowPlacement(window, &GlobalWindowPosition) &&
                GetMonitorInfo(MonitorFromWindow(window,
                        MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                    mi.rcMonitor.left, mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &GlobalWindowPosition);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    GetWindowRect(window, &GlobalWindowRect);
}

LRESULT CALLBACK MainWindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = {};

    switch(message)
    {
        case WM_SIZE:
        {
            GetWindowRect(hWnd, &GlobalWindowRect);
            OutputDebugStringA("WM_SIZE\n");
        } break;
        case WM_DESTROY:
        {
            GlobalRunning = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;
        case WM_CLOSE:
        {
            GlobalRunning = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard input isn't handled here anymore!");
        } break;
        default:
        {
            result = DefWindowProc(hWnd, message, wParam, lParam);
        } break;
    }

    return result;
}

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

static void LoadXInput()
{
    HMODULE xinputlib = LoadLibrary("xinput1_4.dll");
    if(!xinputlib)
    {
        xinputlib = LoadLibrary("xinput1_3.dll");
    }

    if(xinputlib)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(xinputlib, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(xinputlib, "XInputSetState");
    }
}

static void ProcessXInputButton(DWORD ButtonState, game_button_state *OldState, DWORD ButtonBit, game_button_state *NewState)
{
    NewState->EndedDown = (ButtonState & ButtonBit) == ButtonBit;
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

static vec2 ProcessXInputStick(SHORT x, SHORT y, SHORT deadzone)
{
    vec2 result = {};
    if(x > deadzone)
    {
        result.x = ((r32)x - (r32)deadzone) / (32767.0f - (r32)deadzone);
    }
    else if (x < -deadzone)
    {
        result.x = ((r32)x - (r32)deadzone) / (32768.0f - (r32)deadzone);
    }
    if(y > deadzone)
    {
        result.y = ((r32)y - (r32)deadzone) / (32767.0f - (r32)deadzone);
    }
    else if (y < -deadzone)
    {
        result.y = ((r32)y - (r32)deadzone) / (32768.0f - (r32)deadzone);
    }
    return result;
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

static void InitDSound(HWND hWnd, s32 SamplesPerSec, s32 BufferSize)
{
    HMODULE DSoundLib = LoadLibraryA("dsound.dll");

    if(DSoundLib)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLib, "DirectSoundCreate");

        LPDIRECTSOUND DSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DSound, 0)))
        {
            WAVEFORMATEX Wavefmt = {};
            Wavefmt.wFormatTag = WAVE_FORMAT_PCM;
            Wavefmt.nChannels = 2;
            Wavefmt.nSamplesPerSec = SamplesPerSec;
            Wavefmt.wBitsPerSample = 16;
            Wavefmt.nBlockAlign = (Wavefmt.nChannels * Wavefmt.wBitsPerSample) / 8;
            Wavefmt.nAvgBytesPerSec = Wavefmt.nSamplesPerSec * Wavefmt.nBlockAlign;
            Wavefmt.cbSize = 0;

            if(SUCCEEDED(DSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDesc = {};
                BufferDesc.dwSize = sizeof(BufferDesc);
                BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DSound->CreateSoundBuffer(&BufferDesc, &PrimaryBuffer, 0)))
                {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&Wavefmt)))
                    {
                        OutputDebugStringA("Primary buffer format created and set.\n");
                    }
                }
            }

            DSBUFFERDESC BufferDesc = {};
            BufferDesc.dwSize = sizeof(BufferDesc);
            BufferDesc.dwFlags = 0;
            BufferDesc.dwBufferBytes = BufferSize;
            BufferDesc.lpwfxFormat = &Wavefmt;
            if(SUCCEEDED(DSound->CreateSoundBuffer(&BufferDesc, &GlobalSoundBuffer, 0)))
            {
                OutputDebugStringA("Secondary buffer created.\n");
            }
        }
    }
}

struct sound_spec
{
    u32 SamplesPerSec;
    u32 LatencySampleCount;
    u32 BytesPerSample;
    u32 BufferSize;
    u32 SampleIdx;
    u32 Period;
    s16 Vol;
};

static void FillSoundBuffer(sound_spec *Spec, DWORD WritePtr, DWORD WriteBytes)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSoundBuffer->Lock(WritePtr, WriteBytes,
                                         &Region1, &Region1Size,
                                         &Region2, &Region2Size,
                                         0)))
    {

        DWORD Region1SampleCount = Region1Size / Spec->BytesPerSample;
        s16 *SampleOut = (s16 *)Region1;
        for(DWORD Idx = 0; Idx < Region1SampleCount; ++Idx)
        {
            r32 Sine = sinf(2 * PI * ((r32)Spec->SampleIdx / (r32)Spec->Period));
            s16 Sample = (s16)(Sine * Spec->Vol);
            *SampleOut++ = Sample;
            *SampleOut++ = Sample;
            ++Spec->SampleIdx;
        }

        DWORD Region2SampleCount = Region2Size / (sizeof(s16) * 2);
        SampleOut = (s16 *)Region2;
        for(DWORD Idx = 0; Idx < Region2SampleCount; ++Idx)
        {
            r32 Sine = sinf(2 * PI * ((r32)Spec->SampleIdx / (r32)Spec->Period));
            s16 Sample = (s16)(Sine * Spec->Vol);
            *SampleOut++ = Sample;
            *SampleOut++ = Sample;
            ++Spec->SampleIdx;
        }

        GlobalSoundBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

static void ProcessKeyboardKey(game_button_state *NewState, b32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

static void ProcessWindowsMessages(HWND Window, game_controller *KeyboardController)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                WPARAM VKCode = Message.wParam;
                b32 AltIsDown = (Message.lParam & (1 << 29)) != 0;
                b32 WasDown = (Message.lParam & (1 << 30)) != 0;
                b32 IsDown = (Message.lParam & (1 << 31)) == 0;

                if(WasDown != IsDown)
                {
                    switch(Message.wParam)
                    {
                        case 'W':
                        {
                            ProcessKeyboardKey(&KeyboardController->MoveUp, IsDown);
                        } break;
                        case 'S':
                        {
                            ProcessKeyboardKey(&KeyboardController->MoveDown, IsDown);
                        } break;
                        case 'A':
                        {
                            ProcessKeyboardKey(&KeyboardController->MoveLeft, IsDown);
                        } break;
                        case 'D':
                        {
                            ProcessKeyboardKey(&KeyboardController->MoveRight, IsDown);
                        } break;
                        case 'Q':
                        {
                            //ProcessKeyboardKey(&KeyboardController->???, IsDown);
                        } break;
                        case 'E':
                        {
                            //ProcessKeyboardKey(&KeyboardController->???, IsDown);
                        } break;
                        case VK_SPACE:
                        {
                            ProcessKeyboardKey(&KeyboardController->RightShoulder, IsDown);
                        } break;
                        case VK_SHIFT:
                        {
                            ProcessKeyboardKey(&KeyboardController->LeftShoulder, IsDown);
                        } break;
                        case VK_UP:
                        {
                            ProcessKeyboardKey(&KeyboardController->ActionUp, IsDown);
                        } break;
                        case VK_DOWN:
                        {
                            ProcessKeyboardKey(&KeyboardController->ActionDown, IsDown);
                        } break;
                        case VK_LEFT:
                        {
                            ProcessKeyboardKey(&KeyboardController->ActionLeft, IsDown);
                        } break;
                        case VK_RIGHT:
                        {
                            ProcessKeyboardKey(&KeyboardController->ActionRight, IsDown);
                        } break;
                        case VK_ESCAPE:
                        {
                            if(IsDown)
                            {
                                GlobalRunning = false;
                            }
                        } break;
                        case VK_F4:
                        {
                            if(IsDown && AltIsDown)
                            {
                                GlobalRunning = false;
                            }
                        } break;
                        case VK_RETURN:
                        {
                            if(IsDown && AltIsDown)
                            {
                                ToggleFullscreen(Window);
                            }
                            else
                            {
                                ProcessKeyboardKey(&KeyboardController->Start, IsDown);
                            }
                        } break;
                    }
                }
            } break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
}

static PLATFORM_ALLOCATE_MEMORY(AllocateMemory)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    return(Result);
}

static PLATFORM_DEALLOCATE_MEMORY(DeallocateMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

static PLATFORM_FULL_PATH_FROM_RELATIVE_WCHAR(FullPathFromRelativeWchar)
{
    Assert(MaxLen >= 0);
    WCHAR *Result = _wfullpath(AbsPath, RelPath, MaxLen);
    return Result;
}

static PLATFORM_FREE_FILE(FreeFile)
{
    if(File)
    {
        VirtualFree(File, 0, MEM_RELEASE);
    }
}

static PLATFORM_READ_ENTIRE_FILE(ReadEntireFile)
{
    void *Result = 0;

    HANDLE File = CreateFileW(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(File != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER Size;
        if(GetFileSizeEx(File, &Size))
        {
            Assert(Size.QuadPart <= 0xFFFFFFFF);
            Result = VirtualAlloc(0, Size.LowPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result)
            {
                DWORD BytesRead;
                if(ReadFile(File, Result, Size.LowPart, &BytesRead, 0) && (Size.LowPart == BytesRead))
                {
                }
                else
                {
                    FreeFile(File);
                    Result = 0;
                }
            }
        }
        CloseHandle(File);
    }

    return Result;
}

#define WriteBarrier _WriteBarrier(); _mm_sfence()
#define ReadBarrier _ReadBarrier()

struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    u32 volatile Goal;
    u32 volatile Count;
    u32 volatile NextWrite;
    u32 volatile NextRead;
    HANDLE Semaphore;

    platform_work_queue_entry Entries[256];
};

static void AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO: Use InterlockedCompareExchange to allow any thread to add
    u32 NextWrite = (Queue->NextWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NextWrite != Queue->NextRead);

    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->Goal;
    WriteBarrier;
    Queue->NextWrite = NextWrite;
    ReleaseSemaphore(Queue->Semaphore, 1, 0);
}

static b32 DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    b32 Sleep = false;

    u32 NextRead = Queue->NextRead;
    u32 NewNextRead = (NextRead + 1) % ArrayCount(Queue->Entries);

    if(NextRead != Queue->NextWrite)
    {
        u32 Idx = InterlockedCompareExchange((LONG volatile *)&Queue->NextRead,
                                             NewNextRead,
                                             NextRead);
        if(Idx == NextRead)
        {
            platform_work_queue_entry Entry = Queue->Entries[Idx];
            Entry.Callback(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->Count);
        }
    }
    else
    {
        Sleep = true;
    }

    return Sleep;
}

static void CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->Goal != Queue->Count)
    {
        DoNextWorkQueueEntry(Queue);
    }

    Queue->Goal = 0;
    Queue->Count = 0;
}

DWORD WINAPI ThreadProc(LPVOID Param)
{
    platform_work_queue *Queue = (platform_work_queue *)Param;

    u32 TestID = GetThreadID();
    Assert(TestID == GetCurrentThreadId());

    for(;;)
    {
        if(DoNextWorkQueueEntry(Queue))
        {
            WaitForSingleObjectEx(Queue->Semaphore, INFINITE, FALSE);
        }
    }

    //return 0;
}

inline PLATFORM_WORK_QUEUE_CALLBACK(ThreadOutputString)
{
    char Buf[256];
    wsprintf(Buf, "Thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
    OutputDebugStringA(Buf);
}

static void MakeQueue(platform_work_queue *Queue, u32 ThreadCount)
{
    Queue->Goal = 0;
    Queue->Count = 0;
    Queue->NextWrite = 0;
    Queue->NextRead = 0;

    u32 Count = 0;
    Queue->Semaphore = CreateSemaphoreEx(0, Count, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);

    for(u32 Idx = 0; Idx < ThreadCount; ++Idx)
    {
        DWORD ThreadID;
        HANDLE hThread = CreateThread(0, 0, ThreadProc, Queue, 0, &ThreadID);
        CloseHandle(hThread);
    }
}

static s64 GetElapsedTime(s64 Frequency, s64 LastTimestamp)
{
    LARGE_INTEGER Now;
    QueryPerformanceCounter(&Now);

    s64 Time = Now.QuadPart - LastTimestamp;
    Time = (1000 * Time) / Frequency;

    return Time;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    LoadXInput();

#if 0
    platform_work_queue WorkQueue = {};
    MakeQueue(&WorkQueue, 4);

    AddEntry(&WorkQueue, ThreadOutputString, "String A0");
    AddEntry(&WorkQueue, ThreadOutputString, "String A1");
    AddEntry(&WorkQueue, ThreadOutputString, "String A2");
    AddEntry(&WorkQueue, ThreadOutputString, "String A3");
    AddEntry(&WorkQueue, ThreadOutputString, "String A4");
    AddEntry(&WorkQueue, ThreadOutputString, "String A5");
    AddEntry(&WorkQueue, ThreadOutputString, "String A6");
    AddEntry(&WorkQueue, ThreadOutputString, "String A7");
    AddEntry(&WorkQueue, ThreadOutputString, "String A8");
    AddEntry(&WorkQueue, ThreadOutputString, "String A9");

    AddEntry(&WorkQueue, ThreadOutputString, "String B0");
    AddEntry(&WorkQueue, ThreadOutputString, "String B1");
    AddEntry(&WorkQueue, ThreadOutputString, "String B2");
    AddEntry(&WorkQueue, ThreadOutputString, "String B3");
    AddEntry(&WorkQueue, ThreadOutputString, "String B4");
    AddEntry(&WorkQueue, ThreadOutputString, "String B5");
    AddEntry(&WorkQueue, ThreadOutputString, "String B6");
    AddEntry(&WorkQueue, ThreadOutputString, "String B7");
    AddEntry(&WorkQueue, ThreadOutputString, "String B8");
    AddEntry(&WorkQueue, ThreadOutputString, "String B9");

    CompleteAllWork(&WorkQueue);
#endif

    LARGE_INTEGER PerfFrequencyResult;
    QueryPerformanceFrequency(&PerfFrequencyResult);
    s64 PerfFrequency = PerfFrequencyResult.QuadPart;

    LARGE_INTEGER StartCounter;
    QueryPerformanceCounter(&StartCounter);

    WNDCLASS WC = {};
    WC.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WC.lpfnWndProc = MainWindowCallback;
    WC.hInstance = hInstance;
    //WC.hIcon = ;
    WC.hCursor = LoadCursor(0, IDC_ARROW);
    WC.lpszClassName = "AAEngineWindowClass";

    if(RegisterClass(&WC))
    {
        u32 WindowWidth = 1280;
        u32 WindowHeight = 720;

        HWND WindowHandle =
            CreateWindowEx(
                0, 
                WC.lpszClassName,
                "AAEngine",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                0, 0, WindowWidth, WindowHeight,
                0,
                0,
                hInstance,
                0);
        if(WindowHandle)
        {
            HDC DeviceContext;
            HGLRC RenderContext;

            // NOTE: DirectSound init
            sound_spec SoundSpec = {};
            SoundSpec.SamplesPerSec = 48000;
            SoundSpec.LatencySampleCount = SoundSpec.SamplesPerSec / 15;
            SoundSpec.BytesPerSample = sizeof(s16) * 2;
            SoundSpec.BufferSize = SoundSpec.SamplesPerSec * SoundSpec.BytesPerSample;
            SoundSpec.SampleIdx = 0;
            SoundSpec.Period = SoundSpec.SamplesPerSec / 261;
            SoundSpec.Vol = 1600;

            InitDSound(WindowHandle, SoundSpec.SamplesPerSec, SoundSpec.BufferSize);
            FillSoundBuffer(&SoundSpec, 0, SoundSpec.LatencySampleCount * SoundSpec.BytesPerSample);
            //GlobalSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GetWindowRect(WindowHandle, &GlobalWindowRect);
            // NOTE: OpenGL init
            InitializeOpenGL(WindowHandle, &DeviceContext, &RenderContext, WindowWidth, WindowHeight);

            // TODO: Decide render buffer size
            memsize PushBufferSize = Megabytes(16);
            void *PushBuffer = AllocateMemory(PushBufferSize);

#if AAENGINE_INTERNAL
            LPVOID BaseAddr = (LPVOID)Terabytes((u64)2);
#else
            LPVOID BaseAddr = 0;
#endif

            // NOTE: Init game memory and platform API
            game_memory GameMemory = {};
            GameMemory.PermanentMemorySize = Megabytes(64);
            GameMemory.TransientMemorySize = Gigabytes(4);
            GameMemory.PlatformAPI.FreeFile = FreeFile;
            GameMemory.PlatformAPI.ReadEntireFile = ReadEntireFile;
            GameMemory.PlatformAPI.AddEntry = AddEntry;
            GameMemory.PlatformAPI.CompleteAllWork = CompleteAllWork;
            GameMemory.PlatformAPI.AllocateMemory = AllocateMemory;
            GameMemory.PlatformAPI.DeallocateMemory = DeallocateMemory;
            GameMemory.PlatformAPI.FullPathFromRelativeWchar = FullPathFromRelativeWchar;

            u64 GameMemorySize = GameMemory.PermanentMemorySize + GameMemory.TransientMemorySize;

            GameMemory.PermanentMemory = VirtualAlloc(BaseAddr, GameMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientMemory = (u8 *)GameMemory.PermanentMemory + GameMemory.PermanentMemorySize;

            if(GameMemory.PermanentMemory && GameMemory.TransientMemory)
            {
                LARGE_INTEGER LastCounter;
                QueryPerformanceCounter(&LastCounter);

                u64 LastCycleCount = __rdtsc();

                // TODO: Get VREFRESH / refresh rate
                r32 TargetMSPerFrame = 1000.0f / 60.0f;
                r32 MSPerFrame = TargetMSPerFrame;

                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];

                //
                // NOTE: Main loop
                //
                
                GlobalRunning = true;

                while(GlobalRunning)
                {
                    // NOTE: deltaTime in seconds
                    //r32 DeltaTime = MSPerFrame / 1000.0f;
                    s64 ElapsedTime = GetElapsedTime(PerfFrequency, StartCounter.QuadPart);

                    POINT MousePos;
                    GetCursorPos(&MousePos);
                    ScreenToClient(WindowHandle, &MousePos);
                    NewInput->Mouse.X = (r32)MousePos.x / WindowWidth;
                    NewInput->Mouse.Y = (r32)MousePos.y / WindowHeight;

                    // TODO: Mousewheel support

                    DWORD MouseButtonID[MouseButton_Count] =
                    {
                        VK_LBUTTON,
                        VK_MBUTTON,
                        VK_RBUTTON,
                        VK_XBUTTON1,
                        VK_XBUTTON2,
                    };

                    for(u32 BtnIdx = 0; BtnIdx < MouseButton_Count; ++BtnIdx)
                    {
                        NewInput->Mouse.Buttons[BtnIdx] = OldInput->Mouse.Buttons[BtnIdx];
                        NewInput->Mouse.Buttons[BtnIdx].HalfTransitionCount = 0;
                        ProcessKeyboardKey(&NewInput->Mouse.Buttons[BtnIdx],
                                           GetKeyState(MouseButtonID[BtnIdx]) & (1 << 15));
                    }

                    game_controller *OldKeyboardController = GetController(OldInput, 0);
                    game_controller *NewKeyboardController = GetController(NewInput, 0);
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;

                    for(s32 BtnIdx = 0; BtnIdx < ArrayCount(NewKeyboardController->Buttons); ++BtnIdx)
                    {
                        NewKeyboardController->Buttons[BtnIdx] = OldKeyboardController->Buttons[BtnIdx];
                    }

                    ProcessWindowsMessages(WindowHandle, NewKeyboardController);

                    // NOTE: Use XUSER_MAX_COUNT?
                    DWORD MaxControllers = ArrayCount(NewInput->Controllers) - 1;
                    for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllers; ++ControllerIndex)
                    {
                        // NOTE: First controller index is reserved for the keyboard
                        game_controller *OldController = GetController(OldInput, ControllerIndex + 1);
                        game_controller *NewController = GetController(NewInput, ControllerIndex + 1);

                        // TODO: Apparently there's some bug where XInputGetState delays for a relatively long time
                        // if no controllers are connected, so it may be better to listen to HID connections directly
                        // and then only call get state on controllers we know exist.
                        XINPUT_STATE ControllerState;
                        if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                        {
                            NewController->IsConnected = true;
                            NewController->IsAnalog = OldController->IsAnalog;
                            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->MoveUp, XINPUT_GAMEPAD_DPAD_UP,
                                    &NewController->MoveUp);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->MoveDown, XINPUT_GAMEPAD_DPAD_DOWN,
                                    &NewController->MoveDown);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->MoveLeft, XINPUT_GAMEPAD_DPAD_LEFT,
                                    &NewController->MoveLeft);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->MoveRight, XINPUT_GAMEPAD_DPAD_RIGHT,
                                    &NewController->MoveRight);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->Start, XINPUT_GAMEPAD_START,
                                    &NewController->Start);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->Back, XINPUT_GAMEPAD_BACK,
                                    &NewController->Back);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->LeftThumb, XINPUT_GAMEPAD_LEFT_THUMB,
                                    &NewController->LeftThumb);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->RightThumb, XINPUT_GAMEPAD_RIGHT_THUMB,
                                    &NewController->RightThumb);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
                                    &NewController->LeftShoulder);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                    &NewController->RightShoulder);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->ActionDown, XINPUT_GAMEPAD_A,
                                    &NewController->ActionDown);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->ActionRight, XINPUT_GAMEPAD_B,
                                    &NewController->ActionRight);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->ActionLeft, XINPUT_GAMEPAD_X,
                                    &NewController->ActionLeft);
                            ProcessXInputButton(Pad->wButtons,
                                    &OldController->ActionUp, XINPUT_GAMEPAD_Y,
                                    &NewController->ActionUp);

                            NewController->IsAnalog = true;
                            NewController->LeftTrigger = (r32)Pad->bLeftTrigger / 255.0f;
                            NewController->RightTrigger = (r32)Pad->bRightTrigger / 255.0f;

#define GAMEPAD_LEFT_THUMB_DEADZONE 3277
#define GAMEPAD_RIGHT_THUMB_DEADZONE 3277

                            NewController->LeftStickAvg = ProcessXInputStick(Pad->sThumbLX, Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            NewController->RightStickAvg = ProcessXInputStick(Pad->sThumbRX, Pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                            if(Length(NewController->LeftStickAvg) > GAMEPAD_LEFT_THUMB_DEADZONE
                                || Length(NewController->RightStickAvg) > GAMEPAD_RIGHT_THUMB_DEADZONE)
                            {
                                NewController->IsAnalog = true;
                            }
                            else
                            {
                                NewController->IsAnalog = false;
                            }

                            r32 Threshold = 0.5f;
                            ProcessXInputButton(
                                    (NewController->LeftStickAvg.x < -Threshold) ? 1 : 0,
                                    &OldController->MoveLeft, 1,
                                    &NewController->MoveLeft);
                            ProcessXInputButton(
                                    (NewController->LeftStickAvg.x > Threshold) ? 1 : 0,
                                    &OldController->MoveRight, 1,
                                    &NewController->MoveRight);
                            ProcessXInputButton(
                                    (NewController->LeftStickAvg.y < -Threshold) ? 1 : 0,
                                    &OldController->MoveDown, 1,
                                    &NewController->MoveDown);
                            ProcessXInputButton(
                                    (NewController->LeftStickAvg.y > Threshold) ? 1 : 0,
                                    &OldController->MoveUp, 1,
                                    &NewController->MoveUp);
                        }
                        else
                        {
                            // NOTE: Controller unavailable
                            NewController->IsConnected = false;
                        }
                    }

                    //
                    // NOTE: Update
                    //

                    game_render_commands RenderCommands =
                    {
                        (GlobalWindowRect.right - GlobalWindowRect.left),
                        (GlobalWindowRect.bottom - GlobalWindowRect.top),
                        MAT4_IDENTITY, MAT4_IDENTITY,
                        Vec3(2.0f, 4.0f, 2.0f),
                        (u32)PushBufferSize, 0, (u8 *)PushBuffer
                    };

                    GameUpdate(&GameMemory, ElapsedTime, NewInput, &RenderCommands);

                    //
                    // NOTE: Render
                    //

                    //RenderOpenGL(&windowHandle, &deviceContext, 0, 0, windowWidth, windowHeight);
                    RenderOpenGL(&WindowHandle, &DeviceContext, 0, 0,
                                (GlobalWindowRect.right - GlobalWindowRect.left),
                                (GlobalWindowRect.bottom - GlobalWindowRect.top),
                                &RenderCommands);

                    // NOTE: DSound test
                    //      s16     s16     s16     s16     ...
                    //      LEFT    RIGHT   LEFT    RIGHT   ...

                    DWORD PlayCursor;
                    DWORD WriteCursor;
                    if(SUCCEEDED(GlobalSoundBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                    {
                        DWORD Writeptr = (SoundSpec.SampleIdx * SoundSpec.BytesPerSample) % SoundSpec.BufferSize;
                        DWORD Target = (PlayCursor + (SoundSpec.LatencySampleCount * SoundSpec.BytesPerSample)) % SoundSpec.BufferSize;
                        DWORD Writebytes;
                        if(Writeptr > Target)
                        {
                            Writebytes = SoundSpec.BufferSize - Writeptr;
                            Writebytes += Target;
                        }
                        else
                        {
                            Writebytes = Target - Writeptr;
                        }

                        FillSoundBuffer(&SoundSpec, Writeptr, Writebytes);
                    }

                    // NOTE: Swap game input
                    Swap(NewInput, OldInput);

                    // NOTE: Timing
                    u64 EndCycleCount = __rdtsc();

                    LARGE_INTEGER EndCounter;
                    QueryPerformanceCounter(&EndCounter);

                    u64 CyclesElapsed = EndCycleCount - LastCycleCount;
                    u64 Elapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                    MSPerFrame = (1000.0f * (r32)Elapsed) / (r32)PerfFrequency;
                    r32 FPS = (r32)PerfFrequency / (r32)Elapsed;
                    r32 MCPF = CyclesElapsed / 1000000.0f;

                    char Buf[256];
                    sprintf_s(Buf, sizeof(Buf), "%.3fms (%.3f FPS), %.3fMCycles\n", MSPerFrame, FPS, MCPF);
                    OutputDebugStringA(Buf);

                    LastCycleCount = EndCycleCount;
                    LastCounter = EndCounter;
                }
            }

            TerminateOpenGL(WindowHandle, DeviceContext, RenderContext);
        }
        else
        {
            // TODO: log
        }
    }
    else
    {
        // TODO: log
    }

    return 0;
}
