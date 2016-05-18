#include "renderer.h"
#include "renderer_opengl.h"

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

static texture_handle *RendererCreateTexture(texture *Tex, u32 *Data)
{
    texture_handle *Handle = new texture_handle;
    switch(Tex->Type)
    {
        case TextureType_1D:
        {
        } break;

        case TextureType_2D:
        {
            glGenTextures(1, &(Handle->Value));
            glBindTexture(GL_TEXTURE_2D, Handle->Value);
            /*
            switch(Tex->MagFilterType)
            {
                case TextureFilterType_Nearest:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                } break;
                case TextureFilterType_Linear:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } break;
            }
            switch(Tex->MinFilterType)
            {
                case TextureFilterType_Nearest:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                } break;
                case TextureFilterType_Linear:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                } break;
            }
            switch(Tex->WrapModeU)
            {
                case TextureMode_Wrap:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                } break;
                case TextureMode_Clamp:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                } break;
            }
            switch(Tex->WrapModeV)
            {
                case TextureFilterType_Nearest:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                } break;
                case TextureFilterType_Linear:
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                } break;
            }
            */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Tex->MagFilterType);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Tex->MinFilterType);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Tex->WrapModeU);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Tex->WrapModeV);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Tex->Width, Tex->Height, 0, GL_BGR,
                         GL_UNSIGNED_BYTE, Data);
            glBindTexture(GL_TEXTURE_2D, 0);
        } break;

        case TextureType_3D:
        {
        } break;
    }

    GLenum err = glGetError();
    while(err != GL_NO_ERROR)
    {
        err = glGetError();
    }

    return Handle;
}

static void RendererDeleteTexture(texture_handle *Handle)
{
    glDeleteTextures(1, &Handle->Value);
    Handle->Value = 0;
}

static mesh_handles *RendererCreateBuffers(mesh *Mesh)
{
    mesh_handles *Handles = new mesh_handles;

    glGenVertexArrays(1, &Handles->VAO);
    glBindVertexArray(Handles->VAO);

    // NOTE: glDeleteBuffers, also move this to asset loading/unloading
    glGenBuffers(1, &Handles->VBuf);
    glBindBuffer(GL_ARRAY_BUFFER, Handles->VBuf);
    glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex), Mesh->Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &Handles->IBuf);
    glBindBuffer(GL_ARRAY_BUFFER, Handles->IBuf);
    glBufferData(GL_ARRAY_BUFFER, Mesh->IndexCount * sizeof(u16), Mesh->Indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLenum err = glGetError();
    while(err != GL_NO_ERROR)
    {
        err = glGetError();
    }

    return Handles;
}

static void RendererDeleteBuffers(mesh_handles *Handles)
{
    glDeleteVertexArrays(1, &Handles->VAO);
    glDeleteBuffers(1, &Handles->VBuf);
    glDeleteBuffers(1, &Handles->IBuf);
    Handles->VAO = Handles->VBuf = Handles->IBuf = 0;
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

    /*
    for(u32 i = 0; i < ArrayCount(gMeshes); ++i)
    {
        glDeleteBuffers(1, &gMeshes[i]->vbuf);
        glDeleteBuffers(1, &gMeshes[i]->uvbuf);
        glDeleteBuffers(1, &gMeshes[i]->nbuf);
        if(gMeshes[i]->material->diffuseTex)
            glDeleteTextures(1, &gMeshes[i]->material->diffuseTex);
    }
    */

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
}

