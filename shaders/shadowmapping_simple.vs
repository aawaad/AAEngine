#version 330

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNorm;

out vec2 uv;
out vec4 shadowCoord;

uniform mat4 MVP;
uniform mat4 biasMVP;

void main()
{
    gl_Position = MVP * vec4(inPos, 1);
    shadowCoord = biasMVP * vec4(inPos, 1);
    uv = inUV;
}

