#include <stdint.h>

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

#define internal static
#define local_persist static
#define global_variable static

// NOTE: Unity build (single compilation unit)!
#include "game_main.cpp"

#include <windows.h>
#include <gl\gl.h>
#include <stdio.h>
#include <xinput.h>

global_variable b32 GlobalRunning;

LRESULT CALLBACK MainWindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = {};

    switch(message)
    {
        case WM_SIZE:
        {
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

internal void LoadXInput()
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

internal void ProcessXInputButton(DWORD buttonState, game_button_state *oldState, DWORD buttonBit, game_button_state *newState)
{
    newState->endedDown = (buttonState & buttonBit) == buttonBit;
    newState->halfTransitionCount = (oldState->endedDown != newState->endedDown) ? 1 : 0;
}

internal v2 ProcessXInputStick(SHORT x, SHORT y, SHORT deadzone)
{
    v2 result = {};
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

internal void ProcessKeyboardKey(game_button_state *newState, b32 isDown)
{
    Assert(newState->endedDown != isDown);
    newState->endedDown = isDown;
    ++newState->halfTransitionCount;
}

internal void ProcessWindowsMessages(game_controller_input *keyboardController)
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

internal void FreeFile(void *file)
{
    if(file)
    {
        VirtualFree(file, 0, MEM_RELEASE);
    }
}

internal void *ReadEntireFile(char *filename)
{
    void *result = 0;

    HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(file != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER size;
        if(GetFileSizeEx(file, &size))
        {
            Assert(size.QuadPart <= 0xFFFFFFFF);
            result = VirtualAlloc(0, size.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(result)
            {
                DWORD bytesRead;
                if(ReadFile(file, &result, size.LowPart, &bytesRead, 0) && (size.LowPart == bytesRead))
                {
                }
                else
                {
                    FreeFile(file);
                    result = 0;
                }
            }
        }

        CloseHandle(file);
    }

    return result;
}

internal void EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC)
{
    *hDC = GetDC(hWnd);

    s32 format;

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    format = ChoosePixelFormat(*hDC, &pfd);
    SetPixelFormat(*hDC, format, &pfd);

    *hRC = wglCreateContext(*hDC);
    wglMakeCurrent(*hDC, *hRC);
}

internal void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
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
        HWND windowHandle =
            CreateWindowEx(
                0, 
                wc.lpszClassName,
                "AAEngine",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                0, 0, 1280, 720,
                0,
                0,
                hInstance,
                0);
        if(windowHandle)
        {
            HDC deviceContext;
            HGLRC renderContext;

            EnableOpenGL(windowHandle, &deviceContext, &renderContext);

            ReadEntireFile("shader.vs");

            // NOTE: Main loop
            GlobalRunning = true;

            LARGE_INTEGER lastCounter;
            QueryPerformanceCounter(&lastCounter);

            u64 lastCycleCount = __rdtsc();

            game_input input[2] = {};
            game_input *newInput = &input[0];
            game_input *oldInput = &input[1];

            while(GlobalRunning)
            {
                game_controller_input *oldKeyboardController = &oldInput->controllers[0];
                game_controller_input *newKeyboardController = &newInput->controllers[0];
                *newKeyboardController = {};

                for(s32 btnIdx = 0; btnIdx < ArrayCount(newKeyboardController->buttons); ++btnIdx)
                {
                    newKeyboardController->buttons[btnIdx] = oldKeyboardController->buttons[btnIdx];
                }

                ProcessWindowsMessages(newKeyboardController);

                // NOTE: Use XUSER_MAX_COUNT?
                DWORD maxControllers = ArrayCount(newInput->controllers) - 1;
                for(DWORD controllerIndex = 0; controllerIndex < maxControllers; ++controllerIndex)
                {
                    // NOTE: First controller index is reserved for the keyboard
                    game_controller_input *oldController = &oldInput->controllers[controllerIndex + 1];
                    game_controller_input *newController = &newInput->controllers[controllerIndex + 1];

                    // TODO: Apparently there's some bug where XInputGetState delays for a relatively long time
                    // if no controllers are connected, so it may be better to listen to HID connections directly
                    // and then only call get state on controllers we know exist.
                    XINPUT_STATE controllerState;
                    if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
                    {
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

#define GAMEPAD_LEFT_THUMB_DEADZONE = 3277;
#define GAMEPAD_RIGHT_THUMB_DEADZONE = 3277;

                        newController->leftStickAvg = ProcessXInputStick(pad->sThumbLX, pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                        newController->rightStickAvg = ProcessXInputStick(pad->sThumbRX, pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

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
                    }
                }

                //
                // NOTE: Game-layer
                //

                GameUpdate(newInput);

                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                local_persist r32 angle = 0.0f;
                
                glPushMatrix();
                glRotatef(angle, 0.0f, 0.0f, 1.0f);
                glBegin(GL_TRIANGLES);
                glColor3f(1.0f, 0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
                glColor3f(0.0f, 1.0f, 0.0f); glVertex2f(0.87f, -0.5f);
                glColor3f(0.0f, 0.0f, 1.0f); glVertex2f(-0.87f, -0.5f);
                glEnd();
                glPopMatrix();

                SwapBuffers(deviceContext);

                angle += 1.0f;
                
                Swap(newInput, oldInput);

                // NOTE: Timing
                u64 endCycleCount = __rdtsc();

                LARGE_INTEGER endCounter;
                QueryPerformanceCounter(&endCounter);

                u64 cyclesElapsed = endCycleCount - lastCycleCount;
                s64 elapsedTime = endCounter.QuadPart - lastCounter.QuadPart;
                r32 msPerFrame = (1000.0f * (r32)elapsedTime) / (r32)perfFrequency;
                r32 fps = (r32)perfFrequency / (r32)elapsedTime;
                r32 mcpf = cyclesElapsed / 1000000.0f;

                char buf[256];
                sprintf_s(buf, sizeof(buf), "%.3fms (%.3f fps), %.3fMCycles\n", msPerFrame, fps, mcpf);
                OutputDebugStringA(buf);

                lastCycleCount = endCycleCount;
                lastCounter = endCounter;
            }

            DisableOpenGL(windowHandle, deviceContext, renderContext);
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