static void RenderOpenGL(HWND *window, HDC *deviceContext, u32 x, u32 y, u32 windowWidth, u32 windowHeight, game_render_commands *commands)
{
    //glViewport(0, 0, commands->width, commands->height);
    //glUseProgram(gShaderProgram);

    b32 depthPass = true;

    // NOTE: MVP from light's POV
    mat4 depthProj = Orthographic(-10, 10, -10, 10, -10, 20);
    mat4 depthView = LookAt(commands->LightInvDir, Vec3(0, 0, 0), Vec3(0, 1, 0));
    mat4 depthVP = depthProj * depthView;

    for(u32 i = 0; i < 2; ++i)
    {
        if(depthPass)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);
            glViewport(0, 0, 1024, 1024);
            glUseProgram(gRTTShaderProgram);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(x, y, commands->Width, commands->Height);
            glCullFace(GL_BACK);
            glUseProgram(gShaderProgram);
        }

        u32 offset = 0;
        while(offset < commands->PushBufferSize)
        {
            render_entry_header *header = (render_entry_header *)(commands->PushBufferBase + offset);

            switch(header->Type)
            {
                case RenderEntry_Clear:
                {
                    render_entry_clear *clear = (render_entry_clear *)header;
                    offset += sizeof(render_entry_clear);

                    if(depthPass)
                    {
                        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                    }
                    else
                    {
                        glClearColor(clear->Colour.r, clear->Colour.g, clear->Colour.b, clear->Colour.a);
                    }
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                } break;

                case RenderEntry_Wireframe:
                {
                    render_entry_wireframe *Entry = (render_entry_wireframe *)header;
                    offset += sizeof(render_entry_wireframe);

                    if(Entry->Enabled)
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    }
                    else
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    }
                } break;

                case RenderEntry_Mesh:
                {
                    render_entry_mesh *entry = (render_entry_mesh *)header;
                    mesh *Mesh = entry->Mesh;
                    material *Material = entry->Material;
                    offset += sizeof(render_entry_mesh);

                    if(depthPass)
                    {
                        mat4 mvp = depthVP * entry->World;
                        glUniformMatrix4fv(gRTTMVP, 1, GL_FALSE, &mvp.m[0][0]);
                    }

                    /*
                    if(!Mesh->Handles->VAO)
                    {
                        glGenVertexArrays(1, &Mesh->Handles->VAO);
                        glBindVertexArray(Mesh->Handles->VAO);
                    }

                    // NOTE: glDeleteBuffers, also move this to asset loading/unloading
                    if(!Mesh->Handles->VBuf)
                    {
                        glGenBuffers(1, &Mesh->Handles->VBuf);
                        glBindBuffer(GL_ARRAY_BUFFER, Mesh->Handles->VBuf);
                        glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(vertex), Mesh->Vertices, GL_STATIC_DRAW);
                    }

                    if(!Mesh->Handles->IBuf)
                    {
                        glGenBuffers(1, &Mesh->Handles->IBuf);
                        glBindBuffer(GL_ARRAY_BUFFER, Mesh->Handles->IBuf);
                        glBufferData(GL_ARRAY_BUFFER, Mesh->IndexCount * sizeof(u16), Mesh->Indices, GL_STATIC_DRAW);
                    }
                    */

                    if(!depthPass)
                    {
                        mat4 wvp = commands->Projection * commands->View * entry->World;
                        local_persist mat4 biasMatrix = {
                            0.5f, 0.0f, 0.0f, 0.0f,
                            0.0f, 0.5f, 0.0f, 0.0f,
                            0.0f, 0.0f, 0.5f, 0.0f,
                            0.5f, 0.5f, 0.5f, 1.0f
                        };
                        mat4 depthBiasMVP = biasMatrix * depthVP * entry->World;

                        glUniformMatrix4fv(gMVP, 1, GL_FALSE, &wvp.m[0][0]);
                        glUniformMatrix4fv(gM, 1, GL_FALSE, &entry->World.m[0][0]);
                        glUniformMatrix4fv(gV, 1, GL_FALSE, &commands->View.m[0][0]);
                        glUniformMatrix4fv(gDepthBiasMVP, 1, GL_FALSE, &depthBiasMVP.m[0][0]);
                        glUniform3f(gLightInvDir, commands->LightInvDir.x, commands->LightInvDir.y, commands->LightInvDir.z);
                        //glUniform3f(gLightPos, 5.0f, 5.0f, 5.0f);

                        glActiveTexture(GL_TEXTURE0);
                        if(Material->DiffuseTexture->Handle->Value)
                        {
                            glBindTexture(GL_TEXTURE_2D, Material->DiffuseTexture->Handle->Value);
                        }

                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, gDepthTexture);
                        glUniform1i(gShadowSampler, 1);
                    }

                    // NOTE: Bind vertex buffers
                    glBindBuffer(GL_ARRAY_BUFFER, Mesh->Handles->VBuf);
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, Pos));//(void *)0);
                    if(!depthPass)
                    {
                        // NOTE: UVs
                        glEnableVertexAttribArray(1);
                        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, UV));//(2 * sizeof(r32)));
                        // NOTE: Normals
                        glEnableVertexAttribArray(2);
                        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, Normal));//(3 * sizeof(r32)));
                    }

                    // NOTE: Bind index buffer
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->Handles->IBuf);

                    glDrawElements(Mesh->PrimType, Mesh->IndexCount, GL_UNSIGNED_SHORT, 0);

                    glDisableVertexAttribArray(0);
                    if(!depthPass)
                    {
                        glDisableVertexAttribArray(1);
                        glDisableVertexAttribArray(2);
                    }
                    glBindVertexArray(0);
                } break;

                default:
                {
                    Assert(!"Invalid default case!");
                } break;
            }
        }
        depthPass = false;
    }

    GLenum err = glGetError();
    while(err != GL_NO_ERROR)
    {
        err = glGetError();
    }

    SwapBuffers(*deviceContext);
}

