#version 330

layout (location = 0) in vec3 pos;

out vec2 uv;

void main()
{
    gl_Position = vec4(pos, 1.0);
    uv = (pos.xy + vec2(1.0, 1.0)) / 2.0;
}

