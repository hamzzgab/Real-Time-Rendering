#version 330 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D screenTexture;

const float offset_x = 1.0f / 800.0f;
const float offset_y = 1.0f / 800.0f;

vec2 offsets[9] = vec2[]
(
    vec2(-offset_x,  offset_y), vec2( 0.0f,    offset_y), vec2( offset_x,  offset_y),
    vec2(-offset_x,  0.0f),     vec2( 0.0f,    0.0f),     vec2( offset_x,  0.0f),
    vec2(-offset_x, -offset_y), vec2( 0.0f,   -offset_y), vec2( offset_x, -offset_y)
);

float kernel[9] = float[]
(
    1,  1, 1,
    1, -8, 1,
    1,  1, 1
);

uniform vec4 uScale = vec4(0.9f);
uniform vec4 uBias = vec4(0.5f);

uniform int uGhosts = 5;
uniform float uGhostDispersal = 0.1f;
uniform float uHaloWidth = 0.1f;
uniform vec3 uDistortion = vec3(0.5f, 0.2f, 0.5f);

//uniform float uLensColor = 0.5f;
//uniform vec2 uLensColor = vec2(1.0f, 0.0f);
//uniform vec3 uLensColor = vec4(0.5f, 0.0f, 0.0f);

//vec3 textureDistorted(in sampler2D tex,
//                      in vec2 texcoord,
//                      in vec2 direction,
//                      in vec3 distortion)
//{
//    return vec3(
//                texture(tex, texcoord + direction * distortion.r) * r,
//                texture(tex, texcoord + direction * distortion.g) * g,
//                texture(tex, texcoord + direction * distortion.b) * b,
//                );
//}

void main()
{
//    vec2 texcoord = -texCoords + vec2(1.0f);
    vec2 texcoord = texCoords;
    vec2 texelSize = 1.0f / vec2(textureSize(screenTexture, 0));
    
    vec2 ghostVec = (vec2(0.5) - texcoord) * uGhostDispersal;
    
    // sample ghosts:
    vec4 result = vec4(0.0);
    for (int i = 0; i < uGhosts; ++i) {
        vec2 offset = fract(texcoord + ghostVec * float(i));
        
        vec2 haloVec = normalize(ghostVec) * uHaloWidth;
        
        
        float weight = length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));
        weight = pow(1.0f - weight, 5.0f);
        
//        result += texture(screenTexture, texcoord + haloVec) * weight;
        result += texture(screenTexture, offset) * weight;
    }
//    result *= texture(uLensColor, length(vec2(0.5) - texcoord) / length(vec2(0.5)));

    FragColor = result;
}
