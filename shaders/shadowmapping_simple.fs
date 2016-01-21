#version 330

in vec2 uv;
in vec4 shadowCoord;

layout (location = 0) out vec3 colour;

uniform sampler2D sampler;
uniform sampler2D shadowSampler;

void main()
{
    vec3 lightCol = vec3(1.0, 1.0, 1.0);
    vec3 difCol = texture(sampler, uv).rgb;
    float vis = texture(shadowSampler, vec3(shadowCoord.xy, shadowCoord.z / shadowCoord.w));
    colour = vis * difCol * lightCol;
}

