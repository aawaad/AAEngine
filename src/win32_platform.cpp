#include <windows.h>
#define GLEW_STATIC
#include "glew\glew.h"
#include <gl\gl.h>
#include <stdio.h>
#include <xinput.h>
#include <stdint.h>

#include "../../AAMath/src/aamath.h"
using namespace aam;

// NOTE: Unity build (single compilation unit)!
#include "game.cpp"

global_variable b32 GlobalRunning;
global_variable WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};
global_variable RECT GlobalWindowRect;

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

static void ProcessXInputButton(DWORD buttonState, game_button_state *oldState, DWORD buttonBit, game_button_state *newState)
{
    newState->endedDown = (buttonState & buttonBit) == buttonBit;
    newState->halfTransitionCount = (oldState->endedDown != newState->endedDown) ? 1 : 0;
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

static void ProcessKeyboardKey(game_button_state *newState, b32 isDown)
{
    if(newState->endedDown != isDown)
    {
        newState->endedDown = isDown;
        ++newState->halfTransitionCount;
    }
}

static void ProcessWindowsMessages(HWND window, game_controller *keyboardController)
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
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
                WPARAM vkCode = message.wParam;
                b32 altIsDown = (message.lParam & (1 << 29)) != 0;
                b32 wasDown = (message.lParam & (1 << 30)) != 0;
                b32 isDown = (message.lParam & (1 << 31)) == 0;

                if(wasDown != isDown)
                {
                    switch(message.wParam)
                    {
                        case 'W':
                        {
                            ProcessKeyboardKey(&keyboardController->moveUp, isDown);
                        } break;
                        case 'S':
                        {
                            ProcessKeyboardKey(&keyboardController->moveDown, isDown);
                        } break;
                        case 'A':
                        {
                            ProcessKeyboardKey(&keyboardController->moveLeft, isDown);
                        } break;
                        case 'D':
                        {
                            ProcessKeyboardKey(&keyboardController->moveRight, isDown);
                        } break;
                        case 'Q':
                        {
                            ProcessKeyboardKey(&keyboardController->leftShoulder, isDown);
                        } break;
                        case 'E':
                        {
                            ProcessKeyboardKey(&keyboardController->rightShoulder, isDown);
                        } break;
                        case VK_SPACE:
                        {
                            //ProcessKeyboardKey(&keyboardController->???, isDown);
                        } break;
                        case VK_SHIFT:
                        {
                            //ProcessKeyboardKey(&keyboardController->???, isDown);
                        } break;
                        case VK_ESCAPE:
                        {
                            if(isDown)
                            {
                                GlobalRunning = false;
                            }
                        } break;
                        case VK_F4:
                        {
                            if(isDown && altIsDown)
                            {
                                GlobalRunning = false;
                            }
                        } break;
                        case VK_RETURN:
                        {
                            if(isDown && altIsDown)
                            {
                                ToggleFullscreen(window);
                            }
                            else
                            {
                                ProcessKeyboardKey(&keyboardController->start, isDown);
                            }
                        } break;
                    }
                }
            } break;
            default:
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            } break;
        }
    }
}

PLATFORM_ALLOCATE_MEMORY(AllocateMemory)
{
    void *result = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    return(result);
}

