#ifndef RENDERER_OPENGL_H
#define RENDERER_OPENGL_H

global_variable GLuint gVArray;

global_variable GLuint gFramebuffer, gDepthTexture;
global_variable GLuint gQuadBuf;

global_variable GLuint gShaderProgram;
global_variable GLuint gMVP, gM, gV;
global_variable GLuint gDepthBiasMVP, gShadowSampler;
global_variable GLuint gLightInvDir;
//global_variable GLuint gLightPos;
global_variable GLuint gSampler;

global_variable GLuint gRTTShaderProgram;
global_variable GLuint gRTTMVP;

global_variable GLuint gPreviewProgram;
global_variable GLuint gPreviewSampler;

#endif

