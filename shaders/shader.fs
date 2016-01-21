#version 330

in vec2 uv;
in vec3 pos;
in vec3 norm;
in vec3 eyeDir;
in vec3 lightDir;

uniform sampler2D sampler;
//uniform mat4 MV;
uniform vec3 lightPos;

out vec3 colour;

void main()
{
    // light emission properties
    vec3 lightCol = vec3(1.0, 1.0, 1.0);
    float lightPow = 50.0f;

    // material properties
    vec3 matDifCol = texture(sampler, uv).rgb;
    vec3 matAmbCol = vec3(0.4, 0.4, 0.4) * matDifCol;
    vec3 matSpcCol = vec3(0.8, 0.8, 0.8);

    // light distance
    float dist = length(lightPos - pos);

    vec3 n = normalize(norm);
    vec3 l = normalize(lightDir);
    float cosTheta = clamp(dot(n, l), 0, 1);

    vec3 e = normalize(eyeDir);
    vec3 r = reflect(-l, n);
    float cosAlpha = clamp(dot(e, r), 0, 1);

    colour = matAmbCol + (matDifCol * lightCol * lightPos * cosTheta / (dist * dist))
             + (matSpcCol * lightCol * lightPow * pow(cosAlpha, 5) / (dist * dist));
}

