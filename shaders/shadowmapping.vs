#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;

out vec2 uv;
out vec3 pos;
out vec3 norm;
out vec3 eyeDir;
out vec3 lightDir;
out vec4 shadowCoord;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightInvDir;
uniform mat4 depthBiasMVP;

void main()
{
    gl_Position = MVP * vec4(inPos, 1.0);

    shadowCoord = depthBiasMVP * vec4(inPos, 1.0);

    pos = (M * vec4(inPos, 1.0)).xyz;

    eyeDir = vec3(0, 0, 0) - (V * M * vec4(inPos, 1.0)).xyz;

    lightDir = (V * vec4(lightInvDir, 0)).xyz;

    norm = (V * M * vec4(inNormal, 0)).xyz;

    uv = inUV;
}

