#include <windows.h>
#define GLEW_STATIC
#include "glew\glew.h"
#include <gl\gl.h>
#include <stdio.h>
#include <xinput.h>
#include <stdint.h>

#include "../../AAMath/src/aamath.h"
using namespace aam;

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

#define global_variable static
#define local_persist static

// NOTE: Unity build (single compilation unit)!
#include "game_main.cpp"
#include "objloader.cpp"

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
    Assert(newState->endedDown != isDown);
    newState->endedDown = isDown;
    ++newState->halfTransitionCount;
}

static void ProcessWindowsMessages(game_controller_input *keyboardController)
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

static char *ReadShader(char *filename)
{
    char *result;

    size_t size;
    FILE *file;
    fopen_s(&file, filename, "rb");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    result = (char *)malloc((size + 1) * sizeof(char));
    fread(result, sizeof(char), size, file);
    fclose(file);

    result[size] = 0;
    return result;
}

static void AddShader(GLuint shaderProgram, const char *pShaderText, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);

    if(!shader)
    {
        // TODO: Logging
        exit(1);
    }

    const GLchar *p[1];
    p[0] = pShaderText;
    GLint len[1];
    len[0] = (GLint)strlen(pShaderText);
    glShaderSource(shader, 1, p, len);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        // TODO: Logging
        GLchar log[1024];
        glGetShaderInfoLog(shader, 1024, 0, log);
        OutputDebugStringA(log);
        exit(1);
    }

    glAttachShader(shaderProgram, shader);
}

global_variable GLuint gVArray;

global_variable GLuint gFramebuffer, gDepthTexture;
global_variable GLuint gQuadBuf;

global_variable GLuint gShaderProgram;
global_variable GLuint gMVP, gM, gV;
global_variable GLuint gDepthBiasMVP, gShadowSampler;
global_variable GLuint gLightInvDir;//, gLightPos;
global_variable GLuint gSampler;

global_variable GLuint gRTTShaderProgram;
global_variable GLuint gRTTMVP;

global_variable GLuint gPreviewProgram;
global_variable GLuint gPreviewSampler;

global_variable simple_mesh *gMeshes[2];

static GLuint CompileShaders(char *vert, char *frag)
{
    GLuint shaderProgram = glCreateProgram();

    if(shaderProgram == 0)
    {
        // TODO: Logging
        //exit(1);
    }

    char *vs = ReadShader(vert);
    char *fs = ReadShader(frag);

    if(!vs || !fs) exit(1);

    AddShader(shaderProgram, vs, GL_VERTEX_SHADER);
    AddShader(shaderProgram, fs, GL_FRAGMENT_SHADER);

    GLint success = 0;
    
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    //if(!success) exit(1);

    glValidateProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
    //if(!success) exit(1);

    return shaderProgram;
}

