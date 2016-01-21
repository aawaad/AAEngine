#version 330

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNorm;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

out vec2 uv;
out vec3 pos;
out vec3 norm;
out vec3 eyeDir;
out vec3 lightDir;

void main()
{
    gl_Position = MVP * vec4(inPos, 1.0);

    pos = (M * vec4(inPos, 1.0)).xyz;

    // vertex -> camera vector
    vec3 vpos = (V * M * vec4(inPos, 1.0)).xyz;
    eyeDir = -vpos;

    // vertex -> lightDir vector
    vec3 l = (V * vec4(lightPos, 1.0)).xyz;
    lightDir = l - eyeDir;

    norm = (V * M * vec4(inNorm, 0)).xyz;

    uv = inUV;
}

