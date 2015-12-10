#include <windows.h>
#include <gl\gl.h>
#include <xinput.h>

#include "AAMath\aatypes.h"

#define internal static
#define local_persist static
#define global_variable static

global_variable bool GlobalRunning;

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
        case WM_KEYDOWN:
        {
            switch(wParam)
            {
                case VK_ESCAPE:
                {
                    GlobalRunning = false;
                } break;
            }
        } break;
        default:
        {
            result = DefWindowProc(hWnd, message, wParam, lParam);
        } break;
    }

    return result;
}

void EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC)
{
    *hDC = GetDC(hWnd);

    int format;

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

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

            // NOTE: Main loop
            GlobalRunning = true;
            while(GlobalRunning)
            {
                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if(message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                local_persist float angle = 0.0f;

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