static b32 InitializeRenderToTexture(u32 width, u32 height)
{
    glGenFramebuffers(1, &gFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);

    // NOTE: Depth texture
    glGenRenderbuffers(1, &gDepthTexture);
    glBindTexture(GL_TEXTURE_2D, gDepthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    // NOTE: width, height power of 2?
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gDepthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

static void TerminateOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    glDeleteProgram(gShaderProgram);
    glDeleteProgram(gRTTShaderProgram);
    glDeleteProgram(gPreviewProgram);
    glDeleteVertexArrays(1, &gVArray);

    for(u32 i = 0; i < ArrayCount(gMeshes); ++i)
    {
        glDeleteBuffers(1, &gMeshes[i]->vbuf);
        glDeleteBuffers(1, &gMeshes[i]->uvbuf);
        glDeleteBuffers(1, &gMeshes[i]->nbuf);
        if(gMeshes[i]->material->diffuseTex)
            glDeleteTextures(1, &gMeshes[i]->material->diffuseTex);
    }

    glDeleteBuffers(1, &gFramebuffer);
    glDeleteBuffers(1, &gQuadBuf);
    glDeleteTextures(1, &gDepthTexture);

    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

static void InitializeOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC, u32 windowWidth, u32 windowHeight)
{
    *hDC = GetDC(hWnd);

    PIXELFORMATDESCRIPTOR desiredPFD = {};
    desiredPFD.nSize = sizeof(desiredPFD);
    desiredPFD.nVersion = 1;
    desiredPFD.iPixelType = PFD_TYPE_RGBA;
    desiredPFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desiredPFD.iPixelType = PFD_TYPE_RGBA;
    desiredPFD.cColorBits = 32;
    desiredPFD.cAlphaBits = 8;
    desiredPFD.iLayerType = PFD_MAIN_PLANE;

    s32 suggestedPFIndex = ChoosePixelFormat(*hDC, &desiredPFD);
    PIXELFORMATDESCRIPTOR suggestedPFD;
    DescribePixelFormat(*hDC, suggestedPFIndex, sizeof(suggestedPFD), &suggestedPFD);
    SetPixelFormat(*hDC, suggestedPFIndex, &suggestedPFD);

    *hRC = wglCreateContext(*hDC);
    if(!wglMakeCurrent(*hDC, *hRC))
    {
        TerminateOpenGL(hWnd, *hDC, *hRC);
        exit(1);
    }

    // NOTE: GLEW
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        exit(1);
    }
    
    OutputDebugStringA((char *)glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    // NOTE: Render to texture
    InitializeRenderToTexture(1024, 1024);

    const GLfloat quadVerts[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };

    glGenBuffers(1, &gQuadBuf);
    glBindBuffer(GL_ARRAY_BUFFER, gQuadBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    gRTTShaderProgram = CompileShaders("../shaders/rtt.vs", "../shaders/rtt.fs");
    gRTTMVP = glGetUniformLocation(gRTTShaderProgram, "MVP");

    // NOTE: vertex arrays
    glGenVertexArrays(1, &gVArray);
    glBindVertexArray(gVArray);

    // NOTE: Default shaders
    //gShaderProgram = CompileShaders("../shaders/shader.vs", "../shaders/shader.fs");
    gShaderProgram = CompileShaders("../shaders/shadowmapping.vs", "../shaders/shadowmapping.fs");
    //glUseProgram(gShaderProgram);

    // NOTE: Get uniform locations for default shaders
    gMVP = glGetUniformLocation(gShaderProgram, "MVP");
    //Assert(gMVP != -1);
    gM = glGetUniformLocation(gShaderProgram, "M");
    gV = glGetUniformLocation(gShaderProgram, "V");
    gDepthBiasMVP = glGetUniformLocation(gShaderProgram, "depthBiasMVP");
    gShadowSampler = glGetUniformLocation(gShaderProgram, "shadowSampler");
    gLightInvDir = glGetUniformLocation(gShaderProgram, "lightInvDir");
    //gLightPos = glGetUniformLocation(gShaderProgram, "lightPos");
    gSampler = glGetUniformLocation(gShaderProgram, "sampler");

    // NOTE: simple shaders for shadow map preview window
    gPreviewProgram = CompileShaders("../shaders/basic.vs", "../shaders/basic.fs");
    gPreviewSampler = glGetUniformLocation(gPreviewProgram, "sampler");

    gMeshes[0] = LoadMesh(L"../data/room_thickwalls.obj");
    //gMeshes[1] = LoadMesh(L"../data/cube.obj");
    gMeshes[1] = LoadMesh(L"../data/pig.obj", true);
}

static void RenderMesh(simple_mesh *mesh, mat4 *model, mat4 *view, mat4 *projection,
                       mat4 *depthMVP, vec3 *lightInvDir, b32 depthPass = false)
{
    if(depthPass)
    {
        mat4 depth = *projection * *view * *model;
        glUniformMatrix4fv(gRTTMVP, 1, GL_FALSE, &depth.m[0][0]);
    }
    else
    {
        // NOTE: MVP
        mat4 mvp = *projection * *view * *model;

        local_persist mat4 biasMatrix = {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f
        };
        *depthMVP *= *model;
        mat4 depthBiasMVP = biasMatrix * *depthMVP;

        glUniformMatrix4fv(gMVP, 1, GL_FALSE, &mvp.m[0][0]);
        glUniformMatrix4fv(gM, 1, GL_FALSE, &model->m[0][0]);
        glUniformMatrix4fv(gV, 1, GL_FALSE, &view->m[0][0]);
        glUniformMatrix4fv(gDepthBiasMVP, 1, GL_FALSE, &depthBiasMVP.m[0][0]);
        glUniform3f(gLightInvDir, lightInvDir->x, lightInvDir->y, lightInvDir->z);

        //vec3 lightPos = Vec3(10.0f, 10.0f, 10.0f);
        //glUniform3f(gLightPos, lightPos.x, lightPos.y, lightPos.z);

        // NOTE: Bind the texture
        if(mesh->material->diffuseTex)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh->material->diffuseTex);
            glUniform1i(gSampler, 0);
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gDepthTexture);
        glUniform1i(gShadowSampler, 1);
    }

    // NOTE: Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbuf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    if(!depthPass)
    {
        // NOTE: UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->uvbuf);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // NOTE: Normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->nbuf);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // NOTE: Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibuf);

    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(0);
    if(!depthPass)
    {
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }
}

static void RenderOpenGL(HWND *window, HDC *deviceContext, u32 x, u32 y, u32 width, u32 height)
{
    // NOTE: Render into framebuffer
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gRTTShaderProgram);

    vec3 lightInvDir = Vec3(2.0f, 4.0f, 2.0f);

    // NOTE: MVP from light's POV
    mat4 depthProj = Orthographic(-10, 10, -10, 10, -10, 20);
    mat4 depthView = LookAt(lightInvDir, Vec3(0, 0, 0), Vec3(0, 1, 0));
    mat4 depthMVP = depthProj * depthView;

    // NOTE: Spotlight
    //vec3 lightPos = Vec3(5, 20, 20);
    //mat4 depthProj = Perspective(PI / 2.0f, 1.0f, 2.0f, 50.0f);
    //mat4 depthView = LookAt(lightInvDir, Vec3(0, 0, 0), Vec3(0, 1, 0));

    /*    
    for(u32 i = 0; i < ArrayCount(gMeshes); ++i)
    {
        RenderMesh(gMeshes[i], &depthView, &depthProj, Vec3(0.0f, -1.0f + 1.0f * i, 0.0f), &depthMVP, &lightInvDir, true);
    }
    */

    mat4 wallsModel = Mat4RotationY(-PIOVERTWO);
    wallsModel = MAT4_IDENTITY;
    RenderMesh(gMeshes[0], &wallsModel, &depthView, &depthProj, &depthMVP, &lightInvDir, true);

    local_persist r32 angle = 0.0f;
    mat4 pigSca = Mat4Scaling(1.0f + sinf(4.0f * angle) * 0.25f);
    mat4 pigRot = Mat4RotationY(angle);
    mat4 pigTrs = Mat4Translation(4.0f, 0, 0);
    mat4 pigModel = pigRot * pigTrs * pigSca;

    angle+= 0.025f;

    RenderMesh(gMeshes[1], &pigModel, &depthView, &depthProj, &depthMVP, &lightInvDir, true);

    // NOTE: Render to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glCullFace(GL_BACK);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgram);

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(*window, &p);

    vec2 nCursorPos = Vec2((r32)p.x / (r32)width, (r32)p.y / (r32)height);
    vec2 ndcCursorPos = nCursorPos * 2.0f - 1.0f;

    // NOTE: Camera
    //mat4 projection = Perspective(PI / 6.0f + nCursorPos.y  * PI / 3.0f, (r32)width/(r32)height, 0.1f, 100.0f);
    mat4 projection = Perspective(30.0f + nCursorPos.y * 60.0f, (r32)width/(r32)height, 0.1f, 100.0f);
    //mat4 view = LookAt(Vec3(6.0f, 4.0f, -6.0f),
    r32 limit = 3.0f * PIOVERFOUR;
    r32 offset = -PIOVERTWO;
    mat4 view = LookAt(Vec3(cosf(Clamp(ndcCursorPos.x * limit, -limit, limit) + offset) * -6.0f, 4.0f,
                            sinf(Clamp(ndcCursorPos.x * limit, -limit, limit) + offset) * 6.0f),
                       Vec3(0, 1, 0),
                       Vec3(0, 1, 0));

    /*
    for(u32 i = 0; i < ArrayCount(gMeshes); ++i)
    {
        RenderMesh(gMeshes[i], &view, &projection, Vec3(0, 0, 0) , &depthMVP, &lightInvDir, false);
    }
    */
    RenderMesh(gMeshes[0], &wallsModel, &view, &projection, &depthMVP, &lightInvDir, false);
    RenderMesh(gMeshes[1], &pigModel, &view, &projection, &depthMVP, &lightInvDir, false);

    // NOTE: Render depth texture to quad (testing)
    //       Need to disable GL_COMPARE_R_TO_TEXTURE
#if 0
    glViewport(0, 0, 256, 256);
    glUseProgram(gPreviewProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gDepthTexture);
    glUniform1i(gPreviewSampler, 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, gQuadBuf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(0);
#endif

    SwapBuffers(*deviceContext);
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
                "SOFT351 Demo",
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

            // NOTE: OpenGL init
            InitializeOpenGL(windowHandle, &deviceContext, &renderContext, windowWidth, windowHeight);

            // NOTE: Main loop
            GlobalRunning = true;

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

                //
                // NOTE: Rendering
                //

                RenderOpenGL(&windowHandle, &deviceContext, 0, 0, windowWidth, windowHeight);

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
