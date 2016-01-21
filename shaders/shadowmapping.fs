#version 330

in vec2 uv;
in vec3 pos;
in vec3 norm;
in vec3 eyeDir;
in vec3 lightDir;
in vec4 shadowCoord;

layout (location = 0) out vec3 colour;

uniform sampler2D sampler;
//uniform vec3 lightPos;
uniform sampler2DShadow shadowSampler;

vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float random(vec3 seed, int i)
{
    vec4 seed4 = vec4(seed, i);
    float dotp = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dotp) * 43758.5453);
}

void main()
{
    vec3 lightCol = vec3(1.0, 1.0, .925);
    float lightPow = 2.0;

    vec3 matDifCol = texture(sampler, uv).rgb;
    vec3 matAmbCol = vec3(0.15, 0.175, 0.2) * matDifCol;
    vec3 matSpcCol = vec3(0.8, 0.8, 0.8);

    vec3 n = normalize(norm);
    vec3 l = normalize(lightDir);
    float cosTheta = clamp(dot(n, l), 0, 1);

    vec3 e = normalize(eyeDir);
    vec3 r = reflect(-l, n);
    float cosAlpha = clamp(dot(e, r), 0, 1);

    float vis = 1.0;
    float bias = 0.005;

    // Sample the shadow map
    for(int i = 0; i < 4; ++i)
    {
        //int idx = i;
        // Random sample based upon pixel screen pos
        // int idx = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
        // Random sample based upon pixel world pos
        int idx = int(16.0 * random(floor(pos.xyz * 1000.0), i)) % 16;
        vis -= 0.2 * (1.0 - texture(shadowSampler, vec3(shadowCoord.xy + poissonDisk[idx] / 700.0, (shadowCoord.z - bias) / shadowCoord.w)));
    }

    colour = matAmbCol + vis * matDifCol * lightCol * lightPow * cosTheta
            + vis * matSpcCol * lightCol * lightPow * pow(cosAlpha, 5);
}