PLATFORM_DEALLOCATE_MEMORY(DeallocateMemory)
{
    if(memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    LoadXInput();

    LARGE_INTEGER perfFrequencyResult;
    QueryPerformanceFrequency(&perfFrequencyResult);
    s64 perfFrequency = perfFrequencyResult.QuadPart;

    WNDCLASS wc = {};
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowCallback;
    wc.hInstance = hInstance;
//    windowClass.hIcon = ;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.lpszClassName = "AAEngineWindowClass";

    if(RegisterClass(&wc))
    {
        u32 windowWidth = 1280;
        u32 windowHeight = 720;

        HWND windowHandle =
            CreateWindowEx(
                0, 
                wc.lpszClassName,
                "AAEngine",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                0, 0, windowWidth, windowHeight,
                0,
                0,
                hInstance,
                0);
        if(windowHandle)
        {
            HDC deviceContext;
            HGLRC renderContext;

            GetWindowRect(windowHandle, &GlobalWindowRect);
            // NOTE: OpenGL init
            InitializeOpenGL(windowHandle, &deviceContext, &renderContext, windowWidth, windowHeight);

            // NOTE: Main loop
            GlobalRunning = true;

            // TODO: Decide render buffer size
            memsize pushBufferSize = Megabytes(16);
            void *pushBuffer = AllocateMemory(pushBufferSize);

#if AAENGINE_INTERNAL
            LPVOID baseAddr = (LPVOID)Terabytes((u64)2);
#else
            LPVOID baseAddr = 0;
#endif

            game_memory gameMemory = {};
            gameMemory.permanentMemorySize = Megabytes(64);
            gameMemory.transientMemorySize = Gigabytes(4);

            u64 gameMemorySize = gameMemory.permanentMemorySize + gameMemory.transientMemorySize;

            gameMemory.permanentMemory = VirtualAlloc(baseAddr, gameMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            gameMemory.transientMemory = (u8 *)gameMemory.permanentMemory + gameMemory.permanentMemorySize;

            if(gameMemory.permanentMemory && gameMemory.transientMemory)
            {
                LARGE_INTEGER lastCounter;
                QueryPerformanceCounter(&lastCounter);

                u64 lastCycleCount = __rdtsc();

                // TODO: Get VREFRESH / refresh rate
                r32 targetMSPerFrame = 1000.0f / 60.0f;
                r32 msPerFrame = targetMSPerFrame;

                game_input input[2] = {};
                game_input *newInput = &input[0];
                game_input *oldInput = &input[1];

                while(GlobalRunning)
                {
                    // NOTE: deltaTime in seconds
                    newInput->deltaTime = msPerFrame / 1000.0f;

                    POINT mousePos;
                    GetCursorPos(&mousePos);
                    ScreenToClient(windowHandle, &mousePos);
                    newInput->mouse.x = (r32)mousePos.x / windowWidth;
                    newInput->mouse.y = (r32)mousePos.y / windowHeight;

                    // TODO: Mousewheel support

                    DWORD mouseButtonID[mouse_button_count] =
                    {
                        VK_LBUTTON,
                        VK_MBUTTON,
                        VK_RBUTTON,
                        VK_XBUTTON1,
                        VK_XBUTTON2,
                    };

                    for(u32 btnIdx = 0; btnIdx < mouse_button_count; ++btnIdx)
                    {
                        newInput->mouse.buttons[btnIdx] = oldInput->mouse.buttons[btnIdx];
                        newInput->mouse.buttons[btnIdx].halfTransitionCount = 0;
                        ProcessKeyboardKey(&newInput->mouse.buttons[btnIdx],
                                           GetKeyState(mouseButtonID[btnIdx]) & (1 << 15));
                    }

                    game_controller *oldKeyboardController = GetController(oldInput, 0);
                    game_controller *newKeyboardController = GetController(newInput, 0);
                    *newKeyboardController = {};
                    newKeyboardController->isConnected = true;

                    for(s32 btnIdx = 0; btnIdx < ArrayCount(newKeyboardController->buttons); ++btnIdx)
                    {
                        newKeyboardController->buttons[btnIdx] = oldKeyboardController->buttons[btnIdx];
                    }

                    ProcessWindowsMessages(windowHandle, newKeyboardController);

                    // NOTE: Use XUSER_MAX_COUNT?
                    DWORD maxControllers = ArrayCount(newInput->controllers) - 1;
                    for(DWORD controllerIndex = 0; controllerIndex < maxControllers; ++controllerIndex)
                    {
                        // NOTE: First controller index is reserved for the keyboard
                        game_controller *oldController = GetController(oldInput, controllerIndex + 1);
                        game_controller *newController = GetController(newInput, controllerIndex + 1);

                        // TODO: Apparently there's some bug where XInputGetState delays for a relatively long time
                        // if no controllers are connected, so it may be better to listen to HID connections directly
                        // and then only call get state on controllers we know exist.
                        XINPUT_STATE controllerState;
                        if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
                        {
                            newController->isConnected = true;
                            newController->isAnalog = oldController->isAnalog;
                            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

                            ProcessXInputButton(pad->wButtons,
                                    &oldController->moveUp, XINPUT_GAMEPAD_DPAD_UP,
                                    &newController->moveUp);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->moveDown, XINPUT_GAMEPAD_DPAD_DOWN,
                                    &newController->moveDown);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->moveLeft, XINPUT_GAMEPAD_DPAD_LEFT,
                                    &newController->moveLeft);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->moveRight, XINPUT_GAMEPAD_DPAD_RIGHT,
                                    &newController->moveRight);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->start, XINPUT_GAMEPAD_START,
                                    &newController->start);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->back, XINPUT_GAMEPAD_BACK,
                                    &newController->back);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->leftThumb, XINPUT_GAMEPAD_LEFT_THUMB,
                                    &newController->leftThumb);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->rightThumb, XINPUT_GAMEPAD_RIGHT_THUMB,
                                    &newController->rightThumb);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->leftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
                                    &newController->leftShoulder);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->rightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                    &newController->rightShoulder);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->actionDown, XINPUT_GAMEPAD_A,
                                    &newController->actionDown);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->actionRight, XINPUT_GAMEPAD_B,
                                    &newController->actionRight);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->actionLeft, XINPUT_GAMEPAD_X,
                                    &newController->actionLeft);
                            ProcessXInputButton(pad->wButtons,
                                    &oldController->actionUp, XINPUT_GAMEPAD_Y,
                                    &newController->actionUp);

                            newController->isAnalog = true;
                            newController->leftTrigger = (r32)pad->bLeftTrigger / 255.0f;
                            newController->rightTrigger = (r32)pad->bRightTrigger / 255.0f;

