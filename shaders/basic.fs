#version 330

layout (location = 0) out vec4 colour;

uniform sampler2D sampler;

in vec2 uv;

void main()
{
    colour = texture(sampler, uv);
}

