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

#if 0
static void RenderMesh(opengl_mesh *mesh, mat4 *model, mat4 *view, mat4 *projection,
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

static void RenderOpenGL(HWND *window, HDC *deviceContext, u32 x, u32 y, u32 width, u32 height, game_render_commands *renderCommands)
{
    // NOTE: Render into framebuffer
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // NOTE: Shadow/depth pass

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

    // NOTE: Main rendering pass

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
#endif

static void RenderOpenGL(HWND *window, HDC *deviceContext, u32 x, u32 y, u32 windowWidth, u32 windowHeight, game_render_commands *commands)
{
    //glViewport(0, 0, commands->width, commands->height);
    //glUseProgram(gShaderProgram);

    b32 depthPass = true;

    // NOTE: MVP from light's POV
    mat4 depthProj = Orthographic(-10, 10, -10, 10, -10, 20);
    mat4 depthView = LookAt(commands->lightInvDir, Vec3(0, 0, 0), Vec3(0, 1, 0));
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
            glViewport(x, y, commands->width, commands->height);
            glCullFace(GL_BACK);
            glUseProgram(gShaderProgram);
        }

        u32 offset = 0;
        while(offset < commands->pushBufferSize)
        {
            render_entry_header *header = (render_entry_header *)(commands->pushBufferBase + offset);

            switch(header->type)
            {
                case render_entry_type_clear:
                {
                    render_entry_clear *clear = (render_entry_clear *)header;
                    offset += sizeof(render_entry_clear);

                    if(depthPass)
                    {
                        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                    }
                    else
                    {
                        glClearColor(clear->colour.r, clear->colour.g, clear->colour.b, clear->colour.a);
                    }
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                } break;

                case render_entry_type_mesh:
                {
                    render_entry_mesh *entry = (render_entry_mesh *)header;
                    simple_mesh *mesh = entry->mesh;
                    offset += sizeof(render_entry_mesh);

                    if(depthPass)
                    {
                        mat4 mvp = depthVP * entry->world;
                        glUniformMatrix4fv(gRTTMVP, 1, GL_FALSE, &mvp.m[0][0]);
                    }

                    if(!mesh->vaoHandle)
                    {
                        glGenVertexArrays(1, &mesh->vaoHandle);
                        glBindVertexArray(mesh->vaoHandle);
                    }

                    // NOTE: glDeleteBuffers, also move this to asset loading/unloading
                    if(!mesh->vbufHandle)
                    {
                        glGenBuffers(1, &mesh->vbufHandle);
                        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbufHandle);
                        glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(vertex), mesh->vertices, GL_STATIC_DRAW);
                    }

                    if(!mesh->ibufHandle)
                    {
                        glGenBuffers(1, &mesh->ibufHandle);
                        glBindBuffer(GL_ARRAY_BUFFER, mesh->ibufHandle);
                        glBufferData(GL_ARRAY_BUFFER, mesh->numIndices * sizeof(u16), mesh->indices, GL_STATIC_DRAW);
                    }

                    if(!depthPass)
                    {
                        mat4 wvp = commands->projection * commands->view * entry->world;
                        local_persist mat4 biasMatrix = {
                            0.5f, 0.0f, 0.0f, 0.0f,
                            0.0f, 0.5f, 0.0f, 0.0f,
                            0.0f, 0.0f, 0.5f, 0.0f,
                            0.5f, 0.5f, 0.5f, 1.0f
                        };
                        mat4 depthBiasMVP = biasMatrix * depthVP * entry->world;

                        glUniformMatrix4fv(gMVP, 1, GL_FALSE, &wvp.m[0][0]);
                        glUniformMatrix4fv(gM, 1, GL_FALSE, &entry->world.m[0][0]);
                        glUniformMatrix4fv(gV, 1, GL_FALSE, &commands->view.m[0][0]);
                        glUniformMatrix4fv(gDepthBiasMVP, 1, GL_FALSE, &depthBiasMVP.m[0][0]);
                        glUniform3f(gLightInvDir, commands->lightInvDir.x, commands->lightInvDir.y, commands->lightInvDir.z);
                        //glUniform3f(gLightPos, 5.0f, 5.0f, 5.0f);

                        glActiveTexture(GL_TEXTURE0);
                        if(mesh->material->diffuseTexture.handle)
                        {
                            glBindTexture(GL_TEXTURE_2D, mesh->material->diffuseTexture.handle);
                        }
                        else
                        {
                            // NOTE: glDeleteTextures, also handle this in asset loading/unloading
                            glGenTextures(1, &(mesh->material->diffuseTexture.handle));
                            glBindTexture(GL_TEXTURE_2D, mesh->material->diffuseTexture.handle);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mesh->material->diffuseTexture.width, mesh->material->diffuseTexture.height,
                                    0, GL_BGR, GL_UNSIGNED_BYTE, mesh->material->diffuseTexture.data);
                        }

                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, gDepthTexture);
                        glUniform1i(gShadowSampler, 1);
                    }

                    // NOTE: Bind vertex buffers
                    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbufHandle);
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
                    if(!depthPass)
                    {
                        // NOTE: UVs
                        glEnableVertexAttribArray(1);
                        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, uv));//(2 * sizeof(r32)));
                        // NOTE: Normals
                        glEnableVertexAttribArray(2);
                        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, normal));//(3 * sizeof(r32)));
                    }

                    // NOTE: Bind index buffer
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibufHandle);

                    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_SHORT, 0);

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

    SwapBuffers(*deviceContext);
}