#define GAMEPAD_LEFT_THUMB_DEADZONE 3277
#define GAMEPAD_RIGHT_THUMB_DEADZONE 3277

                            newController->leftStickAvg = ProcessXInputStick(pad->sThumbLX, pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            newController->rightStickAvg = ProcessXInputStick(pad->sThumbRX, pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                            if(Length(newController->leftStickAvg) > GAMEPAD_LEFT_THUMB_DEADZONE
                                || Length(newController->rightStickAvg) > GAMEPAD_RIGHT_THUMB_DEADZONE)
                            {
                                newController->isAnalog = true;
                            }
                            else
                            {
                                newController->isAnalog = false;
                            }

                            r32 threshold = 0.5f;
                            ProcessXInputButton(
                                    (newController->leftStickAvg.x < -threshold) ? 1 : 0,
                                    &oldController->moveLeft, 1,
                                    &newController->moveLeft);
                            ProcessXInputButton(
                                    (newController->leftStickAvg.x > threshold) ? 1 : 0,
                                    &oldController->moveRight, 1,
                                    &newController->moveRight);
                            ProcessXInputButton(
                                    (newController->leftStickAvg.y < -threshold) ? 1 : 0,
                                    &oldController->moveDown, 1,
                                    &newController->moveDown);
                            ProcessXInputButton(
                                    (newController->leftStickAvg.y > threshold) ? 1 : 0,
                                    &oldController->moveUp, 1,
                                    &newController->moveUp);
                        }
                        else
                        {
                            // NOTE: Controller unavailable
                            newController->isConnected = false;
                        }
                    }

                    //
                    // NOTE: Update
                    //

                    game_render_commands renderCommands =
                    {
                        (GlobalWindowRect.right - GlobalWindowRect.left),
                        (GlobalWindowRect.bottom - GlobalWindowRect.top),
                        MAT4_IDENTITY, MAT4_IDENTITY,
                        Vec3(2.0f, 4.0f, 2.0f),
                        (u32)pushBufferSize, 0, (u8 *)pushBuffer
                    };

                    GameUpdate(&gameMemory, newInput, &renderCommands);

                    //
                    // NOTE: Render
                    //

                    //RenderOpenGL(&windowHandle, &deviceContext, 0, 0, windowWidth, windowHeight);
                    RenderOpenGL(&windowHandle, &deviceContext, 0, 0,
                                (GlobalWindowRect.right - GlobalWindowRect.left),
                                (GlobalWindowRect.bottom - GlobalWindowRect.top),
                                &renderCommands);

                    // NOTE: Swap game input
                    Swap(newInput, oldInput);

                    // NOTE: Timing
                    u64 endCycleCount = __rdtsc();

                    LARGE_INTEGER endCounter;
                    QueryPerformanceCounter(&endCounter);

                    u64 cyclesElapsed = endCycleCount - lastCycleCount;
                    s64 elapsedTime = endCounter.QuadPart - lastCounter.QuadPart;
                    msPerFrame = (1000.0f * (r32)elapsedTime) / (r32)perfFrequency;
                    r32 fps = (r32)perfFrequency / (r32)elapsedTime;
                    r32 mcpf = cyclesElapsed / 1000000.0f;

                    char buf[256];
                    sprintf_s(buf, sizeof(buf), "%.3fms (%.3f fps), %.3fMCycles\n", msPerFrame, fps, mcpf);
                    OutputDebugStringA(buf);

                    lastCycleCount = endCycleCount;
                    lastCounter = endCounter;
                }
            }

            TerminateOpenGL(windowHandle, deviceContext, renderContext);
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
